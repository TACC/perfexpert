
#include "rose.h"
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
        SgExpression*& incr_expr) {

    int return_value = 0;

    // Initialization
    idxv_expr = NULL;
    init_expr = NULL;
    test_expr = NULL;
    incr_expr = NULL;

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
                    test_expr = operand[0];
                }
                else if (isSgValueExp(operand[1]) ||
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

SgExprStatement* ir_methods::insert_instrumentation_call(inst_info_t& inst_info) {
    SgNode* parent = inst_info.bb->get_parent();
    if (isSgIfStmt(parent)) {
        SgIfStmt* if_node = static_cast<SgIfStmt*>(parent);
        if_node->set_use_then_keyword(true);
        if_node->set_has_end_statement(true);
    }

    SgExprStatement* fCall = buildFunctionCallStmt(
            SgName(inst_info.function_name),
            buildVoidType(),
            buildExprListExp(inst_info.params),
            inst_info.bb
            );

    if (inst_info.before) {
        insertStatementBefore(inst_info.stmt, fCall);
    } else {
        insertStatementAfter(inst_info.stmt, fCall);
    }
}
