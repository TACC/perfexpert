
#include <rose.h>

#include "instrumentor.h"

using namespace SageBuilder;
using namespace SageInterface;

attrib instrumentor_t::evaluateInheritedAttribute(SgNode* node, attrib attr)
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

	if (attr.read != TYPE_UNKNOWN	// Are we sure whether this is a read or a write operation?
		&& isSgPntrArrRefExp(node)	// Is this an array reference statement?
		&& parent
		&& (!isSgPntrArrRefExp(parent)|| (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)))	// Is this node representing the top-level dimension of an array or if this array expression is used to read from another array?
	{
		if (!isSgLocatedNode(node))	// If no debug info present, can't do anything
		{
			std::cerr << "Debug info not present, cannot proceed!" << std::endl;
			return attr;
		}

		Sg_File_Info* fileInfo = ((SgLocatedNode*) node)->get_file_info();

		int line_number = fileInfo->get_line();
		const char* szfilename = fileInfo->get_filenameString().c_str();

		SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(node);
		SgExprStatement* containingExprStmt = getEnclosingNode<SgExprStatement>(node);

		if (containingBB && containingExprStmt)
		{
			std::string stream_name = node->unparseToString();
			if (stream_name.at(stream_name.length()-1) == ']')
			{
				// Strip off the last [.*]
				unsigned index = stream_name.find_last_of('[');
				stream_name = stream_name.substr(0, index);
			}


			// For fortran array notation
			if (stream_name.at(stream_name.length()-1) == ')')
			{
				// Strip off the last [.*]
				unsigned index = stream_name.find_last_of('(');
				stream_name = stream_name.substr(0, index);
			}

			// TODO: Count streams only when asked
			// std::cout << "Name: " << stream_name << std::endl;

/*
			// FIXME: Cast operations don't seem to work with Fortran code
			SgExpression* castExpr = buildCastExp (buildAddressOfOp((SgExpression*) node), buildPointerType(buildVoidType()));
			std::vector<SgExpression*> expr_vector;
			expr_vector.push_back(castExpr);

			SgExprStatement* fCall = buildFunctionCallStmt(attr.fName, buildVoidType(), buildExprListExp(expr_vector), containingBB);
			insertStatementBefore(containingExprStmt, fCall);
*/

			// Reset this attribute for any child expressions
			attr.skip = false;
		}
	}

	return attr;
}
