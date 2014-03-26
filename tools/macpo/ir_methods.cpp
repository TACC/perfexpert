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

SgExpression* ir_methods::_strip_unary_operators(SgExpression* expr) {
    // If we found a unary operation, return it's pointer.
    if (SgUnaryOp* unary_op = isSgUnaryOp(expr)) {
        return unary_op->get_operand_i();
    }

    // If it's a pointer, drill deeper.
    if (SgPntrArrRefExp* pntr = isSgPntrArrRefExp(expr)) {
        SgExpression* rhs_expr = pntr->get_rhs_operand_i();
        SgExpression* revised_rhs = _strip_unary_operators(rhs_expr);

        pntr->set_rhs_operand_i(revised_rhs);
        return pntr;
    }

    // Otherwise, return just the expression as it is.
    return expr;
}

SgExpression* ir_methods::strip_unary_operators(SgExpression* expr) {
    ROSE_ASSERT(expr && "Empty expression as input to "
            "ir_methods::strip_unary_operators(SgExpression* expr)!");

    SgExpression* copy = copyExpression(expr);
    return _strip_unary_operators(copy);
}

void ir_methods::place_alignment_checks(expr_list_t& expr_list,
        Sg_File_Info* fileInfo, SgScopeStatement* loop_stmt,
        statement_list_t& statement_list, const std::string& prefix) {
    std::vector<SgExpression*> addresses;
    for (expr_list_t::iterator it = expr_list.begin();
            it != expr_list.end(); it++) {
        SgExpression* expr = *it;
        if (SgPntrArrRefExp* pntr = isSgPntrArrRefExp(expr)) {
            SgExpression *param_addr = (SgExpression*) pntr;

            // Strip unary operators like ++ or -- from the expression.
            param_addr = ir_methods::strip_unary_operators(expr);
            ROSE_ASSERT(param_addr && "Bug in stripping unary operators from "
                    "given expression!");

            if (is_Fortran_language() == false) {
                SgAddressOfOp* address_op = NULL;
                SgType* void_pointer_type = NULL;

                address_op = buildAddressOfOp(param_addr);
                void_pointer_type = buildPointerType(buildVoidType());
                param_addr = buildCastExp (address_op, void_pointer_type);
            }

            addresses.push_back(param_addr);
        }
    }

    if (addresses.size() > 0) {
        std::vector<SgExpression*> params;

        int line_number = loop_stmt->get_file_info()->get_raw_line();
        params.push_back(new SgIntVal(fileInfo, line_number));

        int address_count = addresses.size();
        params.push_back(new SgIntVal(fileInfo, address_count));

        // Now push all of the addresses.
        params.insert(params.end(), addresses.begin(), addresses.end());

        SgStatement* first_statement = loop_stmt->firstStatement();
        SgBasicBlock* first_bb = getEnclosingNode<SgBasicBlock>(first_statement);

        std::string function_name = prefix + (is_Fortran_language() ? "_f" :
            "_c");
        SgStatement* call_stmt = ir_methods::prepare_call_statement(first_bb,
                function_name, params, first_statement);

        SgBasicBlock* aligncheck_list = new SgBasicBlock(fileInfo);
        aligncheck_list->append_statement(call_stmt);

        // Create new integer variable called
        // "indigo__aligncheck_init_<line_number>". Funky, eh?
        char var_name[64];
        snprintf (var_name, 64, "%s_%d", prefix.c_str(), line_number);
        SgType* long_type = buildLongType();

        SgVariableDeclaration* aligncheck_init = NULL;
        aligncheck_init = ir_methods::create_long_variable(fileInfo, var_name,
            0);
        SgBasicBlock* parent_bb = getEnclosingNode<SgBasicBlock>(loop_stmt);
        aligncheck_init->set_parent(parent_bb);

        SgStatement* reference_stmt = NULL;
        reference_stmt = getEnclosingNode<SgScopeStatement>(loop_stmt);

        SgOmpBodyStatement* omp_body_stmt = NULL;
        omp_body_stmt = getEnclosingNode<SgOmpBodyStatement>(loop_stmt);
        if (omp_body_stmt && ir_methods::is_ancestor((SgNode*) reference_stmt,
                    (SgNode*) omp_body_stmt) == false) {
            reference_stmt = omp_body_stmt;
        }

        if (reference_stmt == NULL ||
                (ir_methods::is_loop(reference_stmt) == false &&
                isSgOmpBodyStatement(reference_stmt) == false)) {
            reference_stmt = loop_stmt;
        }

        statement_info_t aligncheck_init_decl;
        aligncheck_init_decl.reference_statement = reference_stmt;
        aligncheck_init_decl.statement = aligncheck_init;
        aligncheck_init_decl.before = true;
        statement_list.push_back(aligncheck_init_decl);

        // Create the expression statement.
        SgExpression* guard_condition = NULL;
        guard_condition = new SgEqualityOp(fileInfo, buildVarRefExp(var_name),
            new SgLongIntVal(fileInfo, 0), long_type);

        SgExprStatement* guard_condition_stmt = NULL;
        guard_condition_stmt = new SgExprStatement(fileInfo, guard_condition);
        guard_condition->set_parent(guard_condition_stmt);

        // Create statement to reset guard value.
        SgExprStatement* reset_guard_stmt = NULL;
        reset_guard_stmt = ir_methods::create_long_assign_statement(fileInfo,
             var_name, new SgIntVal(fileInfo, 1));
        aligncheck_list->append_statement(reset_guard_stmt);
        reset_guard_stmt->set_parent(aligncheck_list);

        // Insert the guard condition.
        if (SgIfStmt* init_guard = new SgIfStmt(fileInfo)) {
            init_guard->set_conditional(guard_condition_stmt);
            init_guard->set_true_body(aligncheck_list);

            aligncheck_list->set_parent(init_guard);
            guard_condition_stmt->set_parent(init_guard);

            statement_info_t guard_info;
            guard_info.statement = init_guard;
            guard_info.reference_statement = first_statement;
            guard_info.before = true;
            statement_list.push_back(guard_info);
        }
    }
}

