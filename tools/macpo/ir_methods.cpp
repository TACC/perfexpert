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

#include "inst_defs.h"
#include "ir_methods.h"

using namespace SageBuilder;
using namespace SageInterface;

bool ir_methods::vectorizable(SgStatement*& stmt) {
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

int ir_methods::get_loop_header_components(VariableRenaming*& var_renaming,
        SgForStatement*& for_stmt, def_map_t& def_map, SgExpression*&
        idxv_expr, SgExpression*& init_expr, SgExpression*& test_expr,
        SgExpression*& incr_expr, int& incr_op) {

    int return_value = 0;

    // Initialization
    idxv_expr = NULL;
    init_expr = NULL;
    test_expr = NULL;
    incr_expr = NULL;
    incr_op = INVALID_OP;

    if (!for_stmt)
        return INVALID_LOOP;

    // If we need to instrument at least one scalar variable,
    // can this instrumentation be relocated to an earlier point
    // in the program that is outside all loops?
    VariableRenaming::NumNodeRenameTable rename_table =
        var_renaming->getReachingDefsAtNode(for_stmt);

    // Expand the iterator list into a map for easier lookup.
    ir_methods::construct_def_map(rename_table, def_map);

    // Sanity check to see if this loop contains statements that prevent
    // vectorization.
    SgStatement* first_stmt = getFirstStatement(for_stmt);
    SgStatement* stmt = first_stmt;
    while (stmt) {
        if (!vectorizable(stmt)) {
            std::cout << "This loop cannot be vectorized because of the "
                << "following statement: " << stmt->unparseToString() <<
                "\n";
            return INVALID_FLOW;
        }

        stmt = getNextStatement(stmt);
    }

    SgExpression* increment_var[2] = {0};
    SgBinaryOp* incr_binary_op = isSgBinaryOp(for_stmt->get_increment());
    SgUnaryOp* incr_unary_op = isSgUnaryOp(for_stmt->get_increment());
    if (incr_binary_op) {
        if (SgPlusAssignOp* minus_op = isSgPlusAssignOp(incr_binary_op)) {
            incr_op = OP_SUB;
            incr_expr = minus_op->get_rhs_operand();
        } else if (SgPlusAssignOp* plus_op = isSgPlusAssignOp(incr_binary_op)) {
            incr_op = OP_ADD;
            incr_expr = plus_op->get_rhs_operand();
        } else if (SgAssignOp* assign_op = isSgAssignOp(incr_binary_op)) {
            SgExpression* rhs_operand = assign_op->get_rhs_operand();
            if (SgAddOp* add_op = isSgAddOp(rhs_operand)) {
                incr_op = OP_ADD;
                // Can't set incr_expr as we don't know
                // whether lhs or rhs is incr value.
            } else if (SgSubtractOp* sub_op = isSgSubtractOp(rhs_operand)) {
                incr_op = OP_SUB;
                // Can't set incr_expr as we don't know
                // whether lhs or rhs is incr value.
            }
        }

        increment_var[0] = incr_binary_op->get_lhs_operand();
        increment_var[1] = incr_binary_op->get_rhs_operand();
    } else if (incr_unary_op) {
        Sg_File_Info *fileInfo =
            Sg_File_Info::generateFileInfoForTransformationNode(
                    ((SgLocatedNode*)
                    for_stmt)->get_file_info()->get_filenameString());

        if (SgPlusAssignOp* minus_op = isSgPlusAssignOp(incr_unary_op)) {
            incr_op = OP_SUB;
            incr_expr = minus_op->get_rhs_operand();
        } else if (SgPlusAssignOp* plus_op = isSgPlusAssignOp(incr_unary_op)) {
            incr_op = OP_ADD;
            incr_expr = plus_op->get_rhs_operand();
        } else if (SgMinusMinusOp* minus_op = isSgMinusMinusOp(incr_unary_op)) {
            incr_op = OP_SUB;
            incr_expr = new SgIntVal(fileInfo, 1);
        } else if (SgPlusPlusOp* plus_op = isSgPlusPlusOp(incr_unary_op)) {
            incr_op = OP_ADD;
            incr_expr = new SgIntVal(fileInfo, 1);
        }

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
                    test_expr = operand[0];
                } else if (isSgValueExp(operand[1]) ||
                        isConstType(operand[1]->get_type())) {
                    other = operand[0];
                    test_expr = operand[1];
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

                                    // incr_expr may have already been set
                                    // if the increment was a unary operation.
                                    if (incr_expr == NULL) {
                                        // The other increment expression may
                                        // need to be instrumented, unless
                                        // this is a unary increment operation.
                                        if (increment_var[1-i]) {
                                            incr_expr = increment_var[1-i];
                                        } else {
                                            incr_expr = increment_var[i];
                                        }
                                    }

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
                            // be instrumented, unless this is a unary op.
                            if (increment_var[1-i]) {
                                incr_expr = increment_var[1-i];
                            } else {
                                incr_expr = increment_var[i];
                            }
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

    if (!init_expr) return_value |= INVALID_INIT;
    if (!test_expr) return_value |= INVALID_TEST;
    if (!incr_expr) return_value |= INVALID_INCR;

    // Return true if we were able to discover all three expressions.
    return return_value;
}

void ir_methods::construct_def_map(VariableRenaming::NumNodeRenameTable& rename_table,
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

SgExpression* ir_methods::get_expr_value(SgNode*& node, std::string var_name) {
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

SgExprStatement* ir_methods::prepare_call_statement(SgBasicBlock* bb,
        const std::string& function_name, const std::vector<SgExpression*>&
        params) {
    SgNode* parent = bb->get_parent();

    if (isSgIfStmt(parent)) {
        SgIfStmt* if_node = static_cast<SgIfStmt*>(parent);
        if_node->set_use_then_keyword(true);
        if_node->set_has_end_statement(true);
    }

    SgExprStatement* fCall = buildFunctionCallStmt(SgName(function_name),
            buildVoidType(), buildExprListExp(params), bb);

    return fCall;
}

long ir_methods::get_reference_index(reference_list_t& reference_list,
        std::string& stream_name) {
    if (stream_name.size() == 0)
        return -1;

    size_t idx = 0;
    for (reference_list_t::iterator it = reference_list.begin();
            it != reference_list.end(); it++) {
        if (it->name == stream_name) {
            break;
        }

        idx += 1;
    }

    if (idx == reference_list.size()) {
        reference_info_t stream_info;
        stream_info.name = stream_name;

        reference_list.push_back(stream_info);
        idx = reference_list.size() - 1;
    }

    return idx;
}

std::string ir_methods::strip_index_expr(const std::string& stream_name) {
    std::string return_string = stream_name;
    while (return_string.at(return_string.length()-1) == ']') {
        // Strip off the last [.*]
        unsigned index = return_string.find_last_of('[');
        return_string = return_string.substr(0, index);
    }

    // For fortran array notation
    while (return_string.at(return_string.length()-1) == ')') {
        // Strip off the last [.*]
        unsigned index = return_string.find_last_of('(');
        return_string = return_string.substr(0, index);
    }

    return return_string;
}

bool ir_methods::is_known(const SgExpression* expr) {
    return isSgValueExp(expr) || isConstType(expr->get_type());
}

bool ir_methods::is_linear_reference(const SgBinaryOp* reference,
        bool check_lhs_operand) {
    if (reference) {
        SgExpression* lhs = reference->get_lhs_operand();
        SgExpression* rhs = reference->get_rhs_operand();

        if (check_lhs_operand && lhs && isSgPntrArrRefExp(lhs))
            return false;

        if (rhs && isSgPntrArrRefExp(rhs))
            return false;

        SgBinaryOp* bin_op = isSgBinaryOp(lhs);
        if (lhs && bin_op && !ir_methods::is_linear_reference(bin_op, true))
            return false;

        bin_op = isSgBinaryOp(rhs);
        if (rhs && bin_op && !ir_methods::is_linear_reference(bin_op, true))
            return false;
    }

    return true;
}

bool ir_methods::contains_expr(SgBinaryOp*& bin_op,
        SgExpression*& search_expr) {
    if (bin_op && search_expr) {
        SgExpression* lhs = bin_op->get_lhs_operand();
        SgExpression* rhs = bin_op->get_rhs_operand();

        // If either operand matches, replace it!
        if (lhs && lhs->unparseToString() == search_expr->unparseToString())
            return true;

        if (rhs && rhs->unparseToString() == search_expr->unparseToString())
            return true;

        // Check if we need to call recursively.
        SgBinaryOp* pntr = NULL;
        if (lhs && (pntr = isSgBinaryOp(lhs)))
            if (ir_methods::contains_expr(pntr, search_expr))
                return true;

        if (rhs && (pntr = isSgBinaryOp(rhs)))
            if (ir_methods::contains_expr(pntr, search_expr))
                return true;
    }

    return false;
}

void ir_methods::replace_expr(SgBinaryOp*& bin_op, SgExpression*& search_expr,
        SgExpression*& replace_expr) {
    if (bin_op && search_expr && replace_expr) {
        SgExpression* lhs = bin_op->get_lhs_operand();
        SgExpression* rhs = bin_op->get_rhs_operand();

        // If either operand matches, replace it!
        if (lhs && lhs->unparseToString() == search_expr->unparseToString())
            bin_op->set_lhs_operand(replace_expr);

        if (rhs && rhs->unparseToString() == search_expr->unparseToString())
            bin_op->set_rhs_operand(replace_expr);

        // Check if we need to call recursively.
        SgBinaryOp* pntr = NULL;
        if (lhs && (pntr = isSgBinaryOp(lhs)))
            ir_methods::replace_expr(pntr, search_expr, replace_expr);

        if (rhs && (pntr = isSgBinaryOp(rhs)))
            ir_methods::replace_expr(pntr, search_expr, replace_expr);
    }
}

SgExpression* ir_methods::get_final_value(Sg_File_Info* file_info,
        SgExpression* test_expr, SgExpression* incr_expr, int incr_op) {
    if (test_expr && incr_expr) {
        SgType* type = test_expr->get_type();

        switch(incr_op) {
            case OP_ADD:
                return new SgSubtractOp(file_info, test_expr, incr_expr, type);

            case OP_SUB:
                return new SgAddOp(file_info, test_expr, incr_expr, type);

            case OP_MUL:
                return new SgDivideOp(file_info, test_expr, incr_expr, type);

            case OP_DIV:
                return new SgMultiplyOp(file_info, test_expr, incr_expr, type);
        }
    }

    return NULL;
}
