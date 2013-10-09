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

#include "macpo_record.h"
#include "scourer.h"

using namespace SageBuilder;
using namespace SageInterface;

attrib scourer_t::evaluateInheritedAttribute(SgNode* node, attrib attr)
{
	SgNode* parent = node->get_parent();

	if (currNode == node)
		globalSkip = true;

	// If explicit instructions to skip this node, then just return
	if (globalSkip || attr.skip == TRUE)
		return attr;

	if (assignOp != NULL)
	{
		if (parent == assignOp && parent->getChildIndex(node) == 1)
		{
			assignOp = node;
			attr.skip = true;
			globalSkip = true;
		}
	}

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

	if (attr.read == TYPE_WRITE
		&& node->unparseToString() == var_name
		&& (isSgVarRefExp(node) || isSgPntrArrRefExp(node))	// Is this an array reference statement?
		&& parent
		&& (!isSgPntrArrRefExp(parent)|| (isSgPntrArrRefExp(parent) && parent->getChildIndex(node) > 0)))	// Is this node representing the top-level dimension of an array or if this array expression is used to read from another array?
	{
		assignOp = parent;
		attr.skip = true;
	}

	return attr;
}
