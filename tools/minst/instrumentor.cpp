
#include <algorithm>
#include <rose.h>

#include "instrumentor.h"

using namespace SageBuilder;
using namespace SageInterface;

instrumentor_t::instrumentor_t(short _lang)
{
	lang=_lang;
}

void instrumentor_t::atTraversalStart()
{
	stream_list.clear();
}

std::vector<std::string>& instrumentor_t::getStreamList()
{
	return stream_list;
}

attrib instrumentor_t::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	// If explicit instructions to skip this node, then just return
	if (attr.skip == TRUE)
		return attr;

	SgNode* parent = node->get_parent();

	// Decide whether read / write depending on the operand of AssignOp
	if (parent && isSgAssignOp(parent))
		attr.read = parent->getChildIndex(node) == 0 ? TYPE_WRITE : TYPE_READ;

	if (parent && isSgCompoundAssignOp(parent))
		attr.read = parent->getChildIndex(node) == 0 ? TYPE_READ_AND_WRITE : TYPE_READ;

	// LHS operand of PntrArrRefExp is always skipped
	if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) == 0)
	{
		attr.skip=TRUE;
		return attr;
	}

	// RHS operand of PntrArrRefExp is always read and never written
	if (parent && isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)
		attr.read = TYPE_READ;

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

		SgBasicBlock* containingBB = getEnclosingNode<SgBasicBlock>(node);
		SgExprStatement* containingExprStmt = getEnclosingNode<SgExprStatement>(node);

		if (containingBB && containingExprStmt)
		{
			std::string stream_name = node->unparseToString();
			while (stream_name.at(stream_name.length()-1) == ']')
			{
				// Strip off the last [.*]
				unsigned index = stream_name.find_last_of('[');
				stream_name = stream_name.substr(0, index);
			}

			// For fortran array notation
			while (stream_name.at(stream_name.length()-1) == ')')
			{
				// Strip off the last [.*]
				unsigned index = stream_name.find_last_of('(');
				stream_name = stream_name.substr(0, index);
			}

			std::vector<std::string>::iterator iter = std::find(stream_list.begin(), stream_list.end(), stream_name);
			size_t idx = std::distance(stream_list.begin(), iter);
			if (idx == stream_list.size())
			{
				stream_list.push_back(stream_name);
				idx = stream_list.size()-1;
			}

			Sg_File_Info *fileInfo = Sg_File_Info::generateFileInfoForTransformationNode(
					((SgLocatedNode*) node)->get_file_info()->get_filenameString());

			int line_number=0;
			SgStatement *stmt = getEnclosingNode<SgStatement>(node);
			if (stmt)	line_number = stmt->get_file_info()->get_raw_line();

			SgExpression* param_addr=NULL;
			SgIntVal *param_line_number=NULL, *param_idx=NULL, *param_read_write=NULL;

			param_line_number = new SgIntVal(fileInfo, line_number);
			param_idx = new SgIntVal(fileInfo, idx);
			param_read_write = new SgIntVal(fileInfo, attr.read);

			std::vector<SgExpression*> expr_vector;
			// If not Fortran, cast the address to a void pointer
			param_addr = lang!=LANG_FORTRAN ? buildCastExp (buildAddressOfOp((SgExpression*) node), buildPointerType(buildVoidType())) : (SgExpression*) node;

			expr_vector.push_back(param_read_write);
			expr_vector.push_back(param_line_number);
			expr_vector.push_back(param_addr);
			expr_vector.push_back(param_idx);

			std::string indigo__record = lang!=LANG_FORTRAN ? "indigo__record_c" : "indigo__record_f";
			SgExprStatement* fCall = buildFunctionCallStmt(SgName(indigo__record), buildVoidType(), buildExprListExp(expr_vector), containingBB);
			insertStatementBefore(containingExprStmt, fCall);

			// Reset this attribute for any child expressions
			attr.skip = false;
		}
	}
	else

	return attr;
}
