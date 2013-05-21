/*
 * Copyright (C) 2013 The University of Texas at Austin
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PerfExpert. If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Ashay Rane
 */

package edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.Logger;

public class HPCToolkitProfileConstants {
    Logger log = Logger.getLogger( HPCToolkitProfileConstants.class );

    Map<String,Integer> perfCounterTranslation = new HashMap<String,Integer>();
    Map<Integer,Integer> HPCToPETranslation = new HashMap<Integer,Integer>();
    Map<String,Integer> lcpiTranslation = new HashMap<String,Integer>();

    int indexOfInstructions = -1;
    int indexOfCycles       = -1;
    int discoveredMetrics   = 0;
    long aggregateCycles    = 0;

    public Map<String,Integer> getPerfCounterTranslation() {
        return perfCounterTranslation;
    }

    public Map<String,Integer> getLCPITranslation() {
        return lcpiTranslation;
    }

    public void setPerfCounterTranslation(Map<String,Integer> perfCounterTranslation) {
        this.perfCounterTranslation = perfCounterTranslation;
    }

    public void setLCPITranslation(Map<String,Integer> lcpiTranslation) {
        this.lcpiTranslation = lcpiTranslation;
    }

    public int getIndexOfCycles() {
        return indexOfCycles;
    }

    public int getIndexOfInstructions() {
        return indexOfInstructions;
    }

    public long getAggregateCycles() {
        return aggregateCycles;
    }

    public void setAggregateCycles(long aggregateCycles) {
        this.aggregateCycles = aggregateCycles;
    }

    public void registerMetric(int HPCToolkitIndex, String metricName) {
        if (perfCounterTranslation.containsKey(metricName)) {
            // Register the duplicate
            // WARNING: There is a one-to-one correspondence between the
            //          position of the element within HPCToPETranslation and
            //          HPCToolkitIndex
            log.debug("Registered translation for duplicate metric " +
                      metricName + ", was recorded at " +
                      perfCounterTranslation.get(metricName));
            HPCToPETranslation.put(HPCToolkitIndex,
                                   perfCounterTranslation.get(metricName));
        } else {
            // Register new metric
            // Since we will require this frequently, it is best if we don't turn to the HashMap everytime
            if (metricName.equals("PAPI_TOT_INS")) {
                indexOfInstructions = discoveredMetrics;
            }
            if (metricName.equals("PAPI_TOT_CYC")) {
                indexOfCycles = discoveredMetrics;
            }
            perfCounterTranslation.put(metricName, discoveredMetrics);
            HPCToPETranslation.put(HPCToolkitIndex, discoveredMetrics);

            log.debug("Registered new metric " + metricName +
                      " with mapping: " + HPCToolkitIndex + " -> " +
                      discoveredMetrics);
            discoveredMetrics++;
        }
    }

    public void setUpLCPITranslation(Properties lcpiProperties) {
        // This is just setting up a mapping between LCPI names and their
        // corresponding index in the double array
        int LCPICount = 0;
        for (Object key : lcpiProperties.keySet()) {
            String LCPI = (String) key;
            if (!LCPI.equals("version")) {
                // Ignore version string
                lcpiTranslation.put((String) key, LCPICount);
                LCPICount++;
            }
        }
    }
}
