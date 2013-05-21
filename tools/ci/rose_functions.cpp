/*
 * Copyright (c) 2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
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
 * Author: Leonardo Fialho
 *
 * $HEADER$
 */

/* System standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Utility headers */
#include <rose.h>
#include <sage3.h>

/* sys/stat.h (and hence fcntl.h) needs to be included after rose.h */
#include <fcntl.h>

/* PerfExpert headers */
#include "config.h"
#include "ci.h"
#include "perfexpert_output.h"
#include "perfexpert_util.h"
#include "rose_functions.h"

using namespace std;
using namespace SageInterface;
using namespace SageBuilder;

extern globals_t globals; // globals was defined on 'recommender.c'

SgProject *userProject;

// TODO: it will be nice and polite to add some ROSE_ASSERT to this code

int open_rose(char *source_file) {
    char **files = NULL;

    OUTPUT_VERBOSE((7, "=== %s", _BLUE((char *)"Opening Rose")));

    /* Fill 'files', aka **argv */
    files = (char **)malloc(sizeof(char *) * 3);
    files[0] = (char *)malloc(sizeof("perfexpert_ci") + 1);
    if (NULL == files[0]) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(files[0], sizeof("perfexpert_ci") + 1);
    snprintf(files[0], sizeof("perfexpert_ci"), "perfexpert_ci");
    files[1] = source_file;
    files[2] = NULL;

    /* Load files and build AST */
    userProject = frontend(2, files);
    ROSE_ASSERT(userProject != NULL);

    /* Create directory for files in this project */
    if (PERFEXPERT_ERROR == perfexpert_util_make_path(globals.outputdir, 0755)) {
        OUTPUT(("%s", _ERROR((char *)"Error: cannot create source directory")));
        return PERFEXPERT_ERROR;
    } else {
        OUTPUT_VERBOSE((4, "source code will be put into (%s)",
                        globals.outputdir));
    }

    /* I believe now it is OK to free 'argv' */
    free(files[0]);
    free(files);

    return PERFEXPERT_SUCCESS;
}

int close_rose(void) {
    // TODO: should find a way to free 'userProject'

    OUTPUT_VERBOSE((7, "=== %s", _BLUE((char *)"Closing Rose")));

    return PERFEXPERT_SUCCESS;
}

int replace_function(function_t *function) {
    ciTraversal functionTraversal;
    FILE *destination_file_FP;
    char *destination_file = NULL;
    SgSourceFile* file = NULL;
    int fileNum;
    int i;

    /* Build the traversal object and call the traversal function starting at
     * the project node of the AST, using a pre-order traversal
     */
    functionTraversal.item = function;
    functionTraversal.function_symbol_new = NULL;

    functionTraversal.traverseInputFiles(userProject, preorder);

    /* Unparse files */
    fileNum = userProject->numberOfFiles();
    for (i = 0; i < fileNum; ++i) {
        file = isSgSourceFile(userProject->get_fileList()[i]);

        /* Open output file */
        destination_file = (char *)malloc(strlen(globals.outputdir) + 3 +
                                          strlen(file->get_sourceFileNameWithoutPath().c_str()));
        if (NULL == destination_file) {
            OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
            return PERFEXPERT_ERROR;
        }
        bzero(destination_file, (strlen(globals.outputdir) + 3 +
                                 strlen(file->get_sourceFileNameWithoutPath().c_str())));
        sprintf(destination_file, "%s/%s", globals.outputdir,
                file->get_sourceFileNameWithoutPath().c_str());

        OUTPUT_VERBOSE((10, "   extracting (%s)", (char *)destination_file));

        destination_file_FP = fopen(destination_file, "w+");
        if (NULL == destination_file_FP) {
            OUTPUT(("%s (%s)", _ERROR((char *)"error opening file"),
                    _ERROR(destination_file)));
            free(destination_file);
            return PERFEXPERT_ERROR;
        }

        /* Output source code */
        fprintf(destination_file_FP, "%s",
                file->unparseToCompleteString().c_str());

        /* Close output file */
        fclose(destination_file_FP);
    }

    free(function->source_file);
    function->source_file = (char *)malloc(strlen(destination_file) + 1);
    bzero(function->source_file, strlen(destination_file) + 1);
    if (NULL == function->source_file) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    strncpy(function->source_file, destination_file, strlen(destination_file));

    /* Append replacement function to source files */
    if (PERFEXPERT_ERROR == insert_function(function)) {
        OUTPUT_VERBOSE((8, "   %s [%s] %s at line %d", _BOLDRED((char *)"FAIL"),
                        (char *)function->source_file,
                        (char *)function->function_name,
                        function->line_number));
    } else {
        OUTPUT_VERBOSE((8, "   %s   [%s] %s at line %d",
                        _BOLDGREEN((char *)"OK"), (char *)function->source_file,
                        (char *)function->function_name,
                        function->line_number));
    }

    /* Clean up */
    free(destination_file);
    // TODO: should find a way to free 'file'

    return PERFEXPERT_SUCCESS;
}