void ir_methods::remove_duplicate_expressions(expr_list_t& expr_list) {
    std::map<std::string, SgExpression*> string_expr_map;
    for (expr_list_t::iterator it2 = expr_list.begin();
            it2 != expr_list.end(); it2++) {
        SgExpression* expr = *it2;
        std::string expr_string = expr->unparseToString();
        if (string_expr_map.find(expr_string) == string_expr_map.end()) {
            string_expr_map[expr_string] = expr;
        } else {
            // An expression with the same string representation has
            // already been seen, so we skip this expression.
        }
    }

    // Reuse the same expression list from earlier.
    expr_list.clear();

    // Finally, loop over the map to get the unique expressions.
    for (std::map<std::string, SgExpression*>::iterator it2 =
            string_expr_map.begin(); it2 != string_expr_map.end(); it2++) {
        SgExpression* expr = it2->second;
        expr_list.push_back(expr);
    }
}

bool ir_methods::is_ancestor(SgNode* lower_node, SgNode* upper_node) {
    SgNode* traversal_node = lower_node;
    while (traversal_node) {
        if (traversal_node == upper_node)
            return true;

        traversal_node = traversal_node->get_parent();
    }

    return false;
}

SgExprStatement* ir_methods::create_long_assign_statement(Sg_File_Info* fileInfo,
        const std::string& name, SgIntVal* value) {
    SgType* long_type = buildLongType();
    SgVarRefExp* expr = buildVarRefExp(name);
    SgAssignOp* assign_op = new SgAssignOp(fileInfo, expr, value, long_type);
    SgExprStatement* assign_stmt = new SgExprStatement(fileInfo, assign_op);

    return assign_stmt;
}

SgExprStatement* ir_methods::create_long_incr_statement(Sg_File_Info* fileInfo,
        const std::string& name) {
    SgType* long_type = buildLongType();
    SgVarRefExp* expr = buildVarRefExp(name);
    SgPlusPlusOp* incr_op = new SgPlusPlusOp(fileInfo, expr,long_type);
    SgExprStatement* incr_statement = new SgExprStatement(fileInfo, incr_op);

    return incr_statement;
}

