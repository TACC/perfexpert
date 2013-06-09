
#include <set>
#include <algorithm>
#include <rose.h>

#include "./common/record.h"
#include "splitter.h"
#include "stream_counter.h"

using namespace SageBuilder;
using namespace SageInterface;

#define	STREAM_THRESHOLD	5
#define	TOLERANCE		1

static bool stopCycle=false;
static SgNode* dest = NULL;
static SgBasicBlock* oldbb = NULL;
static SgBasicBlock* newbb = NULL;
static SgFortranDo* doLoop = NULL;
static std::set<std::string> known_streams;
static std::vector<statement_t> partial_stmts;

std::string get_stream_name(std::string stream_name)
{
	unsigned index = stream_name.find_first_of('[');
	std::string ret_string = stream_name.substr(0, index);
	
	// For fortran array notation
	index = ret_string.find_first_of('(');
	ret_string = ret_string.substr(0, index);
	return ret_string;
}

void splitter_t::atTraversalStart()
{
	stopCycle = false;
}

void splitter_t::atTraversalEnd()
{
	if (oldbb != NULL)
	{
		doLoop->set_body(oldbb);
		oldbb->set_parent(doLoop);

		if (newbb != NULL)
		{
			clonedLoop = deepCopy(doLoop);
			clonedLoop->set_body(newbb);
			newbb->set_parent(clonedLoop);

			((SgBasicBlock*) doLoop->get_parent())->append_statement(clonedLoop);
			clonedLoop->set_parent(doLoop->get_parent());

			if (!stopCycle)
			{
				dest = NULL;
				oldbb = NULL;
				newbb = NULL;
				doLoop = NULL;

				known_streams.clear();
				partial_stmts.clear();

				attrib attr(TYPE_UNKNOWN, FALSE, NULL, 0);
				splitter_t newsplitter;
				newsplitter.traverse(clonedLoop, attr);
			}
		}
	}

    dest = NULL;
    oldbb = NULL;
    newbb = NULL;
    doLoop = NULL;
    
}

attrib splitter_t::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	// If explicit instructions to skip this node, then just return
	if (attr.skip == TRUE)
		return attr;

	if (isSgAssignOp(node))
		dest = NULL;

	SgNode* parent = node->get_parent();

	// Decide whether read / write depending on the operand of AssignOp
	if (parent && isSgAssignOp(parent))
		attr.read = parent->getChildIndex(node) == 0 ? TYPE_WRITE : TYPE_READ;