static int insert_function(function_t *function) {
    FILE *replacement_file_FP = NULL;
    FILE *new_source_file_FP = NULL;
    char *new_source_file = NULL;
    FILE *source_file_FP = NULL;
    int line_number = 0;
    char buffer[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];

    /* Open replacement file */
    if (NULL == (replacement_file_FP = fopen(function->replacement_file, "r"))) {
        OUTPUT(("%s (%s)",
                _ERROR((char *)"Error: unable to open function replacement file"),
                function->replacement_file));
        return PERFEXPERT_ERROR;
    }

    /* Open source code file */
    if (NULL == (source_file_FP = fopen(function->source_file, "r"))) {
        OUTPUT(("%s (%s)",
                _ERROR((char *)"Error: unable to open source code file"),
                function->source_file));
        return PERFEXPERT_ERROR;
    }

    /* Open new source code file */
    new_source_file = (char *)malloc(strlen(function->source_file) + 5);
    if (NULL == new_source_file) {
        OUTPUT(("%s", _ERROR((char *)"Error: out of memory")));
        return PERFEXPERT_ERROR;
    }
    bzero(new_source_file, strlen(function->source_file) + 5);
    sprintf(new_source_file, "%s.new", function->source_file);

    if (NULL == (new_source_file_FP = fopen(new_source_file, "w"))) {
        OUTPUT(("%s (%s)",
                _ERROR((char *)"Error: unable to open temporary file"),
                new_source_file));
        return PERFEXPERT_ERROR;
    }

    /* Read replacement file and append it to source file */
    bzero(buffer, BUFFER_SIZE);
    while (NULL != fgets(buffer, BUFFER_SIZE - 1, source_file_FP)) {
        line_number++;

        if (function->line_number == line_number) {
            fprintf(new_source_file_FP,
                    "/* PERFEXPERT: the following function was changed. Don't");
            fprintf(new_source_file_FP,
                    " worry, we created a\n *             copy of your old ");
            fprintf(new_source_file_FP,
                    "function. You will find it just after this one.\n */\n");

            while (NULL != fgets(buffer2, BUFFER_SIZE - 1, replacement_file_FP)) {
                fprintf(new_source_file_FP, "%s", buffer2);
            }

            fprintf(new_source_file_FP,
                    "/* PERFEXPERT: the following function is a copy of your ");
            fprintf(new_source_file_FP,
                    "old '%s' function.\n *             It is safe to remove ",
                    (char *)function->function_name);
            fprintf(new_source_file_FP,
                    "this function and also any other function\n *             ");
            fprintf(new_source_file_FP,
                    "starting with 'PERFEXPERT' unless your original source ");
            fprintf(new_source_file_FP,
                    "code have\n *             functions that starts with ");
            fprintf(new_source_file_FP, "'PERFEXPERT', which is weird.\n */\n");
        }
        fprintf(new_source_file_FP, "%s", buffer);
        bzero(buffer, BUFFER_SIZE);
    }

    /* Close files */
    fclose(source_file_FP);
    fclose(new_source_file_FP);
    fclose(replacement_file_FP);

    return PERFEXPERT_SUCCESS;
}

