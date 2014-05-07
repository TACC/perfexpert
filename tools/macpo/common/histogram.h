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

#ifndef TOOLS_MACPO_COMMON_HISTOGRAM_H_
#define TOOLS_MACPO_COMMON_HISTOGRAM_H_

#include <algorithm>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

#define ADDR_TO_CACHE_LINE(x)   (x >> 6)

template <class bin_t, class val_t>
class histogram_t {
 public:
    typedef std::pair<bin_t, val_t> pair_t;
    typedef std::vector<pair_t> pair_list_t;

    size_t size() {
        return _hist.size();
    }

    const val_t& increment(const bin_t& bin, const val_t& val) {
        _hist[bin] += val;
        return _hist[bin];
    }

    const pair_list_t sort() {
        // Sort and return all of them.
        return sort(-1);
    }

    const pair_list_t sort(const int& k) {
        // Sort and return the top k pairs.
        pair_list_t pair_list;

        std::copy(_hist.begin(), _hist.end(),
                std::back_inserter<pair_list_t>(pair_list));
        std::sort(pair_list.begin(), pair_list.end(), _compare);

        if (k > 0 && pair_list.size() > k) {
            pair_list.resize(k);
        }

        return pair_list;
    }

 private:
    std::map<bin_t, val_t> _hist;

    static bool _compare(const pair_t& a, const pair_t& b) {
        return a.second > b.second;
    }
};

#endif  // TOOLS_MACPO_COMMON_HISTOGRAM_H_
