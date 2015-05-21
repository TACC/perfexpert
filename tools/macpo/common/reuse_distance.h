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

#ifndef TOOLS_MACPO_COMMON_REUSE_DISTANCE_H
#define TOOLS_MACPO_COMMON_REUSE_DISTANCE_H

#include <iostream>
#include <vector>
//#include <unordered_map>
#include <stdint.h>

#ifdef __GNUC__
#include <tr1/unordered_map>
#else
#include <unordered_map>
#endif

/*
 * This class manages reuse distance per set.
 *
 * */
class reuse_distance_manager {
    typedef std::vector<size_t> cache_set_t;
    typedef std::tr1::unordered_map <size_t, int> umap_t;

    public:
    reuse_distance_manager() : access_count(0) {
    }

    cache_set_t::iterator find_in_cache(size_t cache_line) {
        cache_set_t::iterator it = std::find(cache_set.begin(),
                cache_set.end(), cache_line);
        return it;
    }


    void adjust_other_counters(size_t cache_line) {
        int previous = cache_line_count_map[cache_line];

        for (umap_t::iterator itr = cache_line_count_map.begin(),
                ie = cache_line_count_map.end(); itr!=ie;) {
            if (itr->second < previous) {
                itr->second++;
                if ((access_count - itr->second) >= 512) {
                    umap_t::iterator temp = itr;
                    ++itr;
                    cache_line_count_map.erase(temp);
                } else {
                    ++itr;
                }
            } else {
                ++itr;
            }
        }
    }

    // for debugging
    void printMap() {
        std::cout << "------------" << std::endl;
        std::cout << "Access Count: " << access_count << std::endl;
        for (std::tr1::unordered_map <size_t, int>::const_iterator itr = cache_line_count_map.begin(), ie = cache_line_count_map.end(); itr!=ie; ++itr) {
            std::cout << itr->first << " " << itr->second << std::endl;
        }
        std::cout << "------------" << std::endl;
    }

    void insert(size_t cache_line) {
        // only increment the count if not in cache
        if (get_distance(cache_line) != 0) {
            adjust_other_counters(cache_line);
            access_count++;
        }

        already_accessed[cache_line] = 1;
        cache_line_count_map[cache_line] = access_count;
        //cache_set_insert(cache_line);
    }

    size_t get_distance(size_t cache_line) {
        if (cache_line_count_map.find(cache_line) != cache_line_count_map.end())
            return access_count - cache_line_count_map[cache_line];
        else if (already_accessed[cache_line] == 1)
            return 513; // any number greater than 512 will do.
        else
            return -1;
    }

    void cache_set_insert(size_t cache_line) {
        cache_set_t::iterator it =  find_in_cache(cache_line);
        // if we found it then erase so that we can put it in front
        if (it != cache_set.end()) {
            cache_set.erase(it);
        }

        cache_set.push_back(cache_line);
        if (cache_set.size() > 8) {
            cache_set.erase(cache_set.begin());
        }
    }

    private:
    umap_t cache_line_count_map;
    umap_t already_accessed;

    cache_set_t cache_set;
    uint64_t access_count;
};

#endif // TOOLS_MACPO_COMMON_REUSE_DISTANCE_H
