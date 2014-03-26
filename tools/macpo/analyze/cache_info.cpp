
#include <hwloc.h>
#include <iostream>

#include "cache_info.h"

int update_cache_fields(cache_data_t& cache_data, size_t cache_level,
        size_t cache_size, size_t line_size, size_t associativity) {
    if (cache_data.size != 0) {
        if (cache_data.size == cache_size && cache_data.line_size == line_size
                && cache_data.associativity == associativity) {
            // Found a matching cache, increment count and return.
            cache_data.count += 1;
            return 0;
        } else {
            // Found some cache that did not match other caches.
            return -1;
        }
    } else {
        // Initialize this cache entry.
        cache_data.count = 0;
        cache_data.size = cache_size;
        cache_data.line_size = line_size;
        cache_data.associativity = associativity;

        return 0;
    }
}

int insert_cache_type(global_data_t& global_data, size_t cache_level,
        size_t cache_size, size_t line_size, size_t associativity) {
    int i;

    cache_data_t& l1_data = global_data.l1_data;
    cache_data_t& l2_data = global_data.l2_data;
    cache_data_t& l3_data = global_data.l3_data;

    // There aren't many entries relating to cache in global_data.
    // So we search through them linearly.
    switch(cache_level) {
        case 1:
            return update_cache_fields(l1_data, cache_level, cache_size,
                    line_size, associativity);

        case 2:
            return update_cache_fields(l2_data, cache_level, cache_size,
                    line_size, associativity);

        case 3:
            return update_cache_fields(l3_data, cache_level, cache_size,
                    line_size, associativity);
    }

    return -1;
}

int load_cache_info(global_data_t& global_data) {
    int code = 0;
    hwloc_topology_t topology;

    // Load the CPU topology.
    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);

    // Iterate over the CPU toplogy in array-form.
    int max_depth = hwloc_topology_get_depth(topology);
    for (int i=0; i<max_depth; i++) {
        for (int j=0; j<hwloc_get_nbobjs_by_depth(topology, i); j++) {
            hwloc_obj_t obj = hwloc_get_obj_by_depth(topology, i, j);
            if (obj->type == HWLOC_OBJ_CACHE) {
                hwloc_obj_attr_u::hwloc_cache_attr_s& cache = obj->attr->cache;
                if ((code = insert_cache_type(global_data, cache.depth, cache.size,
                        cache.linesize, cache.associativity)) < 0)
                    return code;
            }
        }
    }

    // Free up memory.
    hwloc_topology_destroy(topology);

    return 0;
}
