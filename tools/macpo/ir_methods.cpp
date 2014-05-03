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

#include <map>
#include <set>
#include <string>
#include <vector>

#include "inst_defs.h"
#include "ir_methods.h"

using namespace SageBuilder;
using namespace SageInterface;

std::set<std::string> ir_methods::intrinsic_list;

bool ir_methods::is_intrinsic_function(const std::string& name) {
    // Quick check: "__builtin" prefix.
    std::string prefix("__builtin");
    if (name.compare(0, prefix.size(), prefix) == 0) {
        return true;
    }

    // Do we need to populate the list first?
    if (intrinsic_list.size() == 0) {
        _populate_intrinsic_list();
    }

    // Okay, check the long list of intrinsic functions.
    if (intrinsic_list.find(name) != intrinsic_list.end()) {
        return true;
    }

    return false;
}

void ir_methods::match_end_of_constructs(SgNode* ref_node, SgNode* node) {
    SgLocatedNode* located_stmt = isSgLocatedNode(node);
    SgLocatedNode* located_ref = isSgLocatedNode(ref_node);

    if (located_ref && located_stmt) {
        Sg_File_Info* file_info = located_ref->get_endOfConstruct();
        located_stmt->set_endOfConstruct(file_info);
    }
}

bool ir_methods::create_spans(SgExpression* expr_init, SgExpression* expr_term,
        loop_info_t& loop_info) {
    SgExpression* idxv_expr = loop_info.idxv_expr;
    SgExpression* init_expr = loop_info.init_expr;
    SgExpression* incr_expr = loop_info.incr_expr;
    int incr_op = loop_info.incr_op;

    SgExpression* final_expr = ir_methods::get_terminal_expr(idxv_expr,
            incr_expr, incr_op);

    if (ir_methods::contains_expr(expr_init, idxv_expr) && init_expr) {
        // Simple case: replace index variable with initial value.
        ir_methods::replace_expr(expr_init, idxv_expr, init_expr);

        // The more complicated case: replace index variable with
        // the value just before exiting the loop.
        // Not precise, but get's us by most of the time.
        final_expr->set_parent(expr_term);
        SgLocatedNode* t = isSgLocatedNode(final_expr);
        SgLocatedNode* c = isSgLocatedNode(expr_term);

        if (t && c)
            t->set_endOfConstruct(c->get_endOfConstruct());

        ir_methods::replace_expr(expr_term, idxv_expr, final_expr);
        return true;
    }

    return false;
}

std::vector<SgStatement*> ir_methods::get_var_decls(SgForStatement* for_stmt) {
    std::vector<SgStatement*> var_decls;
    SgForInitStatement* for_init_stmt = for_stmt->get_for_init_stmt();
    SgStatementPtrList init_list = for_init_stmt->get_init_stmt();
    if (init_list.size()) {
        for (SgStatementPtrList::iterator it = init_list.begin();
                it != init_list.end(); it++) {
            SgStatement* stmt = *it;
            SgVariableDeclaration* var_decl = NULL;
            if (var_decl = isSgVariableDeclaration(stmt)) {
                var_decls.push_back(var_decl);
            } else if (SgExprStatement* expr_stmt = isSgExprStatement(stmt)) {
                SgExpression* expr = expr_stmt->get_expression();
                if (isSgAssignOp(expr)) {
                    var_decls.push_back(expr_stmt);
                }
            }
        }
    }

    return var_decls;
}

