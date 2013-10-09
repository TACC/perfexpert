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

#ifndef	SPLITTER_H_
#define	SPLITTER_H_

#include "generic_defs.h"

typedef struct
{
	bool neg;
	SgNode* lhs;
	SgNode* rhs;
} statement_t;

class splitter_t : public AstTopDownProcessing<attrib>
{
	public:
	SgFortranDo* clonedLoop;
	SgNode *assignOp, *initOp, *termOp;
	std::vector<statement_t> partial_statements;

	splitter_t()
	{
		initOp = NULL;
		termOp = NULL;
		assignOp = NULL;
		clonedLoop = NULL;
	}

	virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
	virtual void atTraversalStart();
	virtual void atTraversalEnd();
};

#endif	/* SPLITTER_H_ */
