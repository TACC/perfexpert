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

#include <VariableRenaming.h>

#include "generic_defs.h"

class aligncheck_t : public AstTopDownProcessing<attrib> {

    enum { LOOP_TEST=0, LOOP_INCR, BASE_ALIGNMENT };

    public:
        aligncheck_t(VariableRenaming* _var_renaming);

        bool vectorizable(SgStatement* stmt);

        virtual void atTraversalEnd();
        virtual void atTraversalStart();

        virtual attrib evaluateInheritedAttribute(SgNode* node, attrib attr);

        const inst_list_t::iterator inst_begin();
        const inst_list_t::iterator inst_end();

    private:
        void locate_and_place_instrumentation(SgNode* node,
                SgExpression* instrument_test, SgExpression* instrument_incr);
        void insert_instrumentation(SgNode* node, SgExpression* expr,
                short type, bool before);

        VariableRenaming* var_renaming;
        inst_list_t inst_info_list;
};

#endif	/* ALIGNCHEK_H_ */