static SgFunctionSymbol* build_new_function_declaration(SgStatement* statementLocation,
                                                        SgFunctionType* previousFunctionType,
                                                        char *name) {
    // Must mark the newly built node to be a part of a transformation so that it will be unparsed!
    Sg_File_Info * file_info = new Sg_File_Info();
    ROSE_ASSERT(file_info != NULL);
    file_info->set_isPartOfTransformation(true);

    SgName function_name = name;
    ROSE_ASSERT(previousFunctionType != NULL);
    SgFunctionDeclaration* functionDeclaration = new SgFunctionDeclaration(file_info, function_name, previousFunctionType);
    ROSE_ASSERT(functionDeclaration != NULL);
    ROSE_ASSERT(functionDeclaration->get_parameterList() != NULL);

    // Create the InitializedName for a parameter within the parameter list
    SgTypePtrList & argList = previousFunctionType->get_arguments();
    SgTypePtrList::iterator i = argList.begin();
    while ( i != argList.end() )
    {
        SgName var_name = "";
        SgInitializer* var_initializer = NULL;
        SgInitializedName *var_init_name = new SgInitializedName(var_name, *i, var_initializer, NULL);
        functionDeclaration->get_parameterList()->append_arg(var_init_name);
        i++;
    }

    // Get the scope
    SgScopeStatement* scope = statementLocation->get_scope();

    // Set the parent node in the AST (this could be done by the AstPostProcessing
    functionDeclaration->set_parent(scope);

    // Set the scope explicitly (since it could be different from the parent?)
    functionDeclaration->set_scope(scope);

    // If it is not a forward declaration then the unparser will skip the ";" at the end (need to fix this better)
    functionDeclaration->setForward();
    ROSE_ASSERT(functionDeclaration->isForward() == true);

    // Mark function as extern "C"
    functionDeclaration->get_declarationModifier().get_storageModifier().setExtern();
    functionDeclaration->set_linkage("C");  // This mechanism could be improved!

    bool inFront = true;
    SgGlobal* globalScope = TransformationSupport::getGlobalScope(statementLocation);
    SgFunctionDeclaration* functionDeclarationInGlobalScope =
    TransformationSupport::getFunctionDeclaration(statementLocation);
    ROSE_ASSERT(globalScope != NULL);
    ROSE_ASSERT(functionDeclarationInGlobalScope != NULL);
    globalScope->insert_statement(functionDeclarationInGlobalScope,functionDeclaration,inFront);

    SgFunctionSymbol* functionSymbol = new SgFunctionSymbol(functionDeclaration);
    ROSE_ASSERT(functionSymbol != NULL);
    ROSE_ASSERT(functionSymbol->get_type() != NULL);
    
    return functionSymbol;
}

void ciTraversal::visit(SgNode *node) {
    SgFunctionDeclaration *function_declaration = NULL;
    SgFunctionCallExp *function_call = NULL;
    Sg_File_Info *fileInfo = NULL;

    fileInfo = node->get_file_info();

    /* Find the function declaration */
    if ((NULL != (function_declaration = isSgFunctionDeclaration(node))) &&
        (0 == strcmp(function_declaration->get_name().str(),
                     item->function_name))) {
        ROSE_ASSERT(NULL != function_declaration);

        OUTPUT_VERBOSE((10, "   [%s] declaration at line %d",
                        item->function_name, fileInfo->get_line()));

        /* Rename the old function */
        function_name_new = (char *)malloc(strlen(item->function_name) + 10);
        bzero(function_name_new, strlen(item->function_name) + 10);
        sprintf(function_name_new, "PERFEXPERT_%s", item->function_name);
        SgName name = function_name_new;
        function_declaration->set_name(name);

        /* Save the function declaration position */
        item->line_number = fileInfo->get_line();
        item->node = (void *)node;
    }

    /* Find the function call */
    if (NULL != (function_call = isSgFunctionCallExp(node))) {

        SgExpression *function = function_call->get_function();
        ROSE_ASSERT(function);

        if (V_SgFunctionRefExp == function->variantT()) {

            SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(function);
            SgFunctionSymbol *symbol = functionRefExp->get_symbol();
            ROSE_ASSERT(NULL != symbol);

            function_declaration = symbol->get_declaration();
            ROSE_ASSERT(NULL != function_declaration);

            if ((0 == strcmp(function_declaration->get_name().str(),
                             item->function_name)) ||
                (0 == strcmp(function_declaration->get_name().str(),
                             function_name_new))) {

                OUTPUT_VERBOSE((10, "   [%s] call at line %d",
                                item->function_name, fileInfo->get_line()));

                /* This function call should points to a new symbol */

                if (NULL == function_symbol_new) {
                    SgFunctionType* originalFunctionType = isSgFunctionType(symbol->get_type());
                    ROSE_ASSERT(NULL != originalFunctionType);
                    function_symbol_new =
                        build_new_function_declaration(TransformationSupport::getStatement(node),
                                                       originalFunctionType,
                                                       item->function_name);
                }

                ROSE_ASSERT(NULL != function_symbol_new);
                ROSE_ASSERT(NULL != function_symbol_new->get_type());

                functionRefExp->set_symbol(function_symbol_new);
            }
        }
    }
}

// EOF
