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

#ifndef	INSTRUMENTOR_H_
#define	INSTRUMENTOR_H_

#include "generic_defs.h"

class instrumentor_t : public AstTopDownProcessing<attrib> {
    public:
        std::vector<reference_info_t>& get_reference_list();

        virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);
        virtual void atTraversalStart();
        virtual void atTraversalEnd();

        const inst_list_t::iterator inst_begin();
        const inst_list_t::iterator inst_end();

    private:
        inst_list_t inst_info_list;
        std::vector<reference_info_t> reference_list;
};

#endif	/* INSTRUMENTOR_H_ */
