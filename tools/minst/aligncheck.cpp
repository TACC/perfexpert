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

#include <rose.h>

#include "rose.h"
#include "aligncheck.h"

using namespace SageBuilder;
using namespace SageInterface;

aligncheck_t::aligncheck_t(VariableRenaming* _var_renaming) {
    var_renaming = _var_renaming;
}

void aligncheck_t::atTraversalStart() {
    inst_info_list.clear();
}

void aligncheck_t::atTraversalEnd() {
}

bool aligncheck_t::vectorizable(SgStatement* stmt) {
    if (!stmt)
        return true;

    if (isSgArithmeticIfStatement(stmt) ||
        isSgAssertStmt(stmt) ||
        isSgAssignedGotoStatement(stmt) ||
        isSgBreakStmt(stmt) ||
        isSgCaseOptionStmt(stmt) ||
        isSgComputedGotoStatement(stmt) ||
        isSgContinueStmt(stmt) ||
        isSgElseWhereStatement(stmt) ||
        isSgForInitStatement(stmt) ||
        isSgGotoStatement(stmt) ||
        isSgReturnStmt(stmt) ||
        isSgClassDefinition(stmt) ||
        isSgDoWhileStmt(stmt) ||
        isSgForAllStatement(stmt) ||
        isSgFortranDo(stmt) ||
        isSgFunctionDefinition(stmt) ||
        isSgIfStmt(stmt) ||
        isSgNamespaceDefinitionStatement(stmt) ||
        isSgSwitchStatement(stmt) ||
        isSgWhileStmt(stmt) ||
        isSgWithStatement(stmt)) {
        return false;
    }

    return true;
}

attrib aligncheck_t::evaluateInheritedAttribute(SgNode* node, attrib attr) {
    SgExpression* index_expr = NULL;
    SgExpression* instrument_test = NULL;
    SgExpression* instrument_incr = NULL;
    SgExpression* increment_var[2] = {0};

    SgForStatement* for_stmt = isSgForStatement(node);
    if (for_stmt) {
        // Sanity check to see if this loop contains statements that prevent
        // vectorization.
        SgStatement* first_stmt = getFirstStatement(for_stmt);
        SgStatement* stmt = first_stmt;
        while (stmt) {
            if (!vectorizable(stmt)) {
                std::cout << "This loop cannot be vectorized because of the "
                    << "following statement: " << stmt->unparseToString() <<
                    "\n";
                return attr;
            }

            stmt = getNextStatement(stmt);
        }

        SgExpression* increment_var[2] = {0};
        SgBinaryOp* incr_binary_op = isSgBinaryOp(for_stmt->get_increment());
        SgUnaryOp* incr_unary_op = isSgUnaryOp(for_stmt->get_increment());
        if (incr_binary_op) {
            // Check if one of these is a constant,
            // if so, the compiler has everything to do it's work.

            increment_var[0] = incr_binary_op->get_lhs_operand();
            increment_var[1] = incr_binary_op->get_rhs_operand();
        } else if (incr_unary_op) {
            increment_var[0] = incr_unary_op->get_operand();
        }

        SgExprStatement* test_statement =
            isSgExprStatement(for_stmt->get_test());
        if (test_statement) {
            SgExpression* test_expression = test_statement->get_expression();
            if (test_expression) {
                SgBinaryOp* binaryOp = isSgBinaryOp(test_expression);
                if (binaryOp) {
                    SgExpression* operand[2];
                    operand[0] = binaryOp->get_lhs_operand();
                    operand[1] = binaryOp->get_rhs_operand();

                    SgExpression* other = NULL;
                    if (isSgValueExp(operand[0]) ||
                        isConstType(operand[0]->get_type())) {
                        other = operand[1];
                    }
                    else if (isSgValueExp(operand[1]) ||
                        isConstType(operand[1]->get_type())) {
                        other = operand[0];
                    }

                    if (other == NULL) {
                        // Is one of these the index variable and other some
                        // other variable? 
                        // XXX: Hack: we should be checking if the AST nodes
                        // are the same but such an API does not seem to exist,
                        // hence checking string representation.
                        std::string operand_str[2];
                        operand_str[0] = operand[0]->unparseToString();
                        operand_str[1] = operand[1]->unparseToString();

                        bool terminate = false;
                        for (int i=0; i<2 && !terminate; i++) {
                            if (increment_var[i]) {
                                std::string increment_str =
                                    increment_var[i]->unparseToString();

                                for (int j=0; j<2 && !terminate; j++) {
                                    if (increment_str == operand_str[j]) {
                                        // The variable that's common between
                                        // test and increment is the index
                                        // variable.
                                        index_expr = increment_var[i];

                                        // While the uncommon variable is the
                                        // test expression to be instrumented.
                                        instrument_test = operand[1-i];

                                        // The other increment expression may
                                        // need to be instrumented
                                        SgExpression* incr = increment_var[1-i];
                                        if (incr && !isSgValueExp(incr) &&
                                            !isConstType(incr->get_type()))
                                            instrument_incr = incr;

                                        // We're done with this loop.
                                        terminate = true;
                                    }
                                }
                            }
                        }
                    } else {
                        // Check if this is the same variable being used in the
                        // increment expression
                        std::string var_string = other->unparseToString();
                        for (int i=0; i<2; i++) {
                            if ((increment_var[i] &&
                                increment_var[i]->unparseToString() ==
                                var_string)) {
                                index_expr = other;

                                // The other increment expression may need to
                                // be instrumented
                                SgExpression* incr = increment_var[1-i];
                                if (incr && !isSgValueExp(incr) &&
                                    !isConstType(incr->get_type()))
                                    instrument_incr = incr;
                            }
                        }
                    }
                }
            }
        }
    }

    locate_and_place_instrumentation(node, instrument_test, instrument_incr);
    return attr;
}

