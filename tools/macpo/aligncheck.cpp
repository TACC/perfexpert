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

aligncheck_t::aligncheck_t(VariableRenaming*& _var_renaming) {
    var_renaming = _var_renaming;
}

void aligncheck_t::atTraversalStart() {
    inst_info_list.clear();
}

void aligncheck_t::atTraversalEnd() {
}

bool aligncheck_t::vectorizable(SgStatement*& stmt) {
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
    SgExpression *idxv_expr, *init_expr, *test_expr, *incr_expr;

    SgForStatement* for_stmt = isSgForStatement(node);
    if (for_stmt) {
        def_map_t def_map;

        // If we need to instrument at least one scalar variable,
        // can this instrumentation be relocated to an earlier point
        // in the program that is outside all loops?
        VariableRenaming::NumNodeRenameTable rename_table =
            var_renaming->getReachingDefsAtNode(node);

        // Expand the iterator list into a map for easier lookup.
        construct_def_map(rename_table, def_map);

        // Extract components from the for-loop header.
        get_loop_header_components(for_stmt, def_map, idxv_expr, init_expr,
                test_expr, incr_expr);

        if (init_expr)
            instrument_loop_header_components(node, def_map, init_expr,
                    LOOP_INIT);

        if (test_expr)
            instrument_loop_header_components(node, def_map, test_expr,
                    LOOP_TEST);

        if (incr_expr)
            instrument_loop_header_components(node, def_map, incr_expr,
                    LOOP_INCR);
    }

    return attr;
}

bool aligncheck_t::get_loop_header_components(SgForStatement*& for_stmt,
        def_map_t& def_map, SgExpression*& idxv_expr, SgExpression*& init_expr,
        SgExpression*& test_expr, SgExpression*& incr_expr) {
    bool ret = true;

    // Initialization
    idxv_expr = NULL;
    init_expr = NULL;
    test_expr = NULL;
    incr_expr = NULL;

    // Sanity check to see if this loop contains statements that prevent
    // vectorization.
    SgStatement* first_stmt = getFirstStatement(for_stmt);
    SgStatement* stmt = first_stmt;
    while (stmt) {
        if (!vectorizable(stmt)) {
            std::cout << "This loop cannot be vectorized because of the "
                << "following statement: " << stmt->unparseToString() <<
                "\n";
            return false;
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
                                    idxv_expr = increment_var[i];

                                    // While the uncommon variable is the
                                    // test expression to be instrumented.
                                    test_expr = operand[1-i];

                                    // The other increment expression may
                                    // need to be instrumented
                                    incr_expr = increment_var[1-i];

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
                            idxv_expr = other;

                            // The other increment expression may need to
                            // be instrumented
                            incr_expr = increment_var[1-i];
                        }
                    }
                }
            }
        }
    }

    if (idxv_expr) {
        // XXX: String comparison again instead of tree comparison.
        std::string idxv_string = idxv_expr->unparseToString();

        SgStatementPtrList init_list = for_stmt->get_init_stmt();
        if (init_list.size()) {
            for (SgStatementPtrList::iterator it = init_list.begin();
                    it != init_list.end(); it++) {
                SgStatement* stmt = *it;
                if (SgExprStatement* expr_stmt = isSgExprStatement(stmt)) {
                    if (SgAssignOp* assign_op =
                            isSgAssignOp(expr_stmt->get_expression())) {
                        SgExpression* operand[2];
                        operand[0] = assign_op->get_lhs_operand();
                        operand[1] = assign_op->get_rhs_operand();

                        if (operand[0]->unparseToString() == idxv_string) {
                            init_expr = operand[1];
                            break;
                        } else if (operand[1]->unparseToString() ==
                                idxv_string) {
                            init_expr = operand[0];
                            break;
                        }
                    }
                }
            }
        } else {
            // Find the reaching definition of the index variable.
            std::string istr = idxv_expr->unparseToString();
            if (def_map.find(istr) != def_map.end()) {
                VariableRenaming::NumNodeRenameEntry entry_list = def_map[istr];

                if (entry_list.size() == 1) {
                    SgNode* def_node = entry_list.begin()->second;
                    init_expr = get_expr_value(def_node, istr);
                } else {
                    int grandchild_count = 0;
                    SgExpression* expr_value = NULL;
                    for (entry_iterator entry_it = entry_list.begin();
                            entry_it != entry_list.end() &&
                            grandchild_count < 2; entry_it++) {
                        SgNode* def_node = (*entry_it).second;
                        if (isAncestor(for_stmt, def_node))
                            grandchild_count += 1;
                        else
                            expr_value = get_expr_value(def_node, istr);
                    }

                    if (grandchild_count == 1)
                        init_expr = expr_value;
                }
            }
        }
    }

    // Return true if we were able to discover all three expressions.
    return init_expr && test_expr && incr_expr;
}

