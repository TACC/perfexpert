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
 * Authors: Leonardo Fialho, Ashay Rane and Ankit Goyal
 *
 * $HEADER$
 */

#include <cassert>
#include <map>
#include <vector>
#include <string>

#include "tools/macpo/analyze/include/analysis_defs.h"
#include "tools/macpo/common/reuse_distance.h"
#include "hwloc.h"
#include <iostream>
#include <signal.h>
#include "tools/macpo/analyze/include/cache_info.h""
//#include "mrt.h"

#include "tools/macpo/libset/set_cache_conflict_lib.h"

// TODO(goyalankit): remove hardcoded associativity
#define ASSOCIATIVITY 8

typedef std::vector<conflict_t> conflict_list_t;
typedef std::vector<conflict_prob_t> conflict_prob_list_t;

typedef std::vector<reuse_distance_manager *> reuse_distance_list_t;

global_data_t global_data;
cache_stats_t cache_stats;
cache_data_t l1_data;
size_t l1_cache_lines;
std::vector< conflict_list_t > *set_conflicts_per_core_for_l1_cache;
// list of per set reuse distance manager for each core
std::vector<reuse_distance_list_t *> *rd_trees_per_core;
static bool first = true;


// creates a bit mask with bits between a and b from the right as 1.
// example: b = 4, a = 2 => 0000001100
#define BIT_MASK(a, b) (size_t)(((size_t) -1 >> ((sizeof(size_t)*8) \
                - (b))) & ~((1U << (a)) - 1))



unsigned address_to_set(size_t address) {
    return ((address & BIT_MASK(6, 12)) >> 6);
}

template < class T, class U>
inline std::ostream& operator << (std::ostream& os, const std::map<T, U>& v) {
    os << "[";
    for (typename std::map<T, U>::const_iterator ii = v.begin();
            ii != v.end(); ++ii) {
        os << " " << ii->first;
    }
    os << " ]";
    return os;
}

void print_set_conflicts(std::vector<conflict_list_t> &conflicts,
        int num_sets, const name_list_t &stream_info) {
    int total_conflicts = 0;
    std::map< int, std::map<int, int>* > core_set_var_conf;
    std::map<int, std::map<int, std::map<std::string, int> > > core_set_var_name;
    for (int core_id = 0; core_id < conflicts.size(); core_id++) {
        core_set_var_conf[core_id] = new std::map<int, int>();
        for (int n = 0; n < conflicts[core_id].size(); n++) {
            conflict_t ct = conflicts[core_id][n];
            total_conflicts++;
            (*core_set_var_conf[core_id])[ct.set_number]++;
            core_set_var_name[core_id][ct.set_number][stream_info[ct.var_idx]] = 1;

        }
    }

    std::cout << "Total Conflicts: " << total_conflicts << std::endl;

    for (int i = 0; i < core_set_var_conf.size(); ++i) {
        if (core_set_var_conf[i]->size() != 0) {
            std::cout << "Conflicts for core number: " << i << std::endl;
            std::cout << "set_num" << " " << "num_conflicts" << std::endl;
        }

        for (std::map<int, int>::iterator it = core_set_var_conf[i]->begin();
                it != core_set_var_conf[i]->end(); ++it) {
            std::cout << it->first << " " << it->second <<  " "
                << core_set_var_name[i][it->first] << std::endl;
        }
    }
}

/*
   void printCacheInformation(cache_data_t cache_data, int number_of_sets) {
   std::cout << "----- Cache Information ----------" << std::endl;
   std::cout << "Cache size:            " << cache_data.size << std::endl;
   std::cout << "Cache line size:       " << cache_data.line_size << std::endl;
   std::cout << "Number of cache lines: " << (cache_data.size / cache_data.line_size) << std::endl;
   std::cout << "Cache associativity:   " << ASSOCIATIVITY << std::endl;
   std::cout << "Number of sets:        " << number_of_sets <<  std::endl;
   }
   */

