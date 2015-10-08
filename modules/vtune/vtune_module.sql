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

--
-- Enable foreign keys
--
PRAGMA foreign_keys = ON;

--
-- Create tables if not exist
--
CREATE TABLE IF NOT EXISTS vtune_hotspot (
    perfexpert_id INTEGER NOT NULL,
    id            INTEGER PRIMARY KEY,
    name          VARCHAR NOT NULL,
    type          INTEGER NOT NULL,
    profile       VARCHAR NOT NULL,
    module        VARCHAR NOT NULL,
    file          VARCHAR NOT NULL,
    line          INTEGER NOT NULL,
    depth         INTEGER NOT NULL,
    relevance     INTEGER
);

CREATE TABLE IF NOT EXISTS vtune_event (
    id            INTEGER PRIMARY KEY,
    name          VARCHAR NOT NULL,
    thread_id     INTEGER NOT NULL,
    mpi_task      INTEGER NOT NULL,
    experiment    INTEGER NOT NULL,
    value         REAL    NOT NULL,
    hotspot_id    INTEGER NOT NULL,
    
    FOREIGN KEY (vtune_hotspot_id) REFERENCES vtune_hotspot(id),
    FOREIGN KEY (arch_event_id) REFERENCES arch_event(id)
);

-- The information in this table is collected with
-- $ amplxe-runss -event-list
--CREATE TABLE IF NOT EXISTS vtune_counter_type (
--    id            INTEGER PRIMARY KEY,
--    name          VARCHAR NOT NULL UNIQUE,
--);
-- Not really needed. We use arc_event instead

CREATE TABLE IF NOT EXISTS vtune_counters (
    perfexpert_id INTEGER NOT NULL,
    id            INTEGER PRIMARY KEY,
    samples       INTEGER,
    value         INTEGER NOT NULL,
    mpi_rank      INTEGER,

    FOREIGN KEY (vtune_counters_id) REFERENCES vtune_counter_type(id)
);

CREATE TABLE IF NOT EXISTS vtune_default_events (
   FOREIGN KEY (id) REFERENCES arch_event(id),
   FOREIGN KEY (arch) REFERENCES arch_processor(id)
);

--
-- Populate tables
--
BEGIN TRANSACTION;
    INSERT INTO vtune_default_events (1499, 6); -- 'INST_RETIRED.ANY' SandyBridge
    INSERT INTO vtune_default_events (3211, 13); --'L1_DATA_PF1_MISS' Xeon Phi
END TRANSACTION;