SgExpression* aligncheck_t::get_expr_value(SgNode*& node,
        std::string var_name) {
    SgInitializedName* init_name = isSgInitializedName(node);

    if (init_name && init_name->get_initializer()) {
        // We don't need to instrument this access.
        return init_name->get_initializer();
    } else if (SgAssignOp* asgn_op = isSgAssignOp(node)) {
        SgExpression* lhs = asgn_op->get_lhs_operand();
        SgExpression* rhs = asgn_op->get_rhs_operand();

        if (lhs->unparseToString() == var_name)
            return rhs;
        else if (rhs->unparseToString() == var_name)
            return lhs;
    }

    return NULL;
}

void aligncheck_t::construct_def_map(
        VariableRenaming::NumNodeRenameTable& rename_table,
        def_map_t& def_map) {

    typedef VariableRenaming::NumNodeRenameTable::iterator table_iterator;

    def_map.clear();

    for (table_iterator table_it = rename_table.begin();
            table_it != rename_table.end(); table_it++) {

        VariableRenaming::VarName name_list = table_it->first;
        VariableRenaming::NumNodeRenameEntry entry_list = table_it->second;

        for (std::vector<SgInitializedName*>::iterator name_it =
                name_list.begin(); name_it != name_list.end(); name_it++) {
            std::string var_name = (*name_it)->get_name();

            def_map[var_name] = entry_list;
        }
    }
}

void aligncheck_t::instrument_loop_header_components(SgNode*& loop_node,
        def_map_t& def_map, SgExpression*& instrument_expr,
        short loop_inst_type) {

    std::string expr_str = instrument_expr->unparseToString();
    if (isSgVarRefExp(instrument_expr) && def_map.find(expr_str) !=
            def_map.end()) {
        VariableRenaming::NumNodeRenameEntry entry_list = def_map[expr_str];

        if (entry_list.size() == 1) {
            // Only one definition of this variable reaches here.
            // If it's value is a constant, we can directly use
            // the constant in place of the variable!
            SgNode* def_node = entry_list.begin()->second;
            if (get_expr_value(def_node, expr_str) == NULL)
                 insert_instrumentation(def_node, instrument_expr,
                        loop_inst_type, false);
        } else {
            // Instrument all definitions of this variable.
            for (entry_iterator entry_it = entry_list.begin();
                    entry_it != entry_list.end(); entry_it++) {
                SgNode* def_node = (*entry_it).second;
                insert_instrumentation(def_node, instrument_expr,
                        loop_inst_type, false);
            }
        }
    } else if (!isSgValueExp(instrument_expr) &&
            !isConstType(instrument_expr->get_type())) {
        // Instrument without using the reaching definition.
        insert_instrumentation(loop_node, instrument_expr, loop_inst_type,
                true);
    }
}

void aligncheck_t::insert_instrumentation(SgNode*& node, SgExpression*& expr,
        short type, bool before) {
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
    inst_info.before = before;

    inst_info_list.push_back(inst_info);
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_begin() {
    return inst_info_list.begin();
}

const std::vector<inst_info_t>::iterator aligncheck_t::inst_end() {
    return inst_info_list.end();
}
