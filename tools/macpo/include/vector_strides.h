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

#ifndef	VECTOR_STRIDES_H_
#define	VECTOR_STRIDES_H_

#include <VariableRenaming.h>

#include "analysis_profile.h"
#include "generic_defs.h"
#include "inst_defs.h"

class vector_strides_t {
    typedef VariableRenaming::NumNodeRenameEntry::iterator entry_iterator;
    typedef std::map<std::string, VariableRenaming::NumNodeRenameEntry>
        def_map_t;

    public:
        typedef std::map<SgExpression*, loop_info_t*> expr_map_t;

        vector_strides_t(VariableRenaming*& _var_renaming);

        name_list_t& get_stream_list();

        void atTraversalEnd();
        void atTraversalStart();

        void process_node(SgNode* node);
        void process_loop(SgScopeStatement* scope_stmt, loop_info_t& loop_info_t,
                expr_map_t& loop_map, name_list_t& stream_list);

        bool contains_non_linear_reference(const reference_list_t&
                reference_list);

        const analysis_profile_t& get_analysis_profile();

        const statement_list_t::iterator stmt_begin();
        const statement_list_t::iterator stmt_end();

    private:
        void instrument_vector_strides(Sg_File_Info* fileInfo,
                SgScopeStatement* scope_stmt);

        VariableRenaming* var_renaming;
        statement_list_t statement_list;
        name_list_t var_name_list;
        analysis_profile_t analysis_profile;
};

#endif	/* VECTOR_STRIDES_H_ */