#if 1
	if (getEnclosingNode<SgMultiplyOp>(node) == NULL && getEnclosingNode<SgDivideOp>(node) == NULL && dest && (isSgAddOp(parent) || isSgSubtractOp(parent)))
	{
		if (isSgPntrArrRefExp(dest))
		{
			attrib atemp(TYPE_UNKNOWN, FALSE, NULL, 0);
			stream_counter_t scounter;
			scounter.traverse(parent, atemp);

			if (dest)
				scounter.discovered_streams.erase(get_stream_name(dest->unparseToString()));

			if (scounter.discovered_streams.size() > STREAM_THRESHOLD)
			{
				if (!oldbb)
				{
					oldbb = new SgBasicBlock(node->get_file_info()); // getEnclosingNode<SgBasicBlock>(node);
					oldbb->set_endOfConstruct(node->get_file_info());
				}

				SgBinaryOp* binOp = isSgBinaryOp(parent);
				SgExpression* sibling = NULL;
				SgBinaryOp* gParent = isSgBinaryOp(parent->get_parent());
				if (gParent)
				{
					if (binOp->get_lhs_operand() == node)
						sibling = binOp->get_rhs_operand();
					else if (binOp->get_rhs_operand() == node)
						sibling = binOp->get_lhs_operand();

					bool lhs=false;
					statement_t partial_statement = { false, NULL, NULL };
					if (isSgAddOp(parent) || binOp->get_rhs_operand() == node)
					{
						// Replace parent with sibling
						if (gParent->get_lhs_operand() == parent)
						{
							lhs = true;
							gParent->set_lhs_operand(sibling);

							partial_statement.lhs = dest;
							partial_statement.rhs = node;
						}
						else if (gParent->get_rhs_operand() == parent)
						{
							lhs = false;
							gParent->set_rhs_operand(sibling);

							partial_statement.lhs = dest;
							partial_statement.rhs = node;
						}
					}
					else // if (isSgSubtractOp(parent))
					{
						SgMinusOp* minusOp = new SgMinusOp(node->get_file_info(), sibling, sibling->get_type());
						minusOp->set_operand(sibling);
						minusOp->set_endOfConstruct(node->get_file_info());

						if (gParent->get_lhs_operand() == parent)
						{
							lhs = true;
							gParent->set_lhs_operand(minusOp);

							partial_statement.neg = true;
							partial_statement.lhs = dest;
							partial_statement.rhs = node;
						}
						else if (gParent->get_rhs_operand() == parent)
						{
							lhs = false;
							gParent->set_rhs_operand(minusOp);

							partial_statement.neg = true;
							partial_statement.lhs = dest;
							partial_statement.rhs = node;
						}
					}

					if (partial_statement.lhs != NULL && partial_statement.rhs != NULL)
					{
						if (isSgAssignOp(gParent) && gParent->get_lhs_operand() == gParent->get_rhs_operand())
						{
							// Don't split this statement
							if (lhs)	gParent->set_lhs_operand((SgExpression*) parent);
							else		gParent->set_rhs_operand((SgExpression*) parent);

							stopCycle = true;
						}
						else
						{

							if (!newbb)
							{
								doLoop = getEnclosingNode<SgFortranDo>(node);
								newbb = new SgBasicBlock(node->get_file_info()); // getEnclosingNode<SgBasicBlock>(node);
							}

							SgAddOp* addOp = new SgAddOp(node->get_file_info(), (SgExpression*) partial_statement.lhs, (SgExpression*) partial_statement.rhs, ((SgExpression*) partial_statement.lhs)->get_type());
							addOp->set_endOfConstruct(node->get_file_info());

							SgAssignOp* tempAssignOp = new SgAssignOp(node->get_file_info(), (SgExpression*) partial_statement.lhs, addOp, ((SgExpression*) partial_statement.lhs)->get_type());
							tempAssignOp->set_endOfConstruct(node->get_file_info());

							SgExprStatement* exprStatement = new SgExprStatement(node->get_file_info(), tempAssignOp);
							exprStatement->set_endOfConstruct(node->get_file_info());

							newbb->append_statement(exprStatement);
							partial_stmts.push_back(partial_statement);

							// Save the old (modified) statement into the "old" basic block
							exprStatement = getEnclosingNode<SgExprStatement>(gParent);
							oldbb->append_statement(exprStatement);
						}
					}

					evaluateInheritedAttribute(parent, attr);
				}
			}
		}
	}
#endif

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

	// TODO: Save pointers to other parts of the FortranDo statement
	if (parent == doLoop && parent->getChildIndex(node) == 0)
	{
		assignOp = node;
		attr.skip = true;
		return attr;
	}

	if (parent == doLoop && parent->getChildIndex(node) == 1)
	{
		initOp = node;
		attr.skip = true;
		return attr;
	}

	if (parent == doLoop && parent->getChildIndex(node) == 2)
	{
		termOp = node;
		attr.skip = true;
		return attr;
	}

	if ((isSgPntrArrRefExp(node) || isSgVarRefExp(node))
		&& parent
		&& (!isSgPntrArrRefExp(parent)|| (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)))	// Is this node representing the top-level dimension of an array or if this array expression is used to read from another array?
	{
		if (attr.read == TYPE_WRITE)
		{
			dest = node;
		}

		std::string dest_string;
		if (dest)
			dest_string = get_stream_name(dest->unparseToString());

		if (known_streams.size() < STREAM_THRESHOLD)
		{
			std::string sname = get_stream_name(node->unparseToString());
			if (sname != dest_string)
				known_streams.insert(sname);
		}

		attr.skip = true;
	}

	return attr;
}
