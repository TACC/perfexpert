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

#ifndef	ALIGNCHEK_H_
#define	ALIGNCHEK_H_

#include "generic_defs.h"

class aligncheck_t : public AstTopDownProcessing<attrib> {
    public:
        bool vectorizable(SgStatement* stmt);

        virtual void atTraversalEnd();
        virtual void atTraversalStart();

        virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
};

#endif	/* ALIGNCHEK_H_ */