void printStats() {
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Total Address Accesses " <<  cache_stats.addr_accesses << std::endl;
    std::cout << "Total Address Cold Misses " <<  cache_stats.addr_cold_misses << std::endl;
    std::cout << "Total Address Conflict Misses   " <<  cache_stats.addr_conflict_misses << std::endl;
    std::cout << "Total Address Capacity Misses   " <<  cache_stats.addr_capacity_misses << std::endl;
    std::cout << "Total Address Hits     " <<  cache_stats.addr_hits << std::endl;
    std::cout << "-----------------------------" << std::endl;
}


void sigint_handler(int s){
    printStats();
    exit(1);
}

extern "C" {
    void indigo_set_cache_init() {
        int code = 0;
        // load number of cores
        num_cores = sysconf(_SC_NPROCESSORS_CONF);

        // load cache information
        memset(&global_data, 0, sizeof(global_data));

        if ((code = load_cache_info(global_data)) < 0) {
            std::cerr << "Failed to load cache information, terminating." <<
                std::endl;
            return;
        }

        struct sigaction sigIntHandler;
        sigIntHandler.sa_handler = sigint_handler;
        sigemptyset(&sigIntHandler.sa_mask);
        sigIntHandler.sa_flags = 0;
        sigaction(SIGINT, &sigIntHandler, NULL);
    }

    void indigo__end() {
        printStats();
    }

};

extern "C" {

    void indigo__process_f_(int *read_write, int *line_number, void* addr,
                    int *var_idx, int* type_size) {
        indigo__process_c(*read_write, *line_number,  addr,
            *var_idx, *type_size);
    }

    void indigo__process_c(int read_write, int line_number, void* addr,
            int var_idx, int type_size) {
        uint16_t core_id = 1;//getCoreID();
        if (first) {
            l1_data = global_data.l1_data;
            l1_cache_lines = l1_data.size / l1_data.line_size;
            l1_data.associativity = ASSOCIATIVITY;
            const int num_sets = l1_data.size/
                (l1_data.line_size  * l1_data.associativity);
            set_conflicts_per_core_for_l1_cache = new std::vector< conflict_list_t >(num_cores);
            rd_trees_per_core = new std::vector<reuse_distance_list_t *>(num_cores);

            for (int k = 0; k < num_cores; k++) {
                (*rd_trees_per_core)[k] = new reuse_distance_list_t(num_sets);
                for (int l = 0; l < num_sets; l++) {
                    (*(*rd_trees_per_core)[k])[l] = new reuse_distance_manager();
                }
            }
            first = false;
        }
        // printing cache information 
        //printCacheInformation(l1_data, num_sets);


        // cache level 1 stats
        // std::cout << "Size: " << l1_data.size << " line_size: " << l1_data.line_size
        //    << " count: " << l1_data.count << " Number of lines: " << l1_cache_lines << std::endl;

        assert(l1_data.size != 0 && l1_data.line_size != 0
                && l1_data.associativity !=0);

        size_t cache_line = ADDR_TO_CACHE_LINE((size_t) addr);
        size_t set_id = address_to_set((size_t) addr);
        reuse_distance_list_t &set_rd_list = (*(*rd_trees_per_core)[core_id]);
        conflict_list_t &set_conflicts_for_l1_cache =
            (*set_conflicts_per_core_for_l1_cache)[core_id];

        cache_stats.addr_accesses++;
        // if address is already seen => not a cold miss
        long reuse_distance = set_rd_list[set_id]->get_distance(cache_line);

        if (reuse_distance != -1) {
            // L1 conflicts
            if (reuse_distance > l1_data.associativity && reuse_distance < l1_cache_lines) {
                conflict_t conflict(set_id, var_idx);
                set_conflicts_for_l1_cache.push_back(conflict);
                cache_stats.addr_conflict_misses++;
            } else if (reuse_distance >= l1_cache_lines){
                cache_stats.addr_capacity_misses++;
            } else{
                cache_stats.addr_hits++;
            }
        } else {
            cache_stats.addr_cold_misses++;
        }
        set_rd_list[set_id]->insert(cache_line);
    }
}
