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
    family INTEGER NOT NULL,
    model  INTEGER NOT NULL,
    name   VARCHAR NOT NULL,
    value  REAL    NOT NULL
);

INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'CPI_threshold', 0.5);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'L1_dlat', 9.124111);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'L1_ilat', 9.124111);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'L2_lat', 10.359444);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'L3_lat', 18.654980);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'mem_lat', 275.229797);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'CPU_freq', 2699999000);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'FP_lat', 1.656944);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'FP_slow_lat', 17.912338);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'BR_lat', 1.000000);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'BR_miss_lat', 11.343750);
INSERT INTO hound (family, model, name, value) VALUES (6, 45, 'TLB_lat', 33.890625);

