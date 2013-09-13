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
#include <libxml/tree.h>

/* PerfExpert headers */
#include "analyzer.h" 
#include "perfexpert_constants.h"
// #include "uthash.h"
#include "perfexpert_hash.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"

// TODO: Wrong file!
globals_t globals;

/* hpctoolkit_parse_file */
int hpctoolkit_parse_file(const char *file, profile_t *profiles) {
    xmlDocPtr   document;
    xmlNodePtr  node;
    callpath_t  *callpath;
    callpath_t  *parent = NULL;

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
        profiles, callpath, parent)) {
        OUTPUT(("%s", _ERROR("Error: unable to parse experiment file")));
        xmlFreeDoc(document);
        return PERFEXPERT_ERROR;
    }

    xmlFreeDoc(document);

    return PERFEXPERT_SUCCESS;
}

/* hpctoolkit_parser */
int hpctoolkit_parser(xmlDocPtr document, xmlNodePtr node, profile_t *profiles,
    callpath_t *callpath, callpath_t *parent) {
    metric_t    *local_metric;
    profile_t   *local_profile;
    module_t    *local_module;
    file_t      *local_file;
    procedure_t *local_procedure;

    while (NULL != node) {
        /* SecCallPathProfile */
        if (!xmlStrcmp(node->name, (const xmlChar *)"SecCallPathProfile")) {
            local_profile = (profile_t *)malloc(sizeof(profile_t));
            if (NULL == local_profile) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            local_profile->metrics = NULL;
            local_profile->procedures = NULL;
            local_profile->files = NULL;
            local_profile->modules = NULL;
            local_profile->id = atoi(xmlGetProp(node, "i"));

            local_profile->name = (char *)malloc(strlen(xmlGetProp(node, "n")) + 1);
            if (NULL == local_profile->name) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }
            bzero(local_profile->name, strlen(xmlGetProp(node, "n")) + 1);
            strcpy(local_profile->name, xmlGetProp(node, "n"));

            OUTPUT_VERBOSE((3, "   %s [%d] (%s)", _YELLOW("profile found:"),
                local_profile->id, local_profile->name));

            /* Hash it! */
            perfexpert_hash_add_int(profiles, id, local_profile);

            /* Call the call path profile parser */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, callpath, parent)) {
                OUTPUT(("%s (%s)", _ERROR("Error: unable to parse profile"),
                    xmlGetProp(node, "n")));
                return PERFEXPERT_ERROR;
            }
        }

        /* Metric */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Metric")) {
            char *temp_str[3];

            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                local_metric = (metric_t *)malloc(sizeof(metric_t));
                if (NULL == local_metric) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                local_metric->id = atoi(xmlGetProp(node, "i"));

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
                    local_metric->name = (char *)malloc(strlen(temp_str[1]) +
                        1);
                    if (NULL == local_metric->name) {
                        OUTPUT(("%s", _ERROR("Error: out of memory")));
                        return PERFEXPERT_ERROR;
                    }
                    bzero(local_metric->name, strlen(temp_str[1]) + 1);
                    strcpy(local_metric->name, temp_str[1]);

                    /* Save the thread information */
                    temp_str[1] = strtok(NULL, ".");

                    if (NULL != temp_str[1]) {
                        /* Set the sample */
                        temp_str[2] = strtok(NULL, ".");
                        if (NULL != temp_str[2]) {
                            local_metric->sample = atoi(temp_str[2]);
                        } else {
                            local_metric->sample = 1;                    
                        }

                        /* Set the thread ID */
                        temp_str[2] = strtok(temp_str[1], ",");
                        temp_str[2] = strtok(NULL, ",");
                        if (NULL != temp_str[2]) {
                            temp_str[2][strlen(temp_str[2]) - 1] = '\0';
                            local_metric->thread = atoi(temp_str[2]);
                        }
                    }
                }
    
                OUTPUT_VERBOSE((10, "   %s [%d] (%s) [thread=%d] [sample=%d]",
                    _CYAN("metric"), local_metric->id, local_metric->name,
                    local_metric->thread, local_metric->sample));

                /* Hash it! */
                perfexpert_hash_add_int(profiles->metrics, id, local_metric);
            }
        }

        /* LoadModule */
        if (!xmlStrcmp(node->name, (const xmlChar *)"LoadModule")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                local_module = (module_t *)malloc(sizeof(module_t));
                if (NULL == local_module) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                local_module->id = atoi(xmlGetProp(node, "i"));

                local_module->name = (char *)malloc(strlen(xmlGetProp(node,
                    "n")) + 1);
                if (NULL == local_module->name) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(local_module->name, strlen(xmlGetProp(node, "n")) + 1);
                strcpy(local_module->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    local_module->name, &(local_module->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _GREEN("module"),
                    local_module->id, local_module->name));

                /* Hash it! */
                perfexpert_hash_add_int(profiles->modules, id, local_module);
            }
        }

        /* File */
        if (!xmlStrcmp(node->name, (const xmlChar *)"File")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                local_file = (file_t *)malloc(sizeof(file_t));
                if (NULL == local_file) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                local_file->id = atoi(xmlGetProp(node, "i"));

                local_file->name = (char *)malloc(strlen(xmlGetProp(node, "n")) + 1);
                if (NULL == local_file->name) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(local_file->name, strlen(xmlGetProp(node, "n")) + 1);
                strcpy(local_file->name, xmlGetProp(node, "n"));

                if (PERFEXPERT_SUCCESS != perfexpert_util_filename_only(
                    local_file->name, &(local_file->shortname))) {
                    OUTPUT(("%s", _ERROR("Error: unable to find shortname")));
                    return PERFEXPERT_ERROR;
                }

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _BLUE("file"),
                    local_file->id, local_file->name));

                /* Hash it! */
                perfexpert_hash_add_int(profiles->files, id, local_file);
            }
        }

        /* Procedures */
        if (!xmlStrcmp(node->name, (const xmlChar *)"Procedure")) {
            if ((NULL != xmlGetProp(node, "n")) && (xmlGetProp(node, "i"))) {

                local_procedure = (procedure_t *)malloc(sizeof(procedure_t));
                if (NULL == local_procedure) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                local_procedure->id = atoi(xmlGetProp(node, "i"));

                local_procedure->name = (char *)malloc(strlen(xmlGetProp(node,
                    "n")) + 1);
                if (NULL == local_procedure->name) {
                    OUTPUT(("%s", _ERROR("Error: out of memory")));
                    return PERFEXPERT_ERROR;
                }
                bzero(local_procedure->name, strlen(xmlGetProp(node, "n")) + 1);
                strcpy(local_procedure->name, xmlGetProp(node, "n"));

                OUTPUT_VERBOSE((10, "   %s [%d] (%s)", _MAGENTA("procedure"),
                    local_procedure->id, local_procedure->name));

                /* Hash it! */
                perfexpert_hash_add_int(profiles->procedures, id,
                    local_procedure);
            }
        }

        /* (Pr)ocedure and (P)rocedure(F)rame, let's make the magic happen... */
        if ((!xmlStrcmp(node->name, (const xmlChar *)"PF")) ||
            (!xmlStrcmp(node->name, (const xmlChar *)"Pr"))) {
            int temp_int;

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
            callpath->metrics = NULL;
            callpath->procedure = NULL;
            callpath->type = PERFEXPERT_FUNCTION;
            perfexpert_list_construct(&(callpath->calls));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

            /* Find procedure */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profiles->procedures, &temp_int,
                local_procedure);
            if (NULL == local_procedure) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown procedure")));
                return PERFEXPERT_ERROR;
            } else {
                callpath->procedure = local_procedure;
            }

            /* Find module */
            temp_int = atoi(xmlGetProp(node, "lm"));
            perfexpert_hash_find_int(profiles->modules, &temp_int,
                local_module);
            if (NULL == local_module) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown module")));
                return PERFEXPERT_ERROR;
            } else {
                callpath->module = local_module;
            }

            /* Find file */
            temp_int = atoi(xmlGetProp(node, "f"));
            perfexpert_hash_find_int(profiles->files, &temp_int, local_file);
            if (NULL == local_file) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown file")));
                return PERFEXPERT_ERROR;
            } else {
                callpath->file = local_file;
            }

            /* Set parent */
            if (NULL == parent) {
                callpath->parent = NULL;
            } else {
                callpath->parent = parent;
                perfexpert_list_append(&(parent->calls),
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
                "   %s i=[%d], s=[%d], a=[%d], n=[%s], f=[%s], l=[%d] lm=[%s]",
                _RED("call"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, callpath->file->shortname,
                callpath->line, callpath->module->shortname));

            /* Now I am parent */
            parent = callpath;

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, callpath, parent)) {
                OUTPUT(("%s", _ERROR("Error: unable to parse children")));
                return PERFEXPERT_ERROR;
            }

            /* Now I am child again */
            parent = callpath->parent;
        }

        /* (L)oop */
        if (!xmlStrcmp(node->name, (const xmlChar *)"L")) {
            int temp_int;

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
            callpath->metrics = NULL;
            callpath->file = parent->file;
            callpath->alien = parent->alien;
            callpath->module = parent->module;
            callpath->parent = parent->parent;
            callpath->procedure = parent->procedure;
            callpath->type = PERFEXPERT_LOOP;
            perfexpert_list_construct(&(callpath->calls));
            perfexpert_list_item_construct((perfexpert_list_item_t *)callpath);

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
                "   %s i=[%d], s=[%d], a=[%d], n=[%s], f=[%s], l=[%d] lm=[%s]",
                _RED("loop"), callpath->id, callpath->scope, callpath->alien,
                callpath->procedure->name, callpath->file->shortname,
                callpath->line, callpath->module->shortname));

            /* Keep Walking! (Johnny Walker) */
            if (PERFEXPERT_SUCCESS != hpctoolkit_parser(document,
                node->xmlChildrenNode, profiles, callpath, parent)) {
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
            if (NULL == callpath) {
                OUTPUT(("%s", _ERROR("Error: metric outside callpath")));
                return PERFEXPERT_ERROR;                
            }

            /* Alocate some memory and initialize it */
            local_metric = (metric_t *)malloc(sizeof(metric_t));
            if (NULL == local_metric) {
                OUTPUT(("%s", _ERROR("Error: out of memory")));
                return PERFEXPERT_ERROR;
            }

            /* Find metric */
            temp_int = atoi(xmlGetProp(node, "n"));
            perfexpert_hash_find_int(profiles->metrics, &temp_int,
                metric_entry);
            if (NULL == metric_entry) {
                OUTPUT_VERBOSE((10, "%s", _ERROR("Error: unknown metric")));
                return PERFEXPERT_ERROR;
            }

            local_metric->id     = metric_entry->id;
            local_metric->name   = metric_entry->name;
            local_metric->thread = metric_entry->thread;
            local_metric->sample = metric_entry->sample;
            perfexpert_hash_find_int(callpath->metrics, &temp_int,
                metric_entry);
            perfexpert_hash_add_int(callpath->metrics, id, local_metric);

            /* Set value */
            if (NULL != xmlGetProp(node, "v")) {
                local_metric->value = atof(xmlGetProp(node, "v"));
            }

            OUTPUT_VERBOSE((10, "   %s [%d] (%s) (%g) [thread=%d] [sample=%d]",
                _CYAN("metric value"), local_metric->id, local_metric->name,
                local_metric->value, local_metric->thread,
                local_metric->sample));
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
                node->xmlChildrenNode, profiles, callpath, parent)) {
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
