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
#include "perfexpert_constants.h"
#include "perfexpert_hash.h"
#include "perfexpert_output.h"
#include "perfexpert_md5.h"
#include "perfexpert_util.h"

// TODO: Wrong file!
globals_t globals;

/* hpctoolkit_parse_file */
int hpctoolkit_parse_file(const char *file, perfexpert_list_t *profiles) {
    xmlDocPtr   document;
    xmlNodePtr  node;
    profile_t *profile = NULL;

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
        OUTPUT_VERBOSE((2, "%s (%s)",
            _YELLOW("Parsing HPCToolkit experiment file"),
            xmlGetProp(node, "version")));
    }

    /* Perse the document */
    if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document, node->xmlChildrenNode,
        profiles, NULL, NULL, 0)) {
        OUTPUT(("%s", _ERROR("Error: unable to parse experiment file")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((4, "%d %s", perfexpert_list_get_size(profiles),
        _GREEN("code profile(s) found")));

    profile = (profile_t *)perfexpert_list_get_first(profiles);
    while ((perfexpert_list_item_t *)profile != &(profiles->sentinel)) {
        OUTPUT_VERBOSE((4, "   %s", profile->name));
        profile = (profile_t *)perfexpert_list_get_next(profile);
    }

    xmlFreeDoc(document);

    return PERFEXPERT_SUCCESS;
}

/* hpctoolkit_parser */
int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, profile_t *profile, callpath_t *parent,
    int loopdepth) {
    file_t      *file;
    metric_t    *metric;
    module_t    *module;
    callpath_t  *callpath;
    procedure_t *procedure;

    while (NULL != node) {
        /* SecCallPathProfile */
        if (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfile")) {
            profile = (profile_t *)malloc(sizeof(profile_t));
            if (NULL == profile) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            profile->metrics_by_id = NULL;
            profile->metrics_by_name = NULL;
            profile->procedures = NULL;
            profile->files = NULL;
            profile->modules = NULL;
            profile->id = atoi(xmlGetProp(node, "i"));
            perfexpert_list_construct(&(profile->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)profile);

            profile->name = (char *)malloc(strlen(xmlGetProp(node, "n")) + 1);
            if (NULL == profile->name) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(profile->name, strlen(xmlGetProp(node, "n")) + 1);
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

                metric = (metric_t *)malloc(sizeof(metric_t));
                if (NULL == metric) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                metric->id = atoi(xmlGetProp(node, "i"));

                /* Save the entire metric name */
                temp_str[0] = (char *)malloc(strlen(xmlGetProp(node, "n")) + 1);
                if (NULL == temp_str[0]) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(temp_str[0], strlen(xmlGetProp(node, "n")) + 1);
                strcpy(temp_str[0], xmlGetProp(node, "n"));

                temp_str[1] = strtok(temp_str[0], ".");
                if (NULL != temp_str[1]) {
                    /* Set the name (only the performance counter name) */
                    metric->name = (char *)malloc(strlen(temp_str[1]) + 1);
                    if (NULL == metric->name) {
                        OUTPUT(("%s", _ERROR("Error: out of memory")));
                        return PERFEXPERT_ERROR;
                    }
                    bzero(metric->name, strlen(temp_str[1]) + 1);
                    strcpy(metric->name, temp_str[1]);

                    /* Save the thread information */
                    temp_str[1] = strtok(NULL, ".");

                    // TODO: This is bullshit! Instead of adding different experiments, just check the variability
                    if (NULL != temp_str[1]) {
                        /* Set the experiment */
                        temp_str[2] = strtok(NULL, ".");
                        if (NULL != temp_str[2]) {
                            metric->experiment = atoi(temp_str[2]);
                        } else {
                            metric->experiment = 1;                    
                        }

                        /* Set the thread ID */
                        temp_str[2] = strtok(temp_str[1], ",");
                        temp_str[2] = strtok(NULL, ",");
                        if (NULL != temp_str[2]) {
                            temp_str[2][strlen(temp_str[2]) - 1] = '\0';
                            metric->thread = atoi(temp_str[2]);
                        }
                    }
                }
    
                OUTPUT_VERBOSE((10, "   %s [%d] (%s) [thread=%d] [exp=%d]",
                    _CYAN("metric"), metric->id, metric->name, metric->thread,
                    metric->experiment));

                /* Hash it! */
                perfexpert_hash_add_int(profile->metrics_by_id, id, metric);
            }
        }

        /* LoadModule */
        if (!xmlStrcmp(node->name, (const xmlChar *)"LoadModule")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                module = (module_t *)malloc(sizeof(module_t));
                if (NULL == module) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                module->id = atoi(xmlGetProp(node, "i"));

                module->name = (char *)malloc(strlen(xmlGetProp(node,
                    "n")) + 1);
                if (NULL == module->name) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(module->name, strlen(xmlGetProp(node, "n")) + 1);
                strcpy(module->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    module->name, &(module->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _GREEN("module"),
                    module->id, module->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->modules, id, module);
            }
        }

        /* File */
        if (!xmlStrcmp(node->name, (const xmlChar *)"File")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                file = (file_t *)malloc(sizeof(file_t));
                if (NULL == file) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                file->id = atoi(xmlGetProp(node, "i"));

                file->name = (char *)malloc(strlen(xmlGetProp(node, "n")) + 1);
                if (NULL == file->name) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(file->name, strlen(xmlGetProp(node, "n")) + 1);
                strcpy(file->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    file->name, &(file->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _BLUE("file"), file->id,
                    file->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->files, id, file);
            }
        }

        /* Procedures */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Procedure")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                procedure = (procedure_t *)malloc(sizeof(procedure_t));
                if (NULL == procedure) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                procedure->id = atoi(xmlGetProp(node, "i"));

                procedure->name = (char *)malloc(strlen(xmlGetProp(node, "n")) +
                    1);
                if (NULL == procedure->name) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(procedure->name, strlen(xmlGetProp(node, "n")) + 1);
                strcpy(procedure->name, xmlGetProp(node, "n"));

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _MAGENTA("procedure"),
                    procedure->id, procedure->name));

                /* Hash it! */
                perfexpert_hash_add_int(profile->procedures, id, procedure);
            }
        }

        /* (Pr)ocedure and (P)rocedure(F)rame, let's make the magic happen... */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"PF")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"Pr"))) {
            int temp_int;
            loopdepth = 0;

            /* Just to be sure it will work */
            if ((NULL == xmlGetProp(node, "n")) || 
                (NULL == xmlGetProp(node, "f")) ||
                (NULL == xmlGetProp(node, "lm")) ||
                (NULL == xmlGetProp(node, "i"))) {
                OUTPUT(("%s", _ERROR("Error: malformed callpath")));
            }

            /* Alocate some memory and initialize it */
            callpath = (callpath_t *)malloc(sizeof(callpath_t));
            if (NULL == callpath) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            callpath->id = -1;
            callpath->line = -1;
            callpath->scope = -1;
            callpath->alien = 0;
            callpath->file = NULL;
            callpath->module = NULL;
            callpath->procedure = NULL;
            callpath->metrics_by_id = NULL;
            callpath->metrics_by_name = NULL;
            callpath->type = PERFEXPERT_FUNCTION;
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Find procedure */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profile->procedures, &temp_int, procedure);
            if (NULL == procedure) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown procedure")));
                return PERFEXPERT_ERROR;
            } else {
                callpath->procedure = procedure;
            }

            /* Find module */
            temp_int = atoi(xmlGetProp(node, "lm"));
            perfexpert_hash_find_int(profile->modules, &temp_int, module);
            if (NULL == module) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown module")));
                return PERFEXPERT_ERROR;
            } else {
                callpath->module = module;
            }

            /* Find file */
            temp_int = atoi(xmlGetProp(node, "f"));
            perfexpert_hash_find_int(profile->files, &temp_int, file);
            if (NULL == file) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown file")));
                return PERFEXPERT_ERROR;
            } else {
                callpath->file = file;
            }

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

            /* Set ID */
            if (NULL != xmlGetProp(node, "i")) {
                callpath->id = atoi(xmlGetProp(node, "i"));
            }

            /* Set line */
            if (NULL != xmlGetProp(node, "l")) {
                callpath->line = atoi(xmlGetProp(node, "l"));
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
                callpath->procedure->name, callpath->file->shortname,
                callpath->line, callpath->module->shortname));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (L)oop */
        if (!xmlStrcmp(node->name, (const xmlChar *)"L")) {
            int temp_int;
            loopdepth++;

            /* Just to be sure it will work */
            if (NULL == xmlGetProp(node, "s")) {
                OUTPUT(("%s", _ERROR("Error: malformed loop")));
            }

            /* Alocate some memory and initialize it */
            callpath = (callpath_t *)malloc(sizeof(callpath_t));
            if (NULL == callpath) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            callpath->id = -1;
            callpath->line = -1;
            callpath->scope = -1;
            callpath->metrics_by_id = NULL;
            callpath->metrics_by_name = NULL;
            callpath->file = parent->file;
            callpath->alien = parent->alien;
            callpath->module = parent->module;
            callpath->parent = parent->parent;
            callpath->procedure = parent->procedure;
            callpath->type = PERFEXPERT_LOOP;
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            perfexpert_list_append(&(parent->callees),
                (perfexpert_list_item_t *)callpath);

            /* Set ID */
            if (NULL != xmlGetProp(node, "i")) {
                callpath->id = atoi(xmlGetProp(node, "i"));
            }

            /* Set line */
            if (NULL != xmlGetProp(node, "l")) {
                callpath->line = atoi(xmlGetProp(node, "l"));
            }

            /* Set scope */
            if (NULL != xmlGetProp(node, "s")) {
                callpath->scope = atoi(xmlGetProp(node, "s"));
            }

            OUTPUT_VERBOSE((10,
                "   %s i=[%d] s=[%d], a=[%d] n=[%s] f=[%s] l=[%d] lm=[%s] d=[%d]",
                _GREEN("loop"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, callpath->file->shortname,
                callpath->line, callpath->module->shortname, loopdepth));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (M)etric */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"M"))) {
            int temp_int;
            metric_t *metric_entry;

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

            /* Alocate some memory and initialize it */
            metric = (metric_t *)malloc(sizeof(metric_t));
            if (NULL == metric) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
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

            metric->id         = metric_entry->id;
            metric->name       = metric_entry->name;
            metric->thread     = metric_entry->thread;
            metric->experiment = metric_entry->experiment;
            bzero(metric->name_md5, 33);
            strcpy(metric->name_md5, perfexpert_md5_string(metric_entry->name));

            perfexpert_hash_add_int(parent->metrics_by_id, id, metric);
            perfexpert_hash_add_str(parent->metrics_by_name, name_md5, metric);

            /* Set value */
            if (NULL != xmlGetProp(node, "v")) {
                metric->value = atof(xmlGetProp(node, "v"));
            }

            OUTPUT_VERBOSE((10,
                "   %s [%d] of [%d] (%s) (%g) [tid=%d] [exp=%d] [md5=%s]",
                _CYAN("metric value"), metric->id, parent->id, metric->name,
                metric->value, metric->thread, metric->experiment,
                metric->name_md5));
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
