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
#include "inst_defs.h"

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

    void atTraversalEnd();
    void atTraversalStart();

    void process_node(SgNode* node);
    void process_loop(SgForStatement* for_stmt, loop_info_t& loop_info_t,
            expr_map_t& loop_map);

    bool contains_non_linear_reference(const reference_list_t& reference_list);

    private:
    VariableRenaming* var_renaming;
    inst_list_t inst_info_list;
};

#endif	/* ALIGNCHEK_H_ */
