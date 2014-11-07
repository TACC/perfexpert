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

-- Generated using hound
-- version 2.0

--
-- Create tables if not exist
--
CREATE TABLE IF NOT EXISTS hound (
    id     INTEGER,
    family INTEGER NOT NULL,
    model  INTEGER NOT NULL,
    name   VARCHAR NOT NULL,
    value  REAL    NOT NULL
);

INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'CPI_threshold', 1.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'L1_dlat', 3.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'L1_ilat', 3.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'L2_lat', 24.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'mem_lat', 300.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'CPU_freq', 1100000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'FP_lat', 4.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'FP_slow_lat', 20.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'BR_lat', 1.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'BR_miss_lat', 15.000000);
INSERT INTO hound (id, family, model, name, value) VALUES (7, 6, 45, 'TLB_lat', 40.000000);

