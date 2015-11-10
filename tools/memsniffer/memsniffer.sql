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
