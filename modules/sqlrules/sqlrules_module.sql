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
-- Create tables
--
CREATE TABLE IF NOT EXISTS rule (
    id        INTEGER PRIMARY KEY,
    name      VARCHAR NOT NULL,
    statement VARCHAR NOT NULL
);

--
-- AustoSCOPE recommendation selection algorithm
--
-- @HID is replaced by the hotspot ID
--
INSERT INTO rule (name, statement) VALUES
    ('AutoSCOPE recommendation algorithm',
    "SELECT
    recommendation.id,
    SUM(
        lcpi_metric.value - (maximum * 0.1)
    ) AS score
FROM
    recommendation
INNER JOIN rc ON recommendation.id = rc.rid
INNER JOIN category ON category.id = rc.cid
JOIN lcpi_metric ON category.name = lcpi_metric.name
JOIN (
    SELECT
        MAX(value) AS maximum
    FROM
        lcpi_metric
    WHERE
        hotspot_id = @HID
    AND (
        SELECT
            (
                MAX(
                    CASE name
                    WHEN 'overall' THEN
                        (value * 100)
                    END
                ) / (
                    (
                        0.5 * (
                            100 - MAX(
                                CASE name
                                WHEN 'ratio.floating_point' THEN
                                    value
                                END
                            )
                        )
                    ) + (
                        1.0 * MAX(
                            CASE name
                            WHEN 'ratio.floating_point' THEN
                                value
                            END
                        )
                    )
                )
            )
        FROM
            lcpi_metric
        WHERE
            hotspot_id = @HID
    ) > 1
)
JOIN hotspot ON hotspot.id = lcpi_metric.hotspot_id
WHERE
    (
        recommendation.depth <= hotspot.depth
    )
AND lcpi_metric.hotspot_id = @HID
GROUP BY
    recommendation.id
ORDER BY
    score DESC;");
