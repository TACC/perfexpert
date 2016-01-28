--
-- Copyright (c) 2011-2016  University of Texas at Austin. All rights reserved.
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
-- Authors: Antonio Gomez-Iglesias, Leonardo Fialho and Ashay Rane
--
-- $HEADER$
--

--
-- Enable foreign keys
--
PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS vtune_counters (
    perfexpert_id INTEGER NOT NULL,
    id            INTEGER PRIMARY KEY,
    samples       INTEGER,
    value         INTEGER NOT NULL,
    mpi_rank      INTEGER,
    vtune_counters_id INTEGER,

    FOREIGN KEY (vtune_counters_id) REFERENCES vtune_counter_type(id)
);
