
#include <rose.h>

#include "expander.h"
#include "scourer.h"

using namespace SageBuilder;
using namespace SageInterface;

static int ctr=0;

attrib expander_t::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	// If explicit instructions to skip this node, then just return
	if (attr.skip == TRUE)
		return attr;

	SgNode* parent = node->get_parent();

	// Decide whether read / write depending on the operand of AssignOp
	if (parent && isSgAssignOp(parent))
		attr.read = parent->getChildIndex(node) == 0 ? TYPE_WRITE : TYPE_READ;

	// LHS operand of PntrArrRefExp is always skipped
	if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) == 0)
	{
		attr.skip=TRUE;
		return attr;
	}

	// RHS operand of PntrArrRefExp is always read and never written
	if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)
		attr.read = true;

	SgFortranDo* doLoop = getEnclosingNode<SgFortranDo>(node);
	if (!doLoop)
		return attr;

	if (parent == doLoop && !isSgBasicBlock(node))
	{
		attr.skip = true;
		return attr;
	}

	if (attr.read == TYPE_READ
		&& (isSgVarRefExp(node) || isSgPntrArrRefExp(node))	// Is this an array reference statement?
		&& parent
		&& (!isSgPntrArrRefExp(parent)|| (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)))	// Is this node representing the top-level dimension of an array or if this array expression is used to read from another array?
	{
		attrib a(TYPE_UNKNOWN, FALSE, NULL, 0);

		scourer_t s (getEnclosingNode<SgStatement>(node), node->unparseToString());
		s.traverse(doLoop->get_body(), a);

		if (s.assignOp != NULL)
		{
			// Replace `node' with s.assignOp
			SgNode* clonedNode = deepCopy(s.assignOp);
			clonedNode->set_parent(parent);

			if (isSgBinaryOp(parent) && isSgExpression(clonedNode))
			{
				SgBinaryOp* binOp = isSgBinaryOp(parent);
				SgExpression* exprNode = isSgExpression(clonedNode);

				if (binOp->get_lhs_operand() == node)
					binOp->set_lhs_operand(exprNode);
				else if (binOp->get_rhs_operand() == node)
					binOp->set_rhs_operand(exprNode);
			}
			else if (isSgExprListExp(parent))
			{
				SgExprListExp* exp = isSgExprListExp(parent);
				ROSE_ASSERT(exp->get_expressions().size() == 1);
				SgExpression* old = *(exp->get_expressions().begin());
				exp->replace_expression(old, (SgExpression*) clonedNode);
			}
			else
				std::cout << "[" << node->unparseToString() << "] Did not match!\n";
		}
		
		// attr.skip = true;
	}

	return attr;
}
