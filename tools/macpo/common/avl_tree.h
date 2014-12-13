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

#if 0
    Adapted from: www.zentut.com/c-tutorial/c-avl-tree/
#endif

#ifndef TOOLS_MACPO_COMMON_AVL_TREE_H_
#define TOOLS_MACPO_COMMON_AVL_TREE_H_

#include <stdint.h>

#include <algorithm>
#include <vector>

#include "macpo_record.h"

#define ADDR_TO_CACHE_LINE(x)   (x >> 6)

class avl_tree {
 public:
    void insert(const mem_info_t* mem_info) {
        size_t cache_line = ADDR_TO_CACHE_LINE(mem_info->address);
        addr_vector_t::iterator it = std::find(addr_vector.begin(),
                addr_vector.end(), cache_line);
        if (it != addr_vector.end()) {
            addr_vector.erase(it);
        }

        addr_vector.push_back(cache_line);
    }

    size_t get_distance(size_t cache_line) {
        addr_vector_t::iterator it = std::find(addr_vector.begin(),
                addr_vector.end(), cache_line);
        if (it == addr_vector.end()) {
            return -1;
        }

        return addr_vector.end() - it - 1;
    }

    void make_infinite_distance(size_t cache_line) {
        addr_vector_t::iterator it = std::find(addr_vector.begin(),
                addr_vector.end(), cache_line);
        if (it != addr_vector.end()) {
            addr_vector.erase(it);
        }
    }

    void destroy() {
        addr_vector.clear();
    }

 private:
    typedef std::vector<size_t> addr_vector_t;
    addr_vector_t addr_vector;
};

#endif  // TOOLS_MACPO_COMMON_AVL_TREE_H_
