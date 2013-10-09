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

#ifndef	SCOURER_H_
#define	SCOURER_H_

#include "generic_defs.h"

class scourer_t : public AstTopDownProcessing<attrib>
{
	bool globalSkip;

	public:
	std::string var_name;
	SgNode *currNode, *assignOp;

	scourer_t(SgNode* _currNode, std::string _var_name)
	{
		assignOp=NULL, globalSkip=false, currNode=_currNode, var_name=_var_name;
	}

	virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* SCOURER_H_ */