SgExpression* ir_methods::get_terminal_expr(SgExpression* idxv,
        SgExpression* incr, int incr_op) {
    SgBinaryOp* op = NULL;

    SgType* type = idxv->get_type();
    Sg_File_Info* file_info = idxv->get_file_info();

    // For each known operation, invert the operation
    // using the same index and increment expressions.
    switch (incr_op) {
        case OP_ADD:
            op = new SgSubtractOp(file_info, idxv, incr, type);
            break;

        case OP_SUB:
            op = new SgAddOp(file_info, idxv, incr, type);
            break;

        case OP_MUL:
            op = new SgDivideOp(file_info, idxv, incr, type);
            break;

        case OP_DIV:
            op = new SgMultiplyOp(file_info, idxv, incr, type);
            break;
    }

    if (op != NULL) {
        op->set_endOfConstruct(file_info);
        return op;
    }

    // If we don't know the operation,
    // just return without any modifications.
    return idxv;
}

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
            SgExpression *param_addr = reinterpret_cast<SgExpression*>(pntr);

            // Strip unary operators like ++ or -- from the expression.
            param_addr = ir_methods::strip_unary_operators(expr);
            ROSE_ASSERT(param_addr && "Bug in stripping unary operators from "
                    "given expression!");

            if (is_Fortran_language() == false) {
                SgAddressOfOp* address_op = NULL;
                SgType* void_pointer_type = NULL;

                address_op = buildAddressOfOp(param_addr);
                void_pointer_type = buildPointerType(buildVoidType());
                param_addr = buildCastExp(address_op, void_pointer_type);
            }

            addresses.push_back(param_addr);
        }
    }

    if (addresses.size() > 0) {
        std::vector<SgExpression*> params;

        int line_number = loop_stmt->get_file_info()->get_raw_line();
        SgIntVal* val_line_number = new SgIntVal(fileInfo, line_number);
        val_line_number->set_endOfConstruct(fileInfo);
        params.push_back(val_line_number);

        int address_count = addresses.size();
        SgIntVal* val_address_count = new SgIntVal(fileInfo, address_count);
        val_address_count->set_endOfConstruct(fileInfo);
        params.push_back(val_address_count);

        // Now push all of the addresses.
        params.insert(params.end(), addresses.begin(), addresses.end());

        SgStatement* first_statement = loop_stmt->firstStatement();
        SgBasicBlock* first_bb =
            getEnclosingNode<SgBasicBlock>(first_statement);

        std::string function_name = prefix + (is_Fortran_language() ? "_f" :
                "_c");
        SgStatement* call_stmt = ir_methods::prepare_call_statement(first_bb,
                function_name, params, first_statement);

        SgBasicBlock* aligncheck_list = new SgBasicBlock(fileInfo);
        aligncheck_list->set_endOfConstruct(fileInfo);
        aligncheck_list->append_statement(call_stmt);

        // Create new integer variable called
        // "indigo__aligncheck_init_<line_number>". Funky, eh?
        char var_name[64];
        snprintf (var_name, sizeof(var_name), "%s_%d", prefix.c_str(),
                line_number);
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
        SgNode* ref_node = reinterpret_cast<SgNode*>(reference_stmt);
        SgNode* omp_node = reinterpret_cast<SgNode*>(omp_body_stmt);
        if (omp_body_stmt && ir_methods::is_ancestor(ref_node, omp_node)
                == false) {
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
        SgLongIntVal* val_zero = new SgLongIntVal(fileInfo, 0);
        val_zero->set_endOfConstruct(fileInfo);
        guard_condition = new SgEqualityOp(fileInfo, buildVarRefExp(var_name),
                val_zero, long_type);
        guard_condition->set_endOfConstruct(fileInfo);

        SgExprStatement* guard_condition_stmt = NULL;
        guard_condition_stmt = new SgExprStatement(fileInfo, guard_condition);
        guard_condition->set_endOfConstruct(fileInfo);
        guard_condition->set_parent(guard_condition_stmt);

        // Create statement to reset guard value.
        SgExprStatement* reset_guard_stmt = NULL;
        SgIntVal* val_one = new SgIntVal(fileInfo, 1);
        val_one->set_endOfConstruct(fileInfo);
        reset_guard_stmt = ir_methods::create_long_assign_statement(fileInfo,
                var_name, val_one);
        aligncheck_list->append_statement(reset_guard_stmt);
        reset_guard_stmt->set_parent(aligncheck_list);

        // Insert the guard condition.
        if (SgIfStmt* init_guard = new SgIfStmt(fileInfo)) {
            init_guard->set_conditional(guard_condition_stmt);
            init_guard->set_true_body(aligncheck_list);
            init_guard->set_endOfConstruct(fileInfo);
            init_guard->set_parent(loop_stmt);

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
    if (expr_list.size() <= 1) {
        return;
    }

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

SgExprStatement* ir_methods::create_long_assign_statement(Sg_File_Info*
        fileInfo, const std::string& name, SgIntVal* value) {
    SgType* long_type = buildLongType();
    SgVarRefExp* expr = buildVarRefExp(name);
    SgAssignOp* assign_op = new SgAssignOp(fileInfo, expr, value, long_type);
    SgExprStatement* assign_stmt = new SgExprStatement(fileInfo, assign_op);

    assign_op->set_endOfConstruct(fileInfo);
    assign_stmt->set_endOfConstruct(fileInfo);

    return assign_stmt;
}

SgExprStatement* ir_methods::create_long_incr_statement(Sg_File_Info* fileInfo,
        const std::string& name) {
    SgType* long_type = buildLongType();
    SgVarRefExp* expr = buildVarRefExp(name);
    SgPlusPlusOp* incr_op = new SgPlusPlusOp(fileInfo, expr, long_type);
    SgExprStatement* incr_statement = new SgExprStatement(fileInfo, incr_op);

    incr_op->set_endOfConstruct(fileInfo);
    incr_statement->set_endOfConstruct(fileInfo);

    return incr_statement;
}

SgVariableDeclaration* ir_methods::create_long_variable(Sg_File_Info* fileInfo,
        const std::string& name, int64_t init_value) {
    SgType* long_type = buildLongType();
    SgVariableDeclaration* var_decl = new SgVariableDeclaration(fileInfo,
            name, long_type, buildAssignInitializer(buildIntVal(init_value)));

    var_decl->set_endOfConstruct(fileInfo);
    return var_decl;
}

SgVariableDeclaration* ir_methods::create_long_variable_with_init(
        Sg_File_Info* fileInfo, const std::string& name,
        SgExpression* initializer) {
    SgType* long_type = buildLongType();
    SgVariableDeclaration* var_decl = new SgVariableDeclaration(fileInfo,
            name, long_type, buildAssignInitializer(initializer));

    var_decl->set_endOfConstruct(fileInfo);
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
    } else if (scope_stmt) {
        const SgStatementPtrList stmt_list =
            scope_stmt->generateStatementList();
        for (int i = 0; i < stmt_list.size(); i++) {
            SgStatement* inner_stmt = stmt_list[i];
            if (!vectorizable(inner_stmt)) {
                return false;
            }
        }
    } else if (SgExprStatement* expr_stmt = isSgExprStatement(stmt)) {
        SgExpression* expr = expr_stmt->get_expression();
        if (SgFunctionCallExp* call = isSgFunctionCallExp(expr)) {
            SgFunctionDeclaration* decl = NULL;
            decl = call->getAssociatedFunctionDeclaration();

            // Or whether this is an intrinsic math function.
            std::string& name = decl->get_name().getString();
            if (ir_methods::is_intrinsic_function(name)) {
                return true;
            }

            // If the function body is available, test it.
            SgFunctionDefinition* def = decl->get_definition();
            if (def) {
                SgStatement* statement = def->get_body();
                if (statement && vectorizable(statement)) {
                    return true;
                }
            }

            return false;
        }
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
        if (SgMinusAssignOp* minus_op = isSgMinusAssignOp(incr_binary_op)) {
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
            incr_expr->set_endOfConstruct(fileInfo);
        } else if (SgPlusPlusOp* plus_op = isSgPlusPlusOp(incr_unary_op)) {
            incr_op = OP_ADD;
            incr_expr = new SgIntVal(fileInfo, 1);
            incr_expr->set_endOfConstruct(fileInfo);
        }
    }
}

int ir_methods::get_for_loop_header_components(VariableRenaming*& var_renaming,
        SgForStatement*& for_stmt, def_map_t& def_map, SgExpression*&
        idxv_expr, SgExpression*& init_expr, SgExpression*& test_expr,
        SgExpression*& incr_expr, int& incr_op) {
    int return_value = 0;
    SgExpression* increment_var[2] = {0};

    SgLocatedNode* located_for = reinterpret_cast<SgLocatedNode*>(for_stmt);
    Sg_File_Info *fileInfo =
        Sg_File_Info::generateFileInfoForTransformationNode(
                located_for->get_file_info()->get_filenameString());
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
                    for (int i = 0; i < 2 && !terminate; i++) {
                        if (increment_var[i]) {
                            std::string increment_str =
                                increment_var[i]->unparseToString();

                            for (int j = 0; j < 2 && !terminate; j++) {
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
                    for (int i = 0; i < 2; i++) {
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
                } else if (SgVariableDeclaration* var_decl =
                        isSgVariableDeclaration(stmt)) {
                    std::vector<SgInitializedName*> var_list =
                        var_decl->get_variables();
                    for (int i = 0; i < var_list.size(); i++) {
                        SgInitializedName* init_name = var_list.at(i);
                        SgName name = init_name->get_qualified_name();
                        if (name.getString() == idxv_string) {
                            SgAssignInitializer* init_ptr =
                                isSgAssignInitializer(init_name->get_initptr());
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

    // Check if the index variable is being
    // modified (again) in the loop body.
    if (idxv_expr && incr_expr) {
        SgStatement* loop_body = for_stmt->get_loop_body();
        if (SgBasicBlock* loop_bb = isSgBasicBlock(loop_body)) {
            SgStatementPtrList& stmts = loop_bb->get_statements();
            for (SgStatementPtrList::iterator it = stmts.begin();
                    it != stmts.end(); it++) {
                if (in_write_set(*it, idxv_expr)) {
                    idxv_expr = NULL;
                    incr_expr = NULL;
                    incr_op = ir_methods::INVALID_OP;
                    break;
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

    SgLocatedNode* located_scope = reinterpret_cast<SgLocatedNode*>(scope_stmt);
    Sg_File_Info *fileInfo =
        Sg_File_Info::generateFileInfoForTransformationNode(
                located_scope->get_file_info()->get_filenameString());

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

                        // Check if the loop body references
                        // the index variable again.
                        SgBasicBlock* bb =
                            dynamic_cast<SgBasicBlock*>(loop_body);
                        SgStatement* stmt =
                            dynamic_cast<SgStatement*>(loop_body);
                        if (bb) {
                            SgStatementPtrList& stmts = bb->get_statements();
                            for (SgStatementPtrList::iterator it =
                                    stmts.begin(); it != stmts.end(); it++) {
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
                    } else if (isSgValueExp(operand[1]) ||
                            isConstType(operand[1]->get_type())) {
                        idxv_expr = operand[0];
                        test_expr = operand[1];
                    }

                    // Loop through the body to find which one of the two
                    // is modified in the loop body
                    std::string o0 = operand[0]->unparseToString();
                    std::string o1 = operand[1]->unparseToString();

                    vec_checked = true;

                    // XXX: Bug in Rose, get_body() returns a SgStatement* but
                    // is, actually, a SgBasicBlock*.
                    SgBasicBlock* bb = dynamic_cast<SgBasicBlock*>(loop_body);
                    SgStatement* stmt = dynamic_cast<SgStatement*>(loop_body);
                    if (bb) {
                        SgStatementPtrList& stmts = bb->get_statements();
                        for (SgStatementPtrList::iterator it = stmts.begin();
                                it != stmts.end(); it++) {
                            SgStatement* stmt = *it;
                            if (SgExprStatement* expr_stmt =
                                    isSgExprStatement(stmt)) {
                                if (in_write_set(stmt, operand[0])) {
                                    if (idxv_expr != NULL &&
                                            idxv_expr->unparseToString()
                                            != o0) {
                                        // Inconclusive.
                                        idxv_expr = NULL;
                                        test_expr = NULL;
                                        incr_expr = NULL;
                                        break;
                                    } else {
                                        idxv_expr = operand[0];
                                        test_expr = operand[1];
                                        SgExpression* expr =
                                            expr_stmt->get_expression();
                                        incr_components(fileInfo, expr,
                                                incr_expr, incr_op);
                                    }
                                } else if (in_write_set(stmt, operand[1])) {
                                    if (idxv_expr != NULL &&
                                            idxv_expr->unparseToString()
                                            != o1) {
                                        // Inconclusive.
                                        idxv_expr = NULL;
                                        test_expr = NULL;
                                        incr_expr = NULL;
                                        break;
                                    } else {
                                        idxv_expr = operand[1];
                                        test_expr = operand[0];
                                        SgExpression* expr =
                                            expr_stmt->get_expression();
                                        incr_components(fileInfo, expr,
                                                incr_expr, incr_op);
                                    }
                                }
                            }
                        }
                    } else if (stmt) {
                        if (SgExprStatement* expr_stmt =
                                isSgExprStatement(stmt)) {
                            if (in_write_set(stmt, operand[0])) {
                                if (idxv_expr != NULL &&
                                        idxv_expr->unparseToString() != o0) {
                                    // Inconclusive.
                                    idxv_expr = NULL;
                                    test_expr = NULL;
                                    incr_expr = NULL;
                                } else {
                                    idxv_expr = operand[0];
                                    test_expr = operand[1];
                                    SgExpression* expr =
                                        expr_stmt->get_expression();
                                    incr_components(fileInfo, expr,
                                            incr_expr, incr_op);
                                }
                            } else if (in_write_set(stmt, operand[1])) {
                                if (idxv_expr != NULL &&
                                        idxv_expr->unparseToString() != o1) {
                                    // Inconclusive.
                                    idxv_expr = NULL;
                                    test_expr = NULL;
                                    incr_expr = NULL;
                                } else {
                                    idxv_expr = operand[1];
                                    test_expr = operand[0];
                                    SgExpression* expr =
                                        expr_stmt->get_expression();
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

void ir_methods::construct_def_map(VariableRenaming::NumNodeRenameTable&
        rename_table, def_map_t& def_map) {
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

int64_t ir_methods::get_reference_index(reference_list_t& reference_list,
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

bool ir_methods::contains_expr(SgExpression*& expr,
        SgExpression*& search_expr) {
    SgBinaryOp* bin_op = isSgBinaryOp(expr);

    if (bin_op && search_expr) {
        SgExpression* lhs = bin_op->get_lhs_operand();
        SgExpression* rhs = bin_op->get_rhs_operand();

        // If either operand matches, replace it!
        if (lhs && lhs->unparseToString() == search_expr->unparseToString())
            return true;

        if (rhs && rhs->unparseToString() == search_expr->unparseToString())
            return true;

        // Check if we need to call recursively.
        if (lhs && ir_methods::contains_expr(lhs, search_expr))
            return true;

        if (rhs && ir_methods::contains_expr(rhs, search_expr))
            return true;
    }

    return false;
}

void ir_methods::replace_expr(SgExpression*& expr, SgExpression*& search_expr,
        SgExpression*& replace_expr) {
    if (search_expr == NULL || replace_expr == NULL)
        return;

    if (SgBinaryOp* bin_op = isSgBinaryOp(expr)) {
        SgExpression* lhs = bin_op->get_lhs_operand();
        SgExpression* rhs = bin_op->get_rhs_operand();

        // If either operand matches, replace it!
        if (lhs && lhs->unparseToString() == search_expr->unparseToString()) {
            bin_op->set_lhs_operand(replace_expr);
        } else if (lhs) {
            ir_methods::replace_expr(lhs, search_expr, replace_expr);
        }

        if (rhs && rhs->unparseToString() == search_expr->unparseToString()) {
            bin_op->set_rhs_operand(replace_expr);
        } else if (rhs) {
            ir_methods::replace_expr(rhs, search_expr, replace_expr);
        }
    } else if (SgUnaryOp* unary_op = isSgUnaryOp(expr)) {
        SgExpression* op = unary_op->get_operand();
        if (op && op->unparseToString() == search_expr->unparseToString()) {
            unary_op->set_operand(replace_expr);
        } else if (op) {
            ir_methods::replace_expr(op, search_expr, replace_expr);
        }
    } else if (expr->unparseToString() == search_expr->unparseToString()) {
        expr = replace_expr;
    }
}

bool ir_methods::is_loop(SgNode* node) {
    return isSgFortranDo(node) || isSgForStatement(node) || isSgWhileStmt(node)
        || isSgDoWhileStmt(node);
}

void ir_methods::_populate_intrinsic_list() {
    // Check if the list is already populated. I know we are inserting into
    // a set, so duplicates will automatically be removed. But insertion and
    // duplicate checks will still incur a perf penalty.
    if (intrinsic_list.size() > 0)
        return;

    intrinsic_list.insert("abort");
    intrinsic_list.insert("abs");
    intrinsic_list.insert("acos");
    intrinsic_list.insert("acosf");
    intrinsic_list.insert("acosh");
    intrinsic_list.insert("acoshf");
    intrinsic_list.insert("acoshl");
    intrinsic_list.insert("acosl");
    intrinsic_list.insert("alloca");
    intrinsic_list.insert("asin");
    intrinsic_list.insert("asinf");
    intrinsic_list.insert("asinh");
    intrinsic_list.insert("asinhf");
    intrinsic_list.insert("asinhl");
    intrinsic_list.insert("asinl");
    intrinsic_list.insert("atan");
    intrinsic_list.insert("atan2");
    intrinsic_list.insert("atan2f");
    intrinsic_list.insert("atan2l");
    intrinsic_list.insert("atanf");
    intrinsic_list.insert("atanh");
    intrinsic_list.insert("atanhf");
    intrinsic_list.insert("atanhl");
    intrinsic_list.insert("atanl");
    intrinsic_list.insert("bcmp");
    intrinsic_list.insert("bzero");
    intrinsic_list.insert("cabs");
    intrinsic_list.insert("cabsf");
    intrinsic_list.insert("cabsl");
    intrinsic_list.insert("cacos");
    intrinsic_list.insert("cacosf");
    intrinsic_list.insert("cacosh");
    intrinsic_list.insert("cacoshf");
    intrinsic_list.insert("cacoshl");
    intrinsic_list.insert("cacosl");
    intrinsic_list.insert("calloc");
    intrinsic_list.insert("carg");
    intrinsic_list.insert("cargf");
    intrinsic_list.insert("cargl");
    intrinsic_list.insert("casin");
    intrinsic_list.insert("casinf");
    intrinsic_list.insert("casinh");
    intrinsic_list.insert("casinhf");
    intrinsic_list.insert("casinhl");
    intrinsic_list.insert("casinl");
    intrinsic_list.insert("catan");
    intrinsic_list.insert("catanf");
    intrinsic_list.insert("catanh");
    intrinsic_list.insert("catanhf");
    intrinsic_list.insert("catanhl");
    intrinsic_list.insert("catanl");
    intrinsic_list.insert("cbrt");
    intrinsic_list.insert("cbrtf");
    intrinsic_list.insert("cbrtl");
    intrinsic_list.insert("ccos");
    intrinsic_list.insert("ccosf");
    intrinsic_list.insert("ccosh");
    intrinsic_list.insert("ccoshf");
    intrinsic_list.insert("ccoshl");
    intrinsic_list.insert("ccosl");
    intrinsic_list.insert("ceil");
    intrinsic_list.insert("ceilf");
    intrinsic_list.insert("ceill");
    intrinsic_list.insert("cexp");
    intrinsic_list.insert("cexpf");
    intrinsic_list.insert("cexpl");
    intrinsic_list.insert("cimag");
    intrinsic_list.insert("cimagf");
    intrinsic_list.insert("cimagl");
    intrinsic_list.insert("clog");
    intrinsic_list.insert("clogf");
    intrinsic_list.insert("clogl");
    intrinsic_list.insert("conj");
    intrinsic_list.insert("conjf");
    intrinsic_list.insert("conjl");
    intrinsic_list.insert("copysign");
    intrinsic_list.insert("copysignf");
    intrinsic_list.insert("copysignl");
    intrinsic_list.insert("cos");
    intrinsic_list.insert("cosf");
    intrinsic_list.insert("cosh");
    intrinsic_list.insert("coshf");
    intrinsic_list.insert("coshl");
    intrinsic_list.insert("cosl");
    intrinsic_list.insert("cpow");
    intrinsic_list.insert("cpowf");
    intrinsic_list.insert("cpowl");
    intrinsic_list.insert("cproj");
    intrinsic_list.insert("cprojf");
    intrinsic_list.insert("cprojl");
    intrinsic_list.insert("creal");
    intrinsic_list.insert("crealf");
    intrinsic_list.insert("creall");
    intrinsic_list.insert("csin");
    intrinsic_list.insert("csinf");
    intrinsic_list.insert("csinh");
    intrinsic_list.insert("csinhf");
    intrinsic_list.insert("csinhl");
    intrinsic_list.insert("csinl");
    intrinsic_list.insert("csqrt");
    intrinsic_list.insert("csqrtf");
    intrinsic_list.insert("csqrtl");
    intrinsic_list.insert("ctan");
    intrinsic_list.insert("ctanf");
    intrinsic_list.insert("ctanh");
    intrinsic_list.insert("ctanhf");
    intrinsic_list.insert("ctanhl");
    intrinsic_list.insert("ctanl");
    intrinsic_list.insert("dcgettext");
    intrinsic_list.insert("dgettext");
    intrinsic_list.insert("drem");
    intrinsic_list.insert("dremf");
    intrinsic_list.insert("dreml");
    intrinsic_list.insert("erf");
    intrinsic_list.insert("erfc");
    intrinsic_list.insert("erfcf");
    intrinsic_list.insert("erfcl");
    intrinsic_list.insert("erff");
    intrinsic_list.insert("erfl");
    intrinsic_list.insert("exit");
    intrinsic_list.insert("_exit");
    intrinsic_list.insert("_Exit");
    intrinsic_list.insert("exp");
    intrinsic_list.insert("exp10");
    intrinsic_list.insert("exp10f");
    intrinsic_list.insert("exp10l");
    intrinsic_list.insert("exp2");
    intrinsic_list.insert("exp2f");
    intrinsic_list.insert("exp2l");
    intrinsic_list.insert("expf");
    intrinsic_list.insert("expl");
    intrinsic_list.insert("expm1");
    intrinsic_list.insert("expm1f");
    intrinsic_list.insert("expm1l");
    intrinsic_list.insert("fabs");
    intrinsic_list.insert("fabsf");
    intrinsic_list.insert("fabsl");
    intrinsic_list.insert("fdim");
    intrinsic_list.insert("fdimf");
    intrinsic_list.insert("fdiml");
    intrinsic_list.insert("ffs");
    intrinsic_list.insert("ffsl");
    intrinsic_list.insert("ffsll");
    intrinsic_list.insert("floor");
    intrinsic_list.insert("floorf");
    intrinsic_list.insert("floorl");
    intrinsic_list.insert("fma");
    intrinsic_list.insert("fmaf");
    intrinsic_list.insert("fmal");
    intrinsic_list.insert("fmax");
    intrinsic_list.insert("fmaxf");
    intrinsic_list.insert("fmaxl");
    intrinsic_list.insert("fmin");
    intrinsic_list.insert("fminf");
    intrinsic_list.insert("fminl");
    intrinsic_list.insert("fmod");
    intrinsic_list.insert("fmodf");
    intrinsic_list.insert("fmodl");
    intrinsic_list.insert("fpclassify");
    intrinsic_list.insert("fprintf");
    intrinsic_list.insert("fprintf_unlocked");
    intrinsic_list.insert("fputs");
    intrinsic_list.insert("fputs_unlocked");
    intrinsic_list.insert("frexp");
    intrinsic_list.insert("frexpf");
    intrinsic_list.insert("frexpl");
    intrinsic_list.insert("fscanf");
    intrinsic_list.insert("gamma");
    intrinsic_list.insert("gammaf");
    intrinsic_list.insert("gammaf_r");
    intrinsic_list.insert("gammal");
    intrinsic_list.insert("gammal_r");
    intrinsic_list.insert("gamma_r");
    intrinsic_list.insert("gettext");
    intrinsic_list.insert("hypot");
    intrinsic_list.insert("hypotf");
    intrinsic_list.insert("hypotl");
    intrinsic_list.insert("ilogb");
    intrinsic_list.insert("ilogbf");
    intrinsic_list.insert("ilogbl");
    intrinsic_list.insert("imaxabs");
    intrinsic_list.insert("index");
    intrinsic_list.insert("isalnum");
    intrinsic_list.insert("isalpha");
    intrinsic_list.insert("isascii");
    intrinsic_list.insert("isblank");
    intrinsic_list.insert("iscntrl");
    intrinsic_list.insert("isdigit");
    intrinsic_list.insert("isfinite");
    intrinsic_list.insert("isgraph");
    intrinsic_list.insert("isgreater");
    intrinsic_list.insert("isgreaterequal");
    intrinsic_list.insert("isinf_sign");
    intrinsic_list.insert("isless");
    intrinsic_list.insert("islessequal");
    intrinsic_list.insert("islessgreater");
    intrinsic_list.insert("islower");
    intrinsic_list.insert("isnormal");
    intrinsic_list.insert("isprint");
    intrinsic_list.insert("ispunct");
    intrinsic_list.insert("isspace");
    intrinsic_list.insert("isunordered");
    intrinsic_list.insert("isupper");
    intrinsic_list.insert("iswblank");
    intrinsic_list.insert("isxdigit");
    intrinsic_list.insert("j0");
    intrinsic_list.insert("j0f");
    intrinsic_list.insert("j0l");
    intrinsic_list.insert("j1");
    intrinsic_list.insert("j1f");
    intrinsic_list.insert("j1l");
    intrinsic_list.insert("jn");
    intrinsic_list.insert("jnf");
    intrinsic_list.insert("jnl");
    intrinsic_list.insert("labs");
    intrinsic_list.insert("ldexp");
    intrinsic_list.insert("ldexpf");
    intrinsic_list.insert("ldexpl");
    intrinsic_list.insert("lgamma");
    intrinsic_list.insert("lgammaf");
    intrinsic_list.insert("lgammaf_r");
    intrinsic_list.insert("lgammal");
    intrinsic_list.insert("lgammal_r");
    intrinsic_list.insert("lgamma_r");
    intrinsic_list.insert("llabs");
    intrinsic_list.insert("llrint");
    intrinsic_list.insert("llrintf");
    intrinsic_list.insert("llrintl");
    intrinsic_list.insert("llround");
    intrinsic_list.insert("llroundf");
    intrinsic_list.insert("llroundl");
    intrinsic_list.insert("log");
    intrinsic_list.insert("log10");
    intrinsic_list.insert("log10f");
    intrinsic_list.insert("log10l");
    intrinsic_list.insert("log1p");
    intrinsic_list.insert("log1pf");
    intrinsic_list.insert("log1pl");
    intrinsic_list.insert("log2");
    intrinsic_list.insert("log2f");
    intrinsic_list.insert("log2l");
    intrinsic_list.insert("logb");
    intrinsic_list.insert("logbf");
    intrinsic_list.insert("logbl");
    intrinsic_list.insert("logf");
    intrinsic_list.insert("logl");
    intrinsic_list.insert("lrint");
    intrinsic_list.insert("lrintf");
    intrinsic_list.insert("lrintl");
    intrinsic_list.insert("lround");
    intrinsic_list.insert("lroundf");
    intrinsic_list.insert("lroundl");
    intrinsic_list.insert("malloc");
    intrinsic_list.insert("memchr");
    intrinsic_list.insert("memcmp");
    intrinsic_list.insert("memcpy");
    intrinsic_list.insert("mempcpy");
    intrinsic_list.insert("memset");
    intrinsic_list.insert("modf");
    intrinsic_list.insert("modfl");
    intrinsic_list.insert("nearbyint");
    intrinsic_list.insert("nearbyintf");
    intrinsic_list.insert("nearbyintl");
    intrinsic_list.insert("nextafter");
    intrinsic_list.insert("nextafterf");
    intrinsic_list.insert("nextafterl");
    intrinsic_list.insert("nexttoward");
    intrinsic_list.insert("nexttowardf");
    intrinsic_list.insert("nexttowardl");
    intrinsic_list.insert("pow");
    intrinsic_list.insert("pow10");
    intrinsic_list.insert("pow10f");
    intrinsic_list.insert("pow10l");
    intrinsic_list.insert("powf");
    intrinsic_list.insert("powl");
    intrinsic_list.insert("printf");
    intrinsic_list.insert("printf_unlocked");
    intrinsic_list.insert("putchar");
    intrinsic_list.insert("puts");
    intrinsic_list.insert("remainder");
    intrinsic_list.insert("remainderf");
    intrinsic_list.insert("remainderl");
    intrinsic_list.insert("remquo");
    intrinsic_list.insert("remquof");
    intrinsic_list.insert("remquol");
    intrinsic_list.insert("rindex");
    intrinsic_list.insert("rint");
    intrinsic_list.insert("rintf");
    intrinsic_list.insert("rintl");
    intrinsic_list.insert("round");
    intrinsic_list.insert("roundf");
    intrinsic_list.insert("roundl");
    intrinsic_list.insert("scalb");
    intrinsic_list.insert("scalbf");
    intrinsic_list.insert("scalbl");
    intrinsic_list.insert("scalbln");
    intrinsic_list.insert("scalblnf");
    intrinsic_list.insert("scalblnl");
    intrinsic_list.insert("scalbn");
    intrinsic_list.insert("scalbnf");
    intrinsic_list.insert("scalbnl");
    intrinsic_list.insert("scanf");
    intrinsic_list.insert("signbit");
    intrinsic_list.insert("signbitd128");
    intrinsic_list.insert("signbitd32");
    intrinsic_list.insert("signbitd64");
    intrinsic_list.insert("signbitf");
    intrinsic_list.insert("signbitl");
    intrinsic_list.insert("significand");
    intrinsic_list.insert("significandf");
    intrinsic_list.insert("significandl");
    intrinsic_list.insert("sin");
    intrinsic_list.insert("sincos");
    intrinsic_list.insert("sincosf");
    intrinsic_list.insert("sincosl");
    intrinsic_list.insert("sinf");
    intrinsic_list.insert("sinh");
    intrinsic_list.insert("sinhf");
    intrinsic_list.insert("sinhl");
    intrinsic_list.insert("sinl");
    intrinsic_list.insert("snprintf");
    intrinsic_list.insert("sprintf");
    intrinsic_list.insert("sqrt");
    intrinsic_list.insert("sqrtf");
    intrinsic_list.insert("sqrtl");
    intrinsic_list.insert("sscanf");
    intrinsic_list.insert("stpcpy");
    intrinsic_list.insert("stpncpy");
    intrinsic_list.insert("strcasecmp");
    intrinsic_list.insert("strcat");
    intrinsic_list.insert("strchr");
    intrinsic_list.insert("strcmp");
    intrinsic_list.insert("strcpy");
    intrinsic_list.insert("strcspn");
    intrinsic_list.insert("strdup");
    intrinsic_list.insert("strfmon");
    intrinsic_list.insert("strlen");
    intrinsic_list.insert("strncasecmp");
    intrinsic_list.insert("strncat");
    intrinsic_list.insert("strncmp");
    intrinsic_list.insert("strncpy");
    intrinsic_list.insert("strndup");
    intrinsic_list.insert("strpbrk");
    intrinsic_list.insert("strrchr");
    intrinsic_list.insert("strspn");
    intrinsic_list.insert("strstr");
    intrinsic_list.insert("tan");
    intrinsic_list.insert("tanf");
    intrinsic_list.insert("tanh");
    intrinsic_list.insert("tanhf");
    intrinsic_list.insert("tanhl");
    intrinsic_list.insert("tanl");
    intrinsic_list.insert("tgamma");
    intrinsic_list.insert("tgammaf");
    intrinsic_list.insert("tgammal");
    intrinsic_list.insert("toascii");
    intrinsic_list.insert("tolower");
    intrinsic_list.insert("toupper");
    intrinsic_list.insert("trunc");
    intrinsic_list.insert("truncf");
    intrinsic_list.insert("truncl");
    intrinsic_list.insert("vfprintf");
    intrinsic_list.insert("vfscanf");
    intrinsic_list.insert("vprintf");
    intrinsic_list.insert("vscanf");
    intrinsic_list.insert("vsnprintf");
    intrinsic_list.insert("vsprintf");
    intrinsic_list.insert("vsscanf");
    intrinsic_list.insert("y0");
    intrinsic_list.insert("y0f");
    intrinsic_list.insert("y0l");
    intrinsic_list.insert("y1");
    intrinsic_list.insert("y1f");
    intrinsic_list.insert("y1l");
    intrinsic_list.insert("yn");
    intrinsic_list.insert("ynf");
    intrinsic_list.insert("ynl");
}
