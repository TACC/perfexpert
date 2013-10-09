
#include <rose.h>

#include "rose.h"
#include "aligncheck.h"

using namespace SageBuilder;
using namespace SageInterface;

void aligncheck_t::atTraversalStart() {
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
        // Sanity check to see if this loop contains statements that prevent vectorization.
        SgStatement* first_stmt = getFirstStatement(for_stmt);
        SgStatement* stmt = first_stmt;
        while (stmt) {
            if (!vectorizable(stmt)) {
                std::cout << "This loop cannot be vectorized because of the following statement: " << stmt->unparseToString() << "\n";
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

        SgExprStatement* test_statement = isSgExprStatement(for_stmt->get_test());
        if (test_statement) {
            SgExpression* test_expression = test_statement->get_expression();
            if (test_expression) {
                SgBinaryOp* binaryOp = isSgBinaryOp(test_expression);
                if (binaryOp) {
                    SgExpression* operand[2];
                    operand[0] = binaryOp->get_lhs_operand();
                    operand[1] = binaryOp->get_rhs_operand();

                    SgExpression* other = NULL;
                    if (isSgValueExp(operand[0]) || isConstType(operand[0]->get_type())) other = operand[1];
                    else if (isSgValueExp(operand[1]) || isConstType(operand[1]->get_type())) other = operand[0];

                    if (other == NULL) {
                        // Is one of these the index variable and other some other variable?
                        // XXX: Hack: we should be checking if the AST nodes are the same
                        // but such an API does not seem to exist, hence checking string representation.
                        std::string operand_str[2];
                        operand_str[0] = operand[0]->unparseToString();
                        operand_str[1] = operand[1]->unparseToString();

                        bool terminate = false;
                        for (int i=0; i<2 && !terminate; i++) {
                            if (increment_var[i]) {
                                std::string increment_str = increment_var[i]->unparseToString();

                                for (int j=0; j<2 && !terminate; j++) {
                                    if (increment_str == operand_str[j]) {
                                        // The variable that's common between test and increment is the index variable.
                                        index_expr = increment_var[i];

                                        // While the uncommon variable is the test expression to be instrumented.
                                        instrument_test = operand[1-i];

                                        // The other increment expression may need to be instrumented
                                        SgExpression* incr = increment_var[1-i];
                                        if (incr && !isSgValueExp(incr) && !isConstType(incr->get_type()))
                                            instrument_incr = incr;

                                        // We're done with this loop.
                                        terminate = true;
                                    }
                                }
                            }
                        }
                    } else {
                        // Check if this is the same variable being used in the increment expression
                        std::string var_string = other->unparseToString();
                        for (int i=0; i<2; i++) {
                            if ((increment_var[i] && increment_var[i]->unparseToString() == var_string)) {
                                index_expr = other;

                                // The other increment expression may need to be instrumented
                                SgExpression* incr = increment_var[1-i];
                                if (incr && !isSgValueExp(incr) && !isConstType(incr->get_type()))
                                    instrument_incr = incr;
                            }
                        }
                    }
                }
            }
        }
    }

    if (instrument_test) {
        std::cout << "The following variable in the test expression needs to be instrumented: " << instrument_test->unparseToString() << "\n";
    }

    if (instrument_incr) {
        std::cout << "The following variable in the increment expression needs to be instrumented: " << instrument_incr->unparseToString() << "\n";
    }

    return attr;
}