void aligncheck_t::locate_and_place_instrumentation(SgNode* node,
        SgExpression* instrument_test, SgExpression* instrument_incr) {
    // Reaching definitions provided by the VariableRenaming pass
    // don't work for arrays. Hence we check whether we have at least one
    // scalar expression before proceeding to instrument.
    if ((instrument_test && isSgVarRefExp(instrument_test)) ||
            (instrument_incr && isSgVarRefExp(instrument_incr))) {

        // We need to instrument at least one scalar variable. Can this
        // instrumentation be relocated to an earlier point in the program that
        // is outside all loops?
        VariableRenaming::NumNodeRenameTable rename_table =
            var_renaming->getReachingDefsAtNode(node);

        std::string test_string = instrument_test->unparseToString();
        std::string incr_string = instrument_incr->unparseToString();

        typedef VariableRenaming::NumNodeRenameTable::iterator table_iterator;
        typedef VariableRenaming::NumNodeRenameEntry::iterator entry_iterator;

        for (table_iterator table_it = rename_table.begin();
                table_it != rename_table.end(); table_it++) {
            VariableRenaming::VarName name = table_it->first;
            VariableRenaming::NumNodeRenameEntry entry = table_it->second;

            for (std::vector<SgInitializedName*>::iterator name_it =
                    name.begin(); name_it != name.end(); name_it++) {

                std::string var_name = (*name_it)->get_name();

                if (var_name == test_string || var_name == incr_string) {
                    for (entry_iterator entry_it = entry.begin();
                            entry_it != entry.end(); entry_it++) {
                        SgNode* def_node = (*entry_it).second;

                        short itype;
                        SgExpression* iexpr;

                        if (test_string == var_name &&
                                isSgVarRefExp(instrument_test)) {
                            itype = LOOP_TEST;
                            iexpr = instrument_test;
                        } else if (incr_string == var_name &&
                                isSgVarRefExp(instrument_incr)) {
                            itype = LOOP_INCR;
                            iexpr = instrument_incr;
                        } else {
                            continue;
                        }

                        insert_instrumentation(def_node, iexpr, itype);
                    }
                }
            }
        }
    }
}

void aligncheck_t::insert_instrumentation(SgNode* node, SgExpression* expr,
        short type) {
    assert(isSgLocatedNode(node) &&
            "Cannot obtain file information for node to be instrumented.");

    Sg_File_Info *fileInfo = Sg_File_Info::generateFileInfoForTransformationNode(
            ((SgLocatedNode*) node)->get_file_info()->get_filenameString());

    SgIntVal* param_type = new SgIntVal(fileInfo, type);

    SgBasicBlock* outer_basic_block = getEnclosingNode<SgBasicBlock>(node);
    SgStatement* outer_statement = isSgStatement(node) ? :
            getEnclosingNode<SgStatement>(node);

    assert(outer_basic_block);
    assert(outer_statement);

    inst_info_t inst_info;
    inst_info.bb = outer_basic_block;
    inst_info.stmt = outer_statement;
    inst_info.params.push_back(expr);
    inst_info.params.push_back(param_type);

    inst_info.function_name = SageInterface::is_Fortran_language() ? "indigo__aligncheck_f" : "indigo__aligncheck_c";
    inst_info.before = false;

    inst_info_list.push_back(inst_info);
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_begin() {
    return inst_info_list.begin();
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_end() {
    return inst_info_list.end();
}