SgVariableDeclaration* ir_methods::create_long_variable(Sg_File_Info* fileInfo,
        const std::string& name, long init_value) {
    SgType* long_type = buildLongType();
    SgVariableDeclaration* var_decl = new SgVariableDeclaration(fileInfo,
            name, long_type, buildAssignInitializer(buildIntVal(init_value)));

    return var_decl;
}

SgVariableDeclaration* ir_methods::create_long_variable_with_init(
        Sg_File_Info* fileInfo, const std::string& name,
        SgExpression* initializer) {
    SgType* long_type = buildLongType();
    SgVariableDeclaration* var_decl = new SgVariableDeclaration(fileInfo,
            name, long_type, buildAssignInitializer(initializer));

    return var_decl;
}

bool ir_methods::vectorizable(SgStatement*& stmt) {
    if (!stmt)
        return true;

    SgScopeStatement* scope_stmt = isSgScopeStatement(stmt);
    if (SgIfStmt* if_statement = isSgIfStmt(stmt)) {
        SgStatement* true_body = if_statement->get_true_body();
        SgStatement* false_body = if_statement->get_false_body();

        if (!vectorizable(true_body))
            return false;

        if (!vectorizable(false_body))
            return false;
    } else if (scope_stmt && !ir_methods::is_loop(stmt)) {
        const SgStatementPtrList stmt_list = scope_stmt->generateStatementList();
        for(int i=0; i<stmt_list.size(); i++) {
            SgStatement* inner_stmt = stmt_list[i];
            if (!vectorizable(inner_stmt)) {
                return false;
            }
        }
    } else if (SgExprStatement* expr_stmt = isSgExprStatement(stmt)) {
        SgExpression* expr = expr_stmt->get_expression();
        if (isSgFunctionCallExp(expr))
            return false;
    } else if (isSgArithmeticIfStatement(stmt) ||
            isSgAssertStmt(stmt) ||
            isSgAssignedGotoStatement(stmt) ||
            isSgBreakStmt(stmt) ||
            isSgCaseOptionStmt(stmt) ||
            isSgComputedGotoStatement(stmt) ||
            isSgContinueStmt(stmt) ||
            isSgElseWhereStatement(stmt) ||
            isSgGotoStatement(stmt) ||
            isSgReturnStmt(stmt) ||
            isSgClassDefinition(stmt) ||
            isSgFunctionDefinition(stmt) ||
            /* isSgIfStmt(stmt) || */   // Don't overlook branches completely.
            isSgNamespaceDefinitionStatement(stmt) ||
            isSgSwitchStatement(stmt) ||
            isSgWithStatement(stmt)) {
        return false;
    }

    return true;
}

int ir_methods::get_loop_header_components(VariableRenaming*& var_renaming,
        SgScopeStatement*& scope_stmt, def_map_t& def_map, SgExpression*&
        idxv_expr, SgExpression*& init_expr, SgExpression*& test_expr,
        SgExpression*& incr_expr, int& incr_op) {

    int return_value = 0;

    // Initialization
    idxv_expr = NULL;
    init_expr = NULL;
    test_expr = NULL;
    incr_expr = NULL;
    incr_op = INVALID_OP;

    if (!scope_stmt)
        return INVALID_LOOP;

    // Sanity check to see if this loop contains statements that prevent
    // vectorization.
    SgStatement* first_stmt = getFirstStatement(scope_stmt);
    SgStatement* stmt = first_stmt;
    while (stmt) {
        if (!vectorizable(stmt)) {
            std::cerr << mprefix << "This loop cannot be vectorized because of "
                << "the following statement: " << stmt->unparseToString() <<
                "\n";
            return INVALID_FLOW;
        }

        stmt = getNextStatement(stmt);
    }

    // If we need to instrument at least one scalar variable,
    // can this instrumentation be relocated to an earlier point
    // in the program that is outside all loops?
    VariableRenaming::NumNodeRenameTable rename_table =
        var_renaming->getReachingDefsAtNode(scope_stmt);

    // Expand the iterator list into a map for easier lookup.
    ir_methods::construct_def_map(rename_table, def_map);

    if (SgForStatement* for_stmt = isSgForStatement(scope_stmt)) {
        return get_for_loop_header_components(var_renaming, for_stmt, def_map,
            idxv_expr, init_expr, test_expr, incr_expr, incr_op);
    } else {
        return get_while_loop_header_components(scope_stmt, idxv_expr,
                test_expr, incr_expr, incr_op);
    }
}

