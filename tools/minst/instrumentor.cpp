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

#include <algorithm>
#include <rose.h>

#include "macpo_record.h"
#include "instrumentor.h"

using namespace SageBuilder;
using namespace SageInterface;

void instrumentor_t::atTraversalStart()
{
	stream_list.clear();
	inst_info_list.clear();
}

void instrumentor_t::atTraversalEnd()
{
	std::string indigo__record = SageInterface::is_Fortran_language() ? "indigo__record_f" : "indigo__record_c";
	for (std::vector<inst_info_t>::iterator it=inst_info_list.begin(); it!=inst_info_list.end(); it++)
	{
		inst_info_t& inst_info = *it;

		SgNode* parent = inst_info.bb->get_parent();
		if (isSgIfStmt(parent))
		{
			SgIfStmt* if_node = static_cast<SgIfStmt*>(parent);
			if_node->set_use_then_keyword(true);
			if_node->set_has_end_statement(true);
		}

		SgExprStatement* fCall = buildFunctionCallStmt(
				SgName(indigo__record),
				buildVoidType(),
				buildExprListExp(inst_info.params),
				inst_info.bb
		);

		insertStatementBefore(inst_info.exprStmt, fCall);
	}
}

std::vector<std::string>& instrumentor_t::get_stream_list()
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

	if (parent && (isSgPlusPlusOp(parent) || isSgMinusMinusOp(parent)) && attr.read == TYPE_UNKNOWN)
		attr.read = TYPE_READ_AND_WRITE;

	if (parent && (isSgUnaryOp(parent) || isSgBinaryOp(parent)) && attr.read == TYPE_UNKNOWN)
		attr.read = TYPE_READ;

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

			// If not Fortran, cast the address to a void pointer
			SgExpression *param_addr = SageInterface::is_Fortran_language() ? (SgExpression*) node : buildCastExp (buildAddressOfOp((SgExpression*) node), buildPointerType(buildVoidType()));

			SgIntVal* param_line_number = new SgIntVal(fileInfo, line_number);
			SgIntVal* param_idx = new SgIntVal(fileInfo, idx);
			SgIntVal* param_read_write = new SgIntVal(fileInfo, attr.read);

			inst_info_t inst_info;
			inst_info.bb = containingBB;
			inst_info.exprStmt = containingExprStmt;
			inst_info.params.push_back(param_read_write);
			inst_info.params.push_back(param_line_number);
			inst_info.params.push_back(param_addr);
			inst_info.params.push_back(param_idx);
			inst_info_list.push_back(inst_info);

			// Reset this attribute for any child expressions
			attr.skip = false;
		}
	}

	return attr;
}
