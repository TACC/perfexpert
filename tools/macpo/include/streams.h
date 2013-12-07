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

#ifndef	STREAMS_H_
#define	STREAMS_H_

#include "generic_defs.h"
#include "inst_defs.h"

class streams_t : public AstTopDownProcessing<attrib> {
    public:
        reference_list_t& get_reference_list();

        virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
        virtual void atTraversalStart();
        virtual void atTraversalEnd();

    private:
        reference_list_t reference_list;
};

#endif	/* STREAMS_H_ */
