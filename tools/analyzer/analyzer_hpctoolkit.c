/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane and Leonardo Fialho
 */

/* System standard headers */
#include <stdio.h>
#include <libxml/parser.h>

/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_alloc.h"
#include "perfexpert_constants.h"
#include "perfexpert_hash.h"
#include "perfexpert_output.h"
#include "perfexpert_string.h"
#include "perfexpert_util.h"

/* hpctoolkit_parse_file */
int hpctoolkit_parse_file(const char *file, perfexpert_list_t *profiles) {
    xmlDocPtr  document;
    xmlNodePtr node;
    profile_t  *profile = NULL;

    /* Open file */
    if (NULL == (document = xmlParseFile(file))) {
        OUTPUT(("%s", _ERROR("Error: malformed XML document")));
        return PERFEXPERT_ERROR;
    }

    /* Check if it is not empty */
    if (NULL == (node = xmlDocGetRootElement(document))) {
        OUTPUT(("%s", _ERROR("Error: empty XML document")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    /* Check if it is a HPCToolkit experiment file */
    if (xmlStrcmp(node->name, (const xmlChar *)"HPCToolkitExperiment")) {
        OUTPUT(("%s (%s)",
            _ERROR("Error: file is not a valid HPCToolkit experiment"), file));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    } else {
        OUTPUT_VERBOSE((2, "%s [HPCToolkit experiment file version %s]",
            _BLUE("Loading profiles"), xmlGetProp(node, "version")));
    }

    /* Perse the document */
    if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document, node->xmlChildrenNode,
        profiles, NULL, NULL, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to parse experiment file")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((4, "   (%d) %s", perfexpert_list_get_size(profiles),
        _MAGENTA("code profile(s) found")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((4, "      %s", _GREEN(profile->name)));
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }

    xmlFreeDoc(document);

    return PERFEXPERT_SUCCESS;
}

/* hpctoolkit_parser */
int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, profile_t *profile, callpath_t *parent,
    int loopdepth) {
    file_t      *file = NULL;
    metric_t    *metric = NULL;
    module_t    *module = NULL;
    callpath_t  *callpath = NULL;
    procedure_t *procedure = NULL;

    while (NULL != node) {
        /* SecCallPathProfile */
        if (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfile")) {
            PERFEXPERT_ALLOC(profile_t, profile, sizeof(profile_t));
            profile->cycles = 0.0;
            profile->instructions = 0.0;
            profile->files_by_id = NULL;
            profile->modules_by_id = NULL;
            profile->metrics_by_id = NULL;
            profile->metrics_by_name = NULL;
            profile->procedures_by_id = NULL;
            profile->id = atoi(xmlGetProp(node, "i"));
            perfexpert_list_construct(&(profile->callees));
            perfexpert_list_construct(&(profile->hotspots));
            perfexpert_list_item_construct((perfexpert_list_item_t *)profile);

            PERFEXPERT_ALLOC(char, profile->name,
                (strlen(xmlGetProp(node, "n")) + 1));
            strcpy(profile->name, xmlGetProp(node, "n"));

            OUTPUT_VERBOSE((3, "   %s [%d] (%s)", _YELLOW("profile found:"),
                profile->id, profile->name));

            /* Add to the list of profiles */
            perfexpert_list_append(profiles, (perfexpert_list_item_t *)profile);

            /* Call the call path profile parser */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, parent, loopdepth)) {
                OUTPUT(("%s (%s)", _ERROR("Error: unable to parse profile"),
                    xmlGetProp(node, "n")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Metric */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Metric")) {
            char *temp_str[3];

            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(metric_t, metric, sizeof(metric_t));
                metric->id = atoi(xmlGetProp(node, "i"));
                metric->value = 0.0;

                /* Save the entire metric name */
                PERFEXPERT_ALLOC(char, temp_str[0],
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(temp_str[0], xmlGetProp(node, "n"));

                temp_str[1] = strtok(temp_str[0], ".");
                if (NULL != temp_str[1]) {
                    /* Set the name (only the performance counter name) */
                    PERFEXPERT_ALLOC(char, metric->name,
                        (strlen(temp_str[1]) + 1));
                    perfexpert_string_replace_char(temp_str[1], ':', '_');
                    strcpy(metric->name, temp_str[1]);
                    strcpy(metric->name_md5,
                        perfexpert_md5_string(metric->name));

                    /* Save the thread information */
                    temp_str[1] = strtok(NULL, ".");

                    if (NULL != temp_str[1]) {
                        /* Set the experiment */
                        temp_str[2] = strtok(NULL, ".");
                        if (NULL != temp_str[2]) {
                            metric->experiment = atoi(temp_str[2]);
                        } else {
                            metric->experiment = 0;                    
                        }

                        /* Set the MPI rank */
                        temp_str[2] = strtok(temp_str[1], ",");
                        metric->mpi_rank = 0;

                        /* Set the thread ID */
                        temp_str[2] = strtok(NULL, ",");
                        if (NULL != temp_str[2]) {
                            temp_str[2][strlen(temp_str[2]) - 1] = '\0';
                            metric->thread = atoi(temp_str[2]);
                        }
                    }
                }
    
                OUTPUT_VERBOSE((10,
                    "   %s [%d] (%s) [rank=%d] [tid=%d] [exp=%d]",
                    _CYAN("metric"), metric->id, metric->name, metric->mpi_rank,
                    metric->thread, metric->experiment));

                /* Hash it! */
                perfexpert_hash_add_int(profile->metrics_by_id, id, metric);
            }
        }

        /* LoadModule */
        if (!xmlStrcmp(node->name, (const xmlChar *)"LoadModule")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(module_t, module, sizeof(module_t));
                module->id = atoi(xmlGetProp(node, "i"));

                PERFEXPERT_ALLOC(char, module->name,
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(module->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    module->name, &(module->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _GREEN("module"),
                    module->id, module->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->modules_by_id, id, module);
            }
        }

        /* File */
        if (!xmlStrcmp(node->name, (const xmlChar *)"File")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(file_t, file, sizeof(file_t));
                file->id = atoi(xmlGetProp(node, "i"));

                PERFEXPERT_ALLOC(char, file->name,
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(file->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    file->name, &(file->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _BLUE("file"), file->id,
                    file->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->files_by_id, id, file);
            }
        }

        /* Procedures */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Procedure")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {
                PERFEXPERT_ALLOC(procedure_t, procedure, sizeof(procedure_t));
                procedure->id = atoi(xmlGetProp(node, "i"));

                PERFEXPERT_ALLOC(char, procedure->name,
                    (strlen(xmlGetProp(node, "n")) + 1));
                strcpy(procedure->name, xmlGetProp(node, "n"));

                procedure->type = PERFEXPERT_HOTSPOT_FUNCTION;
                procedure->instructions = 0.0;
                procedure->importance = 0.0;
                procedure->variance = 0.0;
                procedure->cycles = 0.0;
                procedure->valid = PERFEXPERT_TRUE;
                procedure->line = -1;
                procedure->file = NULL;
                procedure->module = NULL;
                procedure->lcpi_by_name = NULL;
                procedure->metrics_by_id = NULL;
                procedure->metrics_by_name = NULL;
                perfexpert_list_item_construct(
                    (perfexpert_list_item_t *)procedure);
                perfexpert_list_construct(&(procedure->metrics));

                /* Add to the profile's hash of procedures and also to the list
                 * of potential hotspots
                 */
                perfexpert_hash_add_int(profile->procedures_by_id, id,
                    procedure);
                perfexpert_list_append(&(profile->hotspots),
                    (perfexpert_list_item_t *)procedure);

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _MAGENTA("procedure"),
                    procedure->id, procedure->name));
            }
        }

        /* (Pr)ocedure and (P)rocedure(F)rame, let's make the magic happen... */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"PF")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"Pr"))) {
            loopdepth = 0;
            int temp_int = 0;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "n")) || 
                (NULL == xmlGetProp(node, "f")) ||
                (NULL == xmlGetProp(node, "l")) ||
                (NULL == xmlGetProp(node, "lm")) ||
                (NULL == xmlGetProp(node, "i"))) {
                OUTPUT(("%s", _ERROR("Error: malformed callpath")));
                return PERFEXPERT_ERROR;
            }

            /* Alocate some memory and initialize it */
            PERFEXPERT_ALLOC(callpath_t, callpath, sizeof(callpath_t));
            callpath->scope = -1;
            callpath->alien = 0;
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Find file */
            temp_int = atoi(xmlGetProp(node, "f"));
            perfexpert_hash_find_int(profile->files_by_id, &temp_int, file);
            if (NULL == file) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown file")));
                return PERFEXPERT_ERROR;
            }

            /* Find module */
            temp_int = atoi(xmlGetProp(node, "lm"));
            perfexpert_hash_find_int(profile->modules_by_id, &temp_int, module);
            if (NULL == module) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown module")));
                return PERFEXPERT_ERROR;
            }

            /* Find the procedure name */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profile->procedures_by_id, &temp_int,
                procedure);
            if (NULL == procedure) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown procedure")));
                PERFEXPERT_DEALLOC(callpath);
                return PERFEXPERT_ERROR;
            } else {
                /* Paranoia checks */
                if ((procedure->line != atoi(xmlGetProp(node, "l"))) &&
                    (-1 != procedure->line)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("Error: procedure line doesn't match")));
                    return PERFEXPERT_ERROR;                
                }
                if ((file->id != atoi(xmlGetProp(node, "f"))) &&
                    (NULL != procedure->file)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("Error: procedure file doesn't match")));
                    return PERFEXPERT_ERROR;                
                }
                if ((module->id != atoi(xmlGetProp(node, "lm"))) &&
                    (NULL != procedure->module)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("Error: procedure module doesn't match")));
                    return PERFEXPERT_ERROR;                
                }

                /* Set procedure file, module, and line */
                procedure->file = file;
                procedure->module = module;
                procedure->line = atoi(xmlGetProp(node, "l"));
                perfexpert_list_construct(&(procedure->metrics));
            }

            /* Set procedure and ID */
            callpath->procedure = procedure;
            callpath->id = atoi(xmlGetProp(node, "i"));

            /* Set parent */
            if (NULL == parent) {
                callpath->parent = NULL;
                perfexpert_list_append(&(profile->callees),
                    (perfexpert_list_item_t *)callpath);
            } else {
                callpath->parent = parent;
                perfexpert_list_append(&(parent->callees),
                    (perfexpert_list_item_t *)callpath);
            }

            /* Set scope */
            if (NULL != xmlGetProp(node, "s")) {
                callpath->scope = atoi(xmlGetProp(node, "s"));
            }

            /* Alien? Terrestrial? E.T.? Home! */
            if (NULL != xmlGetProp(node, "a")) {
                callpath->alien = atoi(xmlGetProp(node, "a"));
            }

            OUTPUT_VERBOSE((10,
                "   %s i=[%d] s=[%d] a=[%d] n=[%s] f=[%s] l=[%d] lm=[%s]",
                _RED("call"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, callpath->procedure->file->shortname,
                callpath->procedure->line,
                callpath->procedure->module->shortname));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (L)oop */
        if (!xmlStrcmp(node->name, (const xmlChar *)"L")) {
            loop_t *hotspot = NULL;
            loop_t *loop = NULL;
            char *name = NULL;
            int temp_int = 0;

            loopdepth++;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "s")) ||
                (NULL == xmlGetProp(node, "i")) ||
                (NULL == xmlGetProp(node, "l"))) {
                OUTPUT(("%s", _ERROR("Error: malformed loop")));
            }

            /* Alocate callpath and initialize it */
            PERFEXPERT_ALLOC(callpath_t, callpath, sizeof(callpath_t));
            callpath->alien = parent->alien;
            callpath->parent = parent->parent;
            callpath->id = atoi(xmlGetProp(node, "i"));
            callpath->scope = atoi(xmlGetProp(node, "s"));
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Add callpath to paren'ts list of callees */
            perfexpert_list_append(&(parent->callees),
                (perfexpert_list_item_t *)callpath);

            /* Generate unique loop name (I know, it could be not unique) */
            if (PERFEXPERT_HOTSPOT_LOOP == parent->procedure->type) {
                PERFEXPERT_ALLOC(char, name, (strlen(parent->procedure->name) +
                    strlen(xmlGetProp(node, "l")) + 6));
                sprintf(name, "%s_loop%s", parent->procedure->name,
                    xmlGetProp(node, "l"));
            } else {
                PERFEXPERT_ALLOC(char, name,
                    (strlen(parent->procedure->module->shortname) +
                    strlen(parent->procedure->file->shortname) +
                    strlen(parent->procedure->name) +
                    strlen(xmlGetProp(node, "l")) + 9));
                sprintf(name, "%s_%s_%s_loop%s",
                    parent->procedure->module->shortname,
                    parent->procedure->file->shortname, parent->procedure->name,
                    xmlGetProp(node, "l"));
            }

            /* Check if this loop is already in the list of hotspots */
            hotspot = (loop_t *)perfexpert_list_get_first(&(profile->hotspots));
            while (((perfexpert_list_item_t *)hotspot !=
                &(profile->hotspots.sentinel)) && (NULL == loop)) {
                if ((0 == strcmp(hotspot->name, name)) &&
                    (PERFEXPERT_HOTSPOT_LOOP == hotspot->type)) {
                    loop = hotspot;
                }
                hotspot = (loop_t *)perfexpert_list_get_next(hotspot);
            }

            if (NULL == loop) {
                OUTPUT_VERBOSE((10, "   --- New loop found"));

                /* Allocate loop and set properties */
                PERFEXPERT_ALLOC(loop_t, loop, sizeof(loop_t));
                loop->name = name;
                loop->valid = PERFEXPERT_TRUE;
                loop->cycles = 0.0;
                loop->variance = 0.0;
                loop->importance = 0.0;
                loop->instructions = 0.0;
                loop->type = PERFEXPERT_HOTSPOT_LOOP;
                loop->id = atoi(xmlGetProp(node, "i"));
                loop->line = atoi(xmlGetProp(node, "l"));

                if (PERFEXPERT_HOTSPOT_FUNCTION == parent->procedure->type) {
                    loop->procedure = parent->procedure;
                } else {
                    loop_t *parent_loop = (loop_t *)parent->procedure;
                    loop->procedure = parent_loop->procedure;
                }

                loop->lcpi_by_name = NULL;
                loop->metrics_by_id = NULL;
                loop->metrics_by_name = NULL;
                loop->depth = loopdepth;
                perfexpert_list_construct(&(loop->metrics));
                perfexpert_list_item_construct((perfexpert_list_item_t *)loop);

                /* Add loop to list of hotspots */
                perfexpert_list_append(&(profile->hotspots),
                    (perfexpert_list_item_t *)loop);
            }

            /* Set procedure */
            callpath->procedure = (procedure_t *)loop;

            OUTPUT_VERBOSE((10,
                "   %s i=[%d] s=[%d], a=[%d] n=[%s] p=[%s] f=[%s] l=[%d] lm=[%s] d=[%d]",
                _GREEN("loop"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, loop->procedure->name,
                loop->procedure->file->shortname, callpath->procedure->line,
                loop->procedure->module->shortname, loopdepth));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (M)etric */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"M"))) {
            int temp_int = 0;
            metric_t *metric_entry = NULL;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "n")) || 
                (NULL == xmlGetProp(node, "v"))) {
                OUTPUT(("%s", _ERROR("Error: malformed metric")));
            }

            /* Are we in the callpath? */
            if (NULL == parent) {
                OUTPUT(("%s", _ERROR("Error: metric outside callpath")));
                return PERFEXPERT_ERROR;                
            }

            /* Find metric */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profile->metrics_by_id, &temp_int,
                metric_entry);
            if (NULL == metric_entry) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown metric")));
                return PERFEXPERT_ERROR;
            }

            /* Alocate metric and initialize it */
            PERFEXPERT_ALLOC(metric_t, metric, sizeof(metric_t));
            metric->id = metric_entry->id;
            metric->name = metric_entry->name;
            metric->thread = metric_entry->thread;
            metric->mpi_rank = metric_entry->mpi_rank;
            metric->experiment = metric_entry->experiment;
            metric->value = atof(xmlGetProp(node, "v"));
            strcpy(metric->name_md5, metric_entry->name_md5);
            perfexpert_list_item_construct((perfexpert_list_item_t *)metric);
            perfexpert_list_append(&(parent->procedure->metrics),
                (perfexpert_list_item_t *)metric);

            OUTPUT_VERBOSE((10,
                "   %s [%d] of [%d] (%s) (%g) [rank=%d] [tid=%d] [exp=%d]",
                _CYAN("metric value"), metric->id, parent->id, metric->name,
                metric->value, metric->mpi_rank, metric->thread,
                metric->experiment));
        }

        /* LoadModuleTable, SecHeader, SecCallPathProfileData, MetricTable */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"LoadModuleTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"SecHeader")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfileData")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"MetricTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"FileTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"ProcedureTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"S")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"C"))) {
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, parent, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parsing children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Move to the next node */
        node = node->next;
    }
    return PERFEXPERT_SUCCESS;
}

// EOF
