/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 *
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifdef __cplusplus
extern "C" {
#endif

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>

/* Module headers */
#include "hpctoolkit.h"
#include "hpctoolkit_types.h"
#include "hpctoolkit_parser.h"

/* PerfExpert common headers */
#include "common/perfexpert_alloc.h"
#include "common/perfexpert_constants.h"
#include "common/perfexpert_hash.h"
#include "common/perfexpert_list.h"
#include "common/perfexpert_md5.h"
#include "common/perfexpert_output.h"
#include "common/perfexpert_string.h"
#include "common/perfexpert_util.h"

/* module_parse_file */
int parse_file(const char *file) {
    xmlChar    *xmlchar;
    xmlDocPtr  document;
    xmlNodePtr node;
    hpctoolkit_profile_t  *p = NULL;

    /* Open file */
    if (NULL == (document = xmlParseFile(file))) {
        OUTPUT(("%s", _ERROR("malformed XML document")));
        return PERFEXPERT_ERROR;
    }

    /* Check if it is not empty */
    if (NULL == (node = xmlDocGetRootElement(document))) {
        OUTPUT(("%s", _ERROR("empty XML document")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    /* Check if it is a HPCToolkit experiment file */
    if (xmlStrcmp(node->name, (const xmlChar *)"HPCToolkitExperiment")) {
        OUTPUT(("%s (%s)", _ERROR("not a valid HPCToolkit experiment"), file));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    } else {
        if (NULL != (xmlchar = xmlGetProp(node, "version"))) {
            OUTPUT_VERBOSE((2, "%s HPCToolkit experiment file version %s [%s]",
                _BLUE("Parsing file:"), xmlchar, file));
            xmlFree(xmlchar);
        }
    }

    /* Parse the document */
    if (PERFEXPERT_SUCCESS != parse_profile(document, node->xmlChildrenNode,
        &(myself_module.profiles), NULL, NULL, 0)) {
        OUTPUT(("%s", _ERROR("unable to parse experiment file")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    OUTPUT_VERBOSE((4, "(%d) %s",
        perfexpert_list_get_size(&(myself_module.profiles)),
        _MAGENTA("code profile(s) found")));

    perfexpert_list_for(p, &(myself_module.profiles), hpctoolkit_profile_t) {
        OUTPUT_VERBOSE((4, "      %s", _GREEN(p->name)));
    }

    xmlFreeDoc(document);

    return PERFEXPERT_SUCCESS;
}

/* parse_profile */
static int parse_profile(xmlDocPtr document, xmlNodePtr node,
    perfexpert_list_t *profiles, hpctoolkit_profile_t *profile,
    hpctoolkit_callpath_t *parent, int loopdepth) {
    int f, i, l, lm, n;
    xmlChar *xmlchar;
    float v;
    char *temp = NULL;

    hpctoolkit_file_t *file = NULL;
    hpctoolkit_metric_t *metric = NULL;
    hpctoolkit_module_t *module = NULL;
    hpctoolkit_callpath_t *callpath = NULL;
    hpctoolkit_procedure_t *procedure = NULL;

    while (NULL != node) {
        f = -1; i = -1; l = -1; lm = -1; n = -1;

        /* SecCallPathProfile */
        if (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfile")) {
            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed SecCallPathProfile")));
                return PERFEXPERT_ERROR;
            }
            if (NULL == (xmlchar = xmlGetProp(node, "n"))) {
                OUTPUT(("%s", _ERROR("malformed SecCallPathProfile")));
                xmlFree(xmlchar);
                return PERFEXPERT_ERROR;
            }

            PERFEXPERT_ALLOC(hpctoolkit_profile_t, profile,
                sizeof(hpctoolkit_profile_t));
            profile->files_by_id = NULL;
            profile->modules_by_id = NULL;
            profile->metrics_by_id = NULL;
            profile->procedures_by_id = NULL;
            profile->id = i;
            perfexpert_list_construct(&(profile->callees));
            perfexpert_list_construct(&(profile->hotspots));
            perfexpert_list_item_construct((perfexpert_list_item_t *)profile);

            PERFEXPERT_ALLOC(char, profile->name, (strlen(xmlchar) + 1));
            strcpy(profile->name, xmlchar);
            xmlFree(xmlchar);

            OUTPUT_VERBOSE((3, "%s [%d] (%s)", _YELLOW("profile found:"),
                profile->id, profile->name));

            /* Add to the list of profiles */
            perfexpert_list_append(profiles, (perfexpert_list_item_t *)profile);

            /* Call the call path profile parser */
            if (PERFEXPERT_SUCCESS != parse_profile(document,
                node->xmlChildrenNode, profiles, profile, parent, loopdepth)) {
                OUTPUT(("%s (%d)", _ERROR("unable to parse profile"), n));
                return PERFEXPERT_ERROR;
            }
        }

        /* Metric */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Metric")) {
            char *temp_str[3];

            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed Metric")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "n"))) {
                /* Save the entire metric name */
                PERFEXPERT_ALLOC(char, temp_str[0], (strlen(xmlchar) + 1));
                strcpy(temp_str[0], xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed Metric")));
                return PERFEXPERT_ERROR;
            }

            PERFEXPERT_ALLOC(hpctoolkit_metric_t, metric,
                sizeof(hpctoolkit_metric_t));
            metric->id = i;
            metric->value = 0.0;

            temp_str[1] = strtok(temp_str[0], ".");
            if (NULL != temp_str[1]) {
                /* Set the name (only the performance counter name) */
                PERFEXPERT_ALLOC(char, metric->name,
                    (strlen(temp_str[1]) + 1));
                perfexpert_string_replace_char(temp_str[1], ':', '.');
                strcpy(metric->name, temp_str[1]);
                strcpy(metric->name_md5, perfexpert_md5_string(metric->name));

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
                    metric->mpi_rank = atoi(temp_str[2]);// 0;

                    /* Set the thread ID */
                    temp_str[2] = strtok(NULL, ",");
                    if (NULL != temp_str[2]) {
                        temp_str[2][strlen(temp_str[2]) - 1] = '\0';
                        metric->thread = atoi(temp_str[2]);
                    }
                }
            }
            PERFEXPERT_DEALLOC(temp_str[0]);

            OUTPUT_VERBOSE((10, "%s [%d] (%s) [rank=%d] [tid=%d] [exp=%d]",
                _CYAN("metric"), metric->id, metric->name, metric->mpi_rank,
                metric->thread, metric->experiment));

            /* Hash it! */
            perfexpert_hash_add_int(profile->metrics_by_id, id, metric);
        }

        /* LoadModule */
        if (!xmlStrcmp(node->name, (const xmlChar *)"LoadModule")) {
            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed LoadModule")));
                return PERFEXPERT_ERROR;
            }
            if (NULL == (xmlchar = xmlGetProp(node, "n"))) {
                xmlFree(xmlchar);
                OUTPUT(("%s", _ERROR("malformed LoadModule")));
                return PERFEXPERT_ERROR;
            }

            PERFEXPERT_ALLOC(hpctoolkit_module_t, module,
                sizeof(hpctoolkit_module_t));
            module->id = i;

            PERFEXPERT_ALLOC(char, module->name, (strlen(xmlchar) + 1));
            strcpy(module->name, xmlchar);
            xmlFree(xmlchar);

            if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                module->name, &(module->shortname))) {
                OUTPUT(("%s", _ERROR("unable to find shortname")));
                return PERFEXPERT_ERROR;
            }

            OUTPUT_VERBOSE((10, "%s [%d] (%s)", _GREEN("module"), module->id,
                module->name));

            /* Hash it! */
            perfexpert_hash_add_int(profile->modules_by_id, id, module);
        }

        /* File */
        if (!xmlStrcmp(node->name, (const xmlChar *)"File")) {
            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed File")));
                return PERFEXPERT_ERROR;
            }
            if (NULL == (xmlchar = xmlGetProp(node, "n"))) {
                xmlFree(xmlchar);
                OUTPUT(("%s", _ERROR("malformed File")));
                return PERFEXPERT_ERROR;
            }

            PERFEXPERT_ALLOC(hpctoolkit_file_t, file,
                sizeof(hpctoolkit_file_t));
            file->id = i;

            /* Strip off the prefix HPCTollkit adds to some files (dangerous) */
            if (0 == strncmp("./src", xmlchar, 5)) {
                temp = xmlchar + 5;
            } else {
                temp = xmlchar;
            }

            PERFEXPERT_ALLOC(char, file->name, (strlen(temp) + 1));
            strcpy(file->name, temp);
            xmlFree(xmlchar);

            if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                file->name, &(file->shortname))) {
                OUTPUT(("%s", _ERROR("unable to find shortname")));
                return PERFEXPERT_ERROR;
            }

            OUTPUT_VERBOSE((10, "%s [%d] (%s)", _BLUE("file"), file->id,
                file->name));

            /* Hash it! */
            perfexpert_hash_add_int(profile->files_by_id, id, file);
        }

        /* Procedures */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Procedure")) {
            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed Procedure")));
                return PERFEXPERT_ERROR;
            }
            if (NULL == (xmlchar = xmlGetProp(node, "n"))) {
                xmlFree(xmlchar);
                OUTPUT(("%s", _ERROR("malformed Procedure")));
                return PERFEXPERT_ERROR;
            }

            PERFEXPERT_ALLOC(hpctoolkit_procedure_t, procedure,
                sizeof(hpctoolkit_procedure_t));
            procedure->id = i;

            PERFEXPERT_ALLOC(char, procedure->name, (strlen(xmlchar) + 1));
            strcpy(procedure->name, xmlchar);
            xmlFree(xmlchar);

            procedure->type = PERFEXPERT_HOTSPOT_FUNCTION;
            procedure->line = -1;
            procedure->file = NULL;
            procedure->module = NULL;
            procedure->metrics_by_id = NULL;
            perfexpert_list_item_construct((perfexpert_list_item_t *)procedure);
            perfexpert_list_construct(&(procedure->metrics));

            /* Add to profile's hash of procedures and to list hotspots */
            perfexpert_hash_add_int(profile->procedures_by_id, id, procedure);
            perfexpert_list_append(&(profile->hotspots),
                (perfexpert_list_item_t *)procedure);

            OUTPUT_VERBOSE((10, "%s [%d] (%s)", _MAGENTA("procedure"),
                procedure->id, procedure->name));
        }

        /* (Pr)ocedure and (P)rocedure(F)rame, let's make the magic happen... */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"PF")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"Pr"))) {
            loopdepth = 0;

            /* Just to be sure it will work */
            if (NULL != (xmlchar = xmlGetProp(node, "f"))) {
                f = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed PF/Pr (callpath)")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed PF/Pr (callpath)")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "l"))) {
                l = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed PF/Pr (callpath)")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "lm"))) {
                lm = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed PF/Pr (callpath)")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "n"))) {
                n = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed PF/Pr (callpath)")));
                return PERFEXPERT_ERROR;
            }

            /* Alocate some memory and initialize it */
            PERFEXPERT_ALLOC(hpctoolkit_callpath_t, callpath,
                sizeof(hpctoolkit_callpath_t));
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Find file */
            perfexpert_hash_find_int(profile->files_by_id, &f, file);
            if (NULL == file) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("unknown file")));
                return PERFEXPERT_ERROR;
            }

            /* Find module */
            perfexpert_hash_find_int(profile->modules_by_id, &lm, module);
            if (NULL == module) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("unknown module")));
                return PERFEXPERT_ERROR;
            }

            /* Find the procedure name */
            perfexpert_hash_find_int(profile->procedures_by_id, &n, procedure);
            if (NULL == procedure) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("unknown procedure")));
                PERFEXPERT_DEALLOC(callpath);
                return PERFEXPERT_ERROR;
            } else {
                /* Paranoia checks */
                if ((procedure->line != l) && (-1 != procedure->line)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("procedure line doesn't match")));
                    return PERFEXPERT_ERROR;
                }
                if ((file->id != f) && (NULL != procedure->file)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("procedure file doesn't match")));
                    return PERFEXPERT_ERROR;
                }
                if ((module->id != lm) && (NULL != procedure->module)) {
                    OUTPUT_VERBOSE((10, "%s",
                        _ERROR("procedure module doesn't match")));
                    return PERFEXPERT_ERROR;
                }

                /* Set procedure file, module, and line */
                procedure->line = l;
                procedure->file = file;
                procedure->module = module;
                perfexpert_list_construct(&(procedure->metrics));
            }

            /* Set procedure and ID */
            callpath->procedure = procedure;
            callpath->id = i;

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

            OUTPUT_VERBOSE((10, "%s i=[%d] n=[%s] f=[%s] l=[%d] lm=[%s]",
                _RED("call"), callpath->id, callpath->procedure->name,
                callpath->procedure->file->shortname, callpath->procedure->line,
                callpath->procedure->module->shortname));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != parse_profile(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (L)oop */
        if (!xmlStrcmp(node->name, (const xmlChar *)"L")) {
            hpctoolkit_loop_t *hotspot = NULL, *loop = NULL;
            char *name = NULL;

            loopdepth++;

            /* Just to be sure it will work */
            if (NULL != (xmlchar = xmlGetProp(node, "i"))) {
                i = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed L (loop)")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "l"))) {
                l = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed L (loop)")));
                return PERFEXPERT_ERROR;
            }

            if (NULL == (xmlchar = xmlGetProp(node, "s"))) {
                OUTPUT(("%s", _ERROR("malformed loop")));
                return PERFEXPERT_ERROR;
            } else {
                xmlFree(xmlchar);
            }

            /* Alocate callpath and initialize it */
            PERFEXPERT_ALLOC(hpctoolkit_callpath_t, callpath,
                sizeof(hpctoolkit_callpath_t));
            callpath->parent = parent->parent;
            callpath->id = i;
            perfexpert_list_construct(&(callpath->callees));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Add callpath to paren'ts list of callees */
            perfexpert_list_append(&(parent->callees),
                (perfexpert_list_item_t *)callpath);

            /* Generate unique loop name (I know, it could be not unique) */
            if (PERFEXPERT_HOTSPOT_LOOP == parent->procedure->type) {
                PERFEXPERT_ALLOC(char, name,
                    (strlen(parent->procedure->name) + 15));
                sprintf(name, "%s_loop%d", parent->procedure->name, l);
            } else {
                PERFEXPERT_ALLOC(char, name,
                    (strlen(parent->procedure->module->shortname) +
                    strlen(parent->procedure->file->shortname) +
                    strlen(parent->procedure->name) + 15));
                sprintf(name, "%s_%s_%s_loop%d",
                    parent->procedure->module->shortname,
                    parent->procedure->file->shortname,
                    parent->procedure->name, l);
            }

            /* Check if this loop is already in the list of hotspots */
            perfexpert_list_for(hotspot, &(profile->hotspots),
                hpctoolkit_loop_t) {
                if ((0 == strcmp(hotspot->name, name)) &&
                    (PERFEXPERT_HOTSPOT_LOOP == hotspot->type)) {
                    loop = hotspot;
                }
            }

            if (NULL == loop) {
                OUTPUT_VERBOSE((10, "--- New loop found"));

                /* Allocate loop and set properties */
                PERFEXPERT_ALLOC(hpctoolkit_loop_t, loop,
                    sizeof(hpctoolkit_loop_t));
                loop->name = name;
                loop->type = PERFEXPERT_HOTSPOT_LOOP;
                loop->id = i;
                loop->line = l;

                if (PERFEXPERT_HOTSPOT_FUNCTION == parent->procedure->type) {
                    loop->procedure = parent->procedure;
                } else {
                    hpctoolkit_loop_t *parent_loop =
                        (hpctoolkit_loop_t *)parent->procedure;
                    loop->procedure = parent_loop->procedure;
                }

                loop->metrics_by_id = NULL;
                loop->depth = loopdepth;
                perfexpert_list_construct(&(loop->metrics));
                perfexpert_list_item_construct((perfexpert_list_item_t *)loop);

                /* Add loop to list of hotspots */
                perfexpert_list_append(&(profile->hotspots),
                    (perfexpert_list_item_t *)loop);
            }

            /* Set procedure */
            callpath->procedure = (hpctoolkit_procedure_t *)loop;

            OUTPUT_VERBOSE((10, "%s i=[%d] n=[%s] p=[%s] f=[%s] l=[%d] lm=[%s] "
                "d=[%d]", _GREEN("loop"), callpath->id,
                callpath->procedure->name, loop->procedure->name,
                loop->procedure->file->shortname, callpath->procedure->line,
                loop->procedure->module->shortname, loopdepth));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != parse_profile(document,
                node->xmlChildrenNode, profiles, profile, callpath, loopdepth)) {
                OUTPUT(("%s", _ERROR("unable to parse children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* (M)etric */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"M"))) {
            hpctoolkit_metric_t *metric_entry = NULL;
            int temp_int = 0;

            /* Just to be sure it will work */
            if (NULL != (xmlchar = xmlGetProp(node, "n"))) {
                n = atoi(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed M (metric)")));
                return PERFEXPERT_ERROR;
            }
            if (NULL != (xmlchar = xmlGetProp(node, "v"))) {
                v = atof(xmlchar);
                xmlFree(xmlchar);
            } else {
                OUTPUT(("%s", _ERROR("malformed M (metric)")));
                return PERFEXPERT_ERROR;
            }

            /* Are we in the callpath? */
            if (NULL == parent) {
                OUTPUT(("%s", _ERROR("metric outside callpath")));
                return PERFEXPERT_ERROR;
            }

            /* Find metric */
            perfexpert_hash_find_int(profile->metrics_by_id, &n, metric_entry);
            if (NULL == metric_entry) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("unknown metric")));
                return PERFEXPERT_ERROR;
            }

            /* Alocate metric and initialize it */
            PERFEXPERT_ALLOC(hpctoolkit_metric_t, metric,
                sizeof(hpctoolkit_metric_t));
            metric->id = metric_entry->id;
            metric->name = metric_entry->name;
            metric->thread = metric_entry->thread;
            metric->mpi_rank = metric_entry->mpi_rank;
            metric->experiment = metric_entry->experiment;
            metric->value = v;
            strcpy(metric->name_md5, metric_entry->name_md5);
            perfexpert_list_item_construct((perfexpert_list_item_t *)metric);
            perfexpert_list_append(&(parent->procedure->metrics),
                (perfexpert_list_item_t *)metric);

            OUTPUT_VERBOSE((10, "%s [%d] of [%d] (%s) (%g) [rank=%d] [tid=%d] "
                "[exp=%d]", _CYAN("metric value"), metric->id, parent->id,
                metric->name, metric->value, metric->mpi_rank, metric->thread,
                metric->experiment));
        }

        /* LoadModuleTable, SecHeader, SecCallPathProfileData, MetricTable... */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"LoadModuleTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"SecHeader")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfileData")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"MetricTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"FileTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"ProcedureTable")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"S")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"C"))) {
            if (PERFEXPERT_SUCCESS != parse_profile(document,
                node->xmlChildrenNode, profiles, profile, parent, loopdepth)) {
                OUTPUT(("%s", _ERROR("unable to parsing children")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Move to next node */
        node = node->next;
    }
    return PERFEXPERT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

// EOF
