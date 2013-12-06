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

/* System standard headers */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

/* Pre-requirements headers */
#include <hwloc.h>

static void print_children(hwloc_topology_t topo, hwloc_obj_t obj, int parent) {
    int i = 0, my_id = 0;
    static int id = 0;

    switch (obj->type) {
        case HWLOC_OBJ_NODE:
            my_id = ++id;
            printf("INSERT INTO memsniffer (id, parent, name, size)\n"
                "  VALUES (%d, %d, '%s', %"PRIu64");\n", id, parent,
                hwloc_obj_type_string(obj->type), obj->memory.total_memory);
            break;

        case HWLOC_OBJ_CACHE:
            my_id = ++id;
            printf("INSERT INTO memsniffer (id, parent, name, size, depth, "
                "linesize, associativity)\n  VALUES (%d, %d, '%s', %"PRIu64", "
                "%d, %d, %d);\n", id, parent, hwloc_obj_type_string(obj->type),
                obj->attr->cache.size, obj->attr->cache.depth,
                obj->attr->cache.linesize, obj->attr->cache.associativity);
            break;
    }

    for (i = 0; i < obj->arity; i++) {
        print_children(topo, obj->children[i], my_id);
    }
}

int main( int argc, char * argv[] ) {
    hwloc_topology_t topology;

    printf("--\n\
-- Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.\n\
--\n\
-- $COPYRIGHT$\n\
--\n\
-- Additional copyrights may follow\n\
--\n\
-- This file is part of PerfExpert.\n\
--\n\
-- PerfExpert is free software: you can redistribute it and/or modify it under\n\
-- the terms of the The University of Texas at Austin Research License\n\
--\n\
-- PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY\n\
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR\n\
-- A PARTICULAR PURPOSE.\n\
--\n\
-- Authors: Leonardo Fialho and Ashay Rane\n\
--\n\
-- $HEADER$\n\
--\n\
\n\
-- Generated using memsniffer\n\
-- version 1.0\n\
\n\
--\n\
-- Create tables if not exist\n\
--\n\
CREATE TABLE IF NOT EXISTS memsniffer (\n\
    id            INTEGER NOT NULL,\n\
    parent        INTEGER NOT NULL,\n\
    name          VARCHAR NOT NULL,\n\
    size          INTEGER NOT NULL,\n\
    depth         INTEGER,\n\
    linesize      INTEGER,\n\
    associativity INTEGER\n\
);\n\n");

    hwloc_topology_init(&topology);
    hwloc_topology_load(topology);
    print_children(topology, hwloc_get_root_obj(topology), 0);
    hwloc_topology_destroy(topology);

    return 0;
}
