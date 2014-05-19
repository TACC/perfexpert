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

    const val_t get(const bin_t& bin) {
        if (_hist.find(bin) != _hist.end()) {
            return _hist[bin];
        }

        // The key was not found, return the default value.
        return 0;
    }

    void set(const bin_t& bin, const val_t& val) {
        _hist[bin] = val;
    }

    const val_t increment(const bin_t& bin, const val_t& val) {
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

template <class bin_t, class val_t>
class multigram_t {
 private:
    typedef histogram_t<bin_t, val_t> hist_t;
    typedef std::map<int64_t, hist_t*> l3_map_t;
    typedef std::map<void*, l3_map_t*> l2_map_t;
    typedef std::map<int64_t, l2_map_t*> l1_map_t;

    l1_map_t map_indexes;

    hist_t* map_into_histogram(int64_t thread_id, void* function_address,
            int64_t line_number) {
        typedef typename l1_map_t::iterator l1_iterator;
        typedef typename l2_map_t::iterator l2_iterator;
        typedef typename l3_map_t::iterator l3_iterator;

        l1_iterator it = map_indexes.find(thread_id);
        if (it != map_indexes.end()) {
            l2_map_t* l2_map = it->second;

            // If it's defined, it has to be non-NULL.
            assert(l2_map != NULL);

            l2_iterator it = l2_map->find(function_address);
            if (it != l2_map->end()) {
                l3_map_t* l3_map = it->second;

                // If it's defined, it has to be non-NULL.
                assert(l3_map != NULL);

                l3_iterator it = l3_map->find(line_number);
                if (it != l3_map->end()) {
                    assert(it->second);
                    return it->second;
                } else {
                    hist_t* hist = new hist_t();

                    (*l3_map)[line_number] = hist;
                    return hist;
                }
            } else {
                hist_t* hist = new hist_t();

                l3_map_t* l3_map = new l3_map_t();
                (*l3_map)[line_number] = hist;

                (*l2_map)[function_address] = l3_map;
                return hist;
            }
        } else {
            hist_t* hist = new hist_t();

            l3_map_t* l3_map = new l3_map_t();
            (*l3_map)[line_number] = hist;

            l2_map_t* l2_map = new l2_map_t();
            (*l2_map)[function_address] = l3_map;

            map_indexes[thread_id] = l2_map;
            return hist;
        }
    }

 public:
    typedef struct {
        int64_t thread_id;
        int64_t line_number;
        void* function_address;
    } hist_id_t;

    typedef std::pair<hist_id_t, hist_t*> pair_t;
    typedef std::vector<pair_t> pair_list_t;

    const val_t get(int64_t thread_id, void* function_address,
            int64_t line_number, const bin_t& bin) {
        // Use the first two arguments to get the histogram
        // and index into the histogram using the third argument.

        hist_t* hist = map_into_histogram(thread_id, function_address,
            line_number);
        assert(hist != NULL);

        return hist->get(bin);
    }

    void set(int64_t thread_id, void* function_address, int64_t line_number,
            const bin_t& bin, const val_t& val) {
        hist_t* hist = map_into_histogram(thread_id, function_address,
            line_number);
        assert(hist != NULL);

        hist->set(bin, val);
    }

    const val_t increment(int64_t thread_id, void* function_address,
            int64_t line_number, const bin_t& bin, const val_t& val) {
        hist_t* hist = map_into_histogram(thread_id, function_address,
            line_number);
        assert(hist != NULL);

        return hist->increment(bin, val);
    }

    pair_list_t get_all_histograms() {
        typedef typename l1_map_t::iterator l1_iterator;
        typedef typename l2_map_t::iterator l2_iterator;
        typedef typename l3_map_t::iterator l3_iterator;

        pair_list_t pair_list;
        for (l1_iterator it = map_indexes.begin(); it != map_indexes.end();
                it++) {
            int64_t thread_id = it->first;
            l2_map_t* l2_map = it->second;
            for (l2_iterator it = l2_map->begin(); it != l2_map->end(); it++) {
                void* function_address = it->first;
                l3_map_t* l3_map = it->second;
                for (l3_iterator it = l3_map->begin(); it != l3_map->end();
                        it++) {
                    int64_t line_number = it->first;
                    hist_t* hist = it->second;
                    assert(hist);

                    hist_id_t hist_id;
                    hist_id.thread_id = thread_id;
                    hist_id.line_number = line_number;
                    hist_id.function_address = function_address;

                    pair_t pair(hist_id, hist);
                    pair_list.push_back(pair);
                }
            }
        }

        return pair_list;
    }
};

#endif  // TOOLS_MACPO_COMMON_HISTOGRAM_H_
