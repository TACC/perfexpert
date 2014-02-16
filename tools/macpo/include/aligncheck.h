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

#include "analysis_profile.h"
#include "generic_defs.h"
#include "inst_defs.h"
#include "loop_traversal.h"

class aligncheck_t {
    typedef VariableRenaming::NumNodeRenameEntry::iterator entry_iterator;
    typedef std::map<std::string, VariableRenaming::NumNodeRenameEntry>
        def_map_t;

    public:
        typedef std::map<SgExpression*, loop_info_t*> expr_map_t;
        typedef std::vector<SgPntrArrRefExp*> pntr_list_t;
        typedef std::vector<SgExpression*> expr_list_t;
        typedef std::vector<SgNode*> node_list_t;
        typedef std::map<std::string, node_list_t> sstore_map_t;

        aligncheck_t(VariableRenaming*& _var_renaming);
        ~aligncheck_t();

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
        void instrument_loop_trip_count(Sg_File_Info* file_info,
                loop_info_t& loop_info);
        void instrument_streaming_stores(Sg_File_Info* file_info,
                loop_info_t& loop_info);
        SgExpression* instrument_alignment_checks(Sg_File_Info* file_info,
                SgScopeStatement* outer_scope_stmt, loop_info_t& loop_info,
                name_list_t& stream_list, expr_map_t& loop_map);
        void instrument_branches(Sg_File_Info* fileInfo,
                SgScopeStatement* scope_stmt, SgExpression* idxv_expr);
        void instrument_vector_strides(Sg_File_Info* fileInfo,
                SgScopeStatement* scope_stmt);

        VariableRenaming* var_renaming;
        statement_list_t statement_list;
        name_list_t var_name_list;
        analysis_profile_t analysis_profile;
        loop_traversal_t* loop_traversal;
};

#endif	/* ALIGNCHEK_H_ */