void ir_methods::incr_components(Sg_File_Info*& fileInfo, SgExpression*& expr,
        SgExpression*& incr_expr, int& incr_op) {
    incr_expr = NULL;
    incr_op = INVALID_OP;

    SgBinaryOp* incr_binary_op = isSgBinaryOp(expr);
    SgUnaryOp* incr_unary_op = isSgUnaryOp(expr);

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
    } else if (incr_unary_op) {
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
    }
}

int ir_methods::get_for_loop_header_components(VariableRenaming*& var_renaming,
        SgForStatement*& for_stmt, def_map_t& def_map, SgExpression*&
        idxv_expr, SgExpression*& init_expr, SgExpression*& test_expr,
        SgExpression*& incr_expr, int& incr_op) {

    int return_value = 0;
    SgExpression* increment_var[2] = {0};

    Sg_File_Info *fileInfo =
        Sg_File_Info::generateFileInfoForTransformationNode(
                ((SgLocatedNode*)
                for_stmt)->get_file_info()->get_filenameString());
    SgExpression* incr = for_stmt->get_increment();
    incr_components(fileInfo, incr, incr_expr, incr_op);

    SgBinaryOp* incr_binary_op = isSgBinaryOp(for_stmt->get_increment());
    SgUnaryOp* incr_unary_op = isSgUnaryOp(for_stmt->get_increment());
    if (incr_binary_op) {
        increment_var[0] = incr_binary_op->get_lhs_operand();
        increment_var[1] = incr_binary_op->get_rhs_operand();
    } else if (incr_unary_op) {
        increment_var[0] = incr_unary_op->get_operand();
    }

    SgExprStatement* test_statement = NULL;
    test_statement = isSgExprStatement(for_stmt->get_test());

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

                            // incr_expr may have already been set
                            // if the increment was a unary operation.
                            if (incr_expr == NULL) {
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
                } else if (SgVariableDeclaration* var_decl = isSgVariableDeclaration(stmt)) {
                    std::vector<SgInitializedName*> var_list = var_decl->get_variables();
                    for (int i=0; i<var_list.size(); i++) {
                        SgInitializedName* init_name = var_list.at(i);
                        SgName name = init_name->get_qualified_name();
                        if (name.getString() == idxv_string) {
                            SgAssignInitializer* init_ptr = isSgAssignInitializer(init_name->get_initptr());
                            init_expr = init_ptr;
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

int ir_methods::get_while_loop_header_components(SgScopeStatement*& scope_stmt,
        SgExpression*& idxv_expr, SgExpression*& test_expr,
        SgExpression*& incr_expr, int& incr_op) {
    // Initialize
    idxv_expr = NULL;
    test_expr = NULL;
    incr_expr = NULL;

    bool vec_checked = false;
    int return_value = 0;

    SgStatement* loop_body = NULL;
    SgExprStatement* test_statement = NULL;
    if (SgWhileStmt* while_stmt = isSgWhileStmt(scope_stmt)) {
        test_statement = isSgExprStatement(while_stmt->get_condition());
        loop_body = while_stmt->get_body();
    } else if (SgDoWhileStmt* do_while_stmt = isSgDoWhileStmt(scope_stmt)) {
        test_statement = isSgExprStatement(do_while_stmt->get_condition());
        loop_body = do_while_stmt->get_body();
    } else {
        ROSE_ASSERT(false && "Invalid loop type!");
    }

    Sg_File_Info *fileInfo =
        Sg_File_Info::generateFileInfoForTransformationNode(
                ((SgLocatedNode*)
                scope_stmt)->get_file_info()->get_filenameString());

    if (test_statement) {
        SgExpression* test_expression = test_statement->get_expression();
        if (test_expression) {
            SgBinaryOp* binaryOp = isSgBinaryOp(test_expression);
            if (binaryOp) {
                SgExpression* operand[2];
                operand[0] = binaryOp->get_lhs_operand();
                operand[1] = binaryOp->get_rhs_operand();

                if (isSgUnaryOp(operand[0]) || isSgUnaryOp(operand[1])) {
                    if (isSgUnaryOp(operand[0]) && isSgUnaryOp(operand[1])) {
                        idxv_expr = NULL;
                        test_expr = NULL;

                        return_value |= INVALID_IDXV;
                        return_value |= INVALID_TEST;
                    } else {
                        if (isSgUnaryOp(operand[0])) {
                            idxv_expr = isSgUnaryOp(operand[0])->get_operand();
                            test_expr = operand[1];
                        } else {
                            idxv_expr = isSgUnaryOp(operand[1])->get_operand();
                            test_expr = operand[0];
                        }

                        // Check if the loop body references the index var again.
                        SgBasicBlock* bb = dynamic_cast<SgBasicBlock*>(loop_body);
                        SgStatement* stmt = dynamic_cast<SgStatement*>(loop_body);
                        if (bb) {
                            SgStatementPtrList& stmts = bb->get_statements();
                            for (SgStatementPtrList::iterator it=stmts.begin(); it!=stmts.end(); it++) {
                                if (in_write_set(*it, idxv_expr)) {
                                    idxv_expr = NULL;
                                    return_value |= INVALID_IDXV;
                                }
                            }
                        } else if (stmt) {
                            if (in_write_set(stmt, idxv_expr)) {
                                idxv_expr = NULL;
                                return_value |= INVALID_IDXV;
                            }
                        }

                        return return_value;
                    }
                } else {
                    SgExpression* other = NULL;
                    if (isSgValueExp(operand[0]) ||
                            isConstType(operand[0]->get_type())) {
                        idxv_expr = operand[1];
                        test_expr = operand[0];
                    }
                    else if (isSgValueExp(operand[1]) ||
                            isConstType(operand[1]->get_type())) {
                        idxv_expr = operand[0];
                        test_expr = operand[1];
                    }

                    // Loop through the body to find which one of the two is modified in the loop body
                    std::string o0 = operand[0]->unparseToString();
                    std::string o1 = operand[1]->unparseToString();

                    vec_checked = true;

                    // XXX: Bug in Rose, get_body() returns a SgStatement* but is, actually, a SgBasicBlock*.
                    SgBasicBlock* bb = dynamic_cast<SgBasicBlock*>(loop_body);
                    SgStatement* stmt = dynamic_cast<SgStatement*>(loop_body);
                    if (bb) {
                        SgStatementPtrList& stmts = bb->get_statements();
                        for (SgStatementPtrList::iterator it=stmts.begin(); it!=stmts.end(); it++) {
                            SgStatement* stmt = *it;
                            if (SgExprStatement* expr_stmt = isSgExprStatement(stmt)) {
                                if (in_write_set(stmt, operand[0])) {
                                    if (idxv_expr != NULL && idxv_expr->unparseToString() != o0) {
                                        // Inconclusive.
                                        idxv_expr = NULL;
                                        test_expr = NULL;
                                        incr_expr = NULL;
                                        break;
                                    } else {
                                        idxv_expr = operand[0];
                                        test_expr = operand[1];
                                        SgExpression* expr = expr_stmt->get_expression();
                                        incr_components(fileInfo, expr,
                                                incr_expr, incr_op);
                                    }
                                } else if (in_write_set(stmt, operand[1])) {
                                    if (idxv_expr != NULL && idxv_expr->unparseToString() != o1) {
                                        // Inconclusive.
                                        idxv_expr = NULL;
                                        test_expr = NULL;
                                        incr_expr = NULL;
                                        break;
                                    } else {
                                        idxv_expr = operand[1];
                                        test_expr = operand[0];
                                        SgExpression* expr = expr_stmt->get_expression();
                                        incr_components(fileInfo, expr,
                                                incr_expr, incr_op);
                                    }
                                }
                            }
                        }
                    } else if (stmt) {
                        if (SgExprStatement* expr_stmt = isSgExprStatement(stmt)) {
                            if (in_write_set(stmt, operand[0])) {
                                if (idxv_expr != NULL && idxv_expr->unparseToString() != o0) {
                                    // Inconclusive.
                                    idxv_expr = NULL;
                                    test_expr = NULL;
                                    incr_expr = NULL;
                                } else {
                                    idxv_expr = operand[0];
                                    test_expr = operand[1];
                                    SgExpression* expr = expr_stmt->get_expression();
                                    incr_components(fileInfo, expr,
                                            incr_expr, incr_op);
                                }
                            } else if (in_write_set(stmt, operand[1])) {
                                if (idxv_expr != NULL && idxv_expr->unparseToString() != o1) {
                                    // Inconclusive.
                                    idxv_expr = NULL;
                                    test_expr = NULL;
                                    incr_expr = NULL;
                                } else {
                                    idxv_expr = operand[1];
                                    test_expr = operand[0];
                                    SgExpression* expr = expr_stmt->get_expression();
                                    incr_components(fileInfo, expr,
                                            incr_expr, incr_op);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (!test_expr) return_value |= INVALID_TEST;
    if (!idxv_expr) return_value |= INVALID_IDXV;
    if (!incr_expr) return_value |= INVALID_INCR;

    return return_value;
}

SgExpression* ir_methods::rhs_expression(SgStatement* statement) {
    if (!statement)
        return false;

    SgExprStatement* expr_stmt = isSgExprStatement(statement);
    if (expr_stmt) {
        SgAssignOp* assign_op = isSgAssignOp(expr_stmt->get_expression());
        SgBinaryOp* binary_op = isSgBinaryOp(expr_stmt->get_expression());
        SgUnaryOp* unary_op = isSgUnaryOp(expr_stmt->get_expression());
        if (assign_op) {
            return assign_op->get_rhs_operand();
        } else if (unary_op) {
            return unary_op->get_operand();
        } else if (binary_op) {
            return binary_op->get_rhs_operand();
        }
    }

    return NULL;
}

bool ir_methods::in_write_set(SgStatement* statement, SgExpression* expr) {
    if (!statement || !expr)
        return false;

    std::string str_expr = expr->unparseToString();

    SgExprStatement* expr_stmt = isSgExprStatement(statement);
    if (expr_stmt) {
        SgAssignOp* assign_op = isSgAssignOp(expr_stmt->get_expression());
        SgBinaryOp* binary_op = isSgBinaryOp(expr_stmt->get_expression());
        SgUnaryOp* unary_op = isSgUnaryOp(expr_stmt->get_expression());
        if (assign_op) {
            SgExpression* lhs = assign_op->get_lhs_operand();
            if (lhs->unparseToString() == str_expr)
                return true;
        } else if (unary_op) {
            SgExpression* operand = unary_op->get_operand();
            if (operand->unparseToString() == str_expr)
                return true;
        } else if (binary_op) {
            SgExpression* lhs = binary_op->get_lhs_operand();
            if (lhs->unparseToString() == str_expr)
                return true;
        }
    }

    return false;
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
        params, const SgNode* reference_statement) {
    ROSE_ASSERT(reference_statement);

    SgNode* parent = bb->get_parent();
    SgNode* ref_parent = reference_statement->get_parent();

    if (isSgIfStmt(parent)) {
        SgIfStmt* if_node = static_cast<SgIfStmt*>(parent);
        if_node->set_use_then_keyword(true);
        if_node->set_has_end_statement(true);
    }

    SgExprStatement* fCall = buildFunctionCallStmt(SgName(function_name),
            buildVoidType(), buildExprListExp(params), bb);

    ROSE_ASSERT(ref_parent);
    fCall->set_parent(ref_parent);
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
    while (return_string.length() > 0 &&
        return_string.at(return_string.length()-1) == ']') {
        // Strip off the last [.*]
        unsigned index = return_string.find_last_of('[');
        return_string = return_string.substr(0, index);
    }

    // For fortran array notation
    while (return_string.length() > 0 &&
        return_string.at(return_string.length()-1) == ')') {
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

bool ir_methods::is_loop(SgNode* node) {
    return isSgFortranDo(node) || isSgForStatement(node) || isSgWhileStmt(node)
        || isSgDoWhileStmt(node);
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
