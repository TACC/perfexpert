--
-- Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
--
-- $COPYRIGHT$
--
-- Additional copyrights may follow
--
-- This file is part of PerfExpert.
--
-- PerfExpert is free software: you can redistribute it and/or modify it under
-- the terms of the The University of Texas at Austin Research License
--
-- PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
-- A PARTICULAR PURPOSE.
--
-- Authors: Leonardo Fialho and Ashay Rane
--
-- $HEADER$
--

-- Generated using memsniffer
-- version 1.0

--
-- Create tables if not exist
--
CREATE TABLE IF NOT EXISTS memsniffer (
    id            INTEGER NOT NULL,
    parent        INTEGER NOT NULL,
    name          VARCHAR NOT NULL,
    size          INTEGER NOT NULL,
    depth         INTEGER,
    linesize      INTEGER,
    associativity INTEGER
);

INSERT INTO memsniffer (id, parent, name, size)
  VALUES (1, 0, 'NUMANode', 34312073216);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (2, 0, 'Cache', 20971520, 3, 64, 20);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (3, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (4, 3, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (5, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (6, 5, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (7, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (8, 7, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (9, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (10, 9, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (11, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (12, 11, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (13, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (14, 13, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (15, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (16, 15, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (17, 2, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (18, 17, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size)
  VALUES (19, 0, 'NUMANode', 34359738368);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (20, 0, 'Cache', 20971520, 3, 64, 20);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (21, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (22, 21, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (23, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (24, 23, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (25, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (26, 25, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (27, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (28, 27, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (29, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (30, 29, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (31, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (32, 31, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (33, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (34, 33, 'Cache', 32768, 1, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (35, 20, 'Cache', 262144, 2, 64, 8);
INSERT INTO memsniffer (id, parent, name, size, depth, linesize, associativity)
  VALUES (36, 35, 'Cache', 32768, 1, 64, 8);
