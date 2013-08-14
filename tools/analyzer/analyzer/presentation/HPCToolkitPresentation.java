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

package edu.utexas.tacc.perfexpert.presentation;

import java.text.DecimalFormat;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.autoscope.AutoSCOPE;
import edu.utexas.tacc.perfexpert.configuration.LCPIConfigManager;
import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;
import edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.MathParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfileConstants;

public class HPCToolkitPresentation {
    static short maxBarWidth = 47;
    enum Metric { METRIC_RATIO, METRIC_LCPI };
    static Logger log = Logger.getLogger( HPCToolkitPresentation.class );
    public static void presentRecommendations(List<HPCToolkitProfile> profiles01,
                                              LCPIConfigManager lcpiConfig,
                                              MachineConfigManager machineConfig,
                                              String file01,
                                              boolean aggregateOnly,
                                              int maxSuggestions,
                                              boolean opttran) { // Fialho: new flag
        Map<String, Float> lcpiMap = new HashMap<String,Float>(20);

        List<HPCToolkitProfile> profiles02 = null;
        String file02 = null;

        Metric metricType;

        if (null == profiles01 || 0 == profiles01.size()) {
            log.error("Received empty profiles as input, terminating...");
            return;
        }

        if (null == lcpiConfig) {
            log.error("Received empty LCPI configuration as input, terminating...");
            return;
        }

        if (null == machineConfig) {
            log.error("Received empty machine configuration as input, terminating...");
            return;
        }

        DecimalFormat doubleFormat = new DecimalFormat("#.###");
        MathParser mathParser = new MathParser();
        HPCToolkitProfileConstants profileConstants = profiles01.get(0).getConstants();
        Map<String, Integer> perfCtrTranslation = profileConstants.getPerfCounterTranslation();
        Integer indexOfCycles = perfCtrTranslation.get("PAPI_TOT_CYC");

        int indexOfInstructions = profileConstants.getIndexOfInstructions();
        String CPIThreshold = machineConfig.getProperties().getProperty("CPI_threshold");

        if (null == indexOfCycles) {
            log.error("Could not find PAPI_TOT_CYC among the list of discovered counters, cannot proceed with LCPI computation");
            return;
        }

        if (null == CPIThreshold) {
            log.error("Could not find CPI_threshold in machine.properties, defaulting to 0.5");
            CPIThreshold = "0.5";
        }

        double dCPIThreshold = Double.parseDouble(CPIThreshold);
        long cpuFrequency = Long.parseLong(machineConfig.getProperties().getProperty("CPU_freq"));

        for (HPCToolkitProfile profile : profiles01) {
            if ((profile.getCodeSectionInfo().equals("Aggregate") && !aggregateOnly) ||
                (aggregateOnly && !profile.getCodeSectionInfo().equals("Aggregate"))) {
                continue;
            }

            if (-1 == profile.getImportance()) {
                continue;
            }

            double cycles = profile.getMetricBasedOnPEIndex(indexOfCycles);
            double instructions = profile.getMetricBasedOnPEIndex(indexOfInstructions);

            // Compute each LCPI metric
            Map<String, Float> lcpi = new HashMap<String, Float>(20);
            for (String LCPI : lcpiConfig.getLCPINames()) {
                String formula = lcpiConfig.getProperties().getProperty(LCPI);
                double result01 = 0;

                try {
                    result01 = doubleFormat.parse(doubleFormat.format(mathParser.parse(formula, profile, machineConfig.getProperties()))).doubleValue();
                }
                catch (edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.ParseException e) {
                    log.error("Error in parsing expression: " + formula +
                              "\nDefaulting value of " + LCPI + " to zero.\n[" +
                              e.getMessage() + "]\n" + e.getStackTrace());
                }
                catch (java.text.ParseException e2) {
                    log.error("Error in parsing expression: " + formula +
                              "\nDefaulting value of " + LCPI + " to zero.\n["
                              + e2.getMessage() + "]\n" + e2.getStackTrace());
                }

                lcpi.put(LCPI, (float) result01);
            }

            lcpi.put("loop-depth", (float) profile.getLoopDepth());

            if (null != machineConfig.getProperties().getProperty("good-int-CPI")) {
                lcpi.put("good-int-CPI", Float.parseFloat(machineConfig.getProperties().getProperty("good-int-CPI")));
            }

            if (null != machineConfig.getProperties().getProperty("good-fp-CPI")) {
                lcpi.put("good-fp-CPI", Float.parseFloat(machineConfig.getProperties().getProperty("good-fp-CPI")));
            }
            
            // Fialho: removing the AutoSCOPE functionality from PerfExpert,
            //         outputing measurement when --recommend is in use.
            if (false == opttran) {
                AutoSCOPE as = new AutoSCOPE("/var/opt-sug-intel.pdb");
                as.addCodeSection(profile.getCodeSectionInfo() + " (" +
                                  doubleFormat.format(profile.getImportance()*100) +
                                  "% of the total runtime)", lcpi);
                System.out.println (as.recommend(maxSuggestions));
            } else {
                System.out.println("%");
                System.out.println("code.section_info=" + profile.getCodeSectionInfo());
                System.out.println("code.filename=" + profile.getCodeFilename());
                System.out.println("code.line_number=" + profile.getCodeLineNumber());
                System.out.println("code.type=" + profile.getCodeType());
                System.out.println("code.function_name=" + profile.getCodeProcedureName());
                System.out.println("code.extra_info=" + profile.getCodeExtraInfo());
                System.out.println("code.representativeness=" + doubleFormat.format(profile.getImportance()*100));
                System.out.println("code.runtime=" + doubleFormat.format(profile.getMetricBasedOnPEIndex(indexOfCycles)/cpuFrequency));

                // PerfExpert metrics with correspondent measurements
                Set s = lcpi.entrySet();
                Iterator it = s.iterator();
                while (it.hasNext()) {
                    Map.Entry m = (Map.Entry)it.next();
                    System.out.println("perfexpert." + m.getKey() + "=" + m.getValue());
                }
                
                // PAPI raw measurements
                s = perfCtrTranslation.entrySet();
                it = s.iterator();
                while (it.hasNext()) {
                    Map.Entry m = (Map.Entry)it.next();
                    System.out.println("papi." + m.getKey() + "=" + m.getValue());
                }
                
                // Hound host measurements
                s = machineConfig.getProperties().entrySet();
                it = s.iterator();
                while (it.hasNext()) {
                    Map.Entry m = (Map.Entry)it.next();
                    System.out.println("hound." + m.getKey() + "=" + m.getValue());
                }
            }
        }
    }

    public static void presentSummaryProfiles(List<HPCToolkitProfile> profiles01,
                                              List<HPCToolkitProfile> profiles02,
                                              LCPIConfigManager lcpiConfig,
                                              MachineConfigManager machineConfig,
                                              String file01, String file02,
                                              boolean aggregateOnly) {
        Metric metricType;

        if (null == profiles01 || 0 == profiles01.size()) {
            // log.error("Received empty profiles as input, terminating...");
            return;
        }

        if (null == lcpiConfig) {
            log.error("Received empty LCPI configuration as input, terminating...");
            return;
        }

        if (null == machineConfig) {
            log.error("Received empty machine configuration as input, terminating...");
            return;
        }

        DecimalFormat doubleFormat = new DecimalFormat("#.###");
        MathParser mathParser = new MathParser();
        HPCToolkitProfileConstants profileConstants = profiles01.get(0).getConstants();
        Map<String, Integer> perfCtrTranslation = profileConstants.getPerfCounterTranslation();
        Integer indexOfCycles = perfCtrTranslation.get("PAPI_TOT_CYC");

        Map<String, Integer> lcpiTranslation = profileConstants.getLCPITranslation();
        int indexOfInstructions = profileConstants.getIndexOfInstructions();
        String CPIThreshold = machineConfig.getProperties().getProperty("CPI_threshold");

        if (null == indexOfCycles) {
            log.error("Could not find PAPI_TOT_CYC among the list of discovered counters, cannot proceed with LCPI computation");
            return;
        }

        if (null == CPIThreshold) {
            log.error("Could not find CPI_threshold in machine.properties, defaulting to 0.5");
            CPIThreshold = "0.5";
        }

        long cpuFrequency = Long.parseLong(machineConfig.getProperties().getProperty("CPU_freq"));

        for (HPCToolkitProfile profile : profiles01) {
            if (profile.getCodeSectionInfo().equals("Aggregate")) {
                System.out.println ("Total running time for \"" + file01 +
                                    "\" is " +
                                    doubleFormat.format(profile.getMetricBasedOnPEIndex(indexOfCycles)/cpuFrequency) +
                                    " sec");
                break;
            }
        }

        if (null != profiles02) {
            for (HPCToolkitProfile profile : profiles02) {
                if (profile.getCodeSectionInfo().equals("Aggregate")) {
                    Integer indexOfCycles2 = profile.getConstants().getPerfCounterTranslation().get("PAPI_TOT_CYC");
                    System.out.println ("Total running time for \"" + file02 +
                                        "\" is " +
                                        doubleFormat.format(profile.getMetricBasedOnPEIndex(indexOfCycles2)/cpuFrequency) +
                                        " sec");
                    break;
                }
            }
        }

        double dCPIThreshold = Double.parseDouble(CPIThreshold);
        for (HPCToolkitProfile profile : profiles01) {
            if ((profile.getCodeSectionInfo().equals("Aggregate") && !aggregateOnly) ||
                (aggregateOnly && !profile.getCodeSectionInfo().equals("Aggregate"))) {
                continue;
            }
            
            if (-1 == profile.getImportance()) {
                continue;
            }
            
            double cycles = profile.getMetricBasedOnPEIndex(indexOfCycles);
            double instructions = profile.getMetricBasedOnPEIndex(indexOfInstructions);

            HPCToolkitProfile matchingProfile = getMatchingProfile(profile,
                                                                   profiles02);

            if (null == matchingProfile) {
                System.out.println("\n" + profile.getCodeSectionInfo() + " (" +
                                   doubleFormat.format(profile.getImportance()*100) +
                                   "% of the total runtime)");
            } else {
                Integer indexOfCycles2 = profiles02.get(0).getConstants().getPerfCounterTranslation().get("PAPI_TOT_CYC");
                double cycles2 = matchingProfile.getMetricBasedOnPEIndex(indexOfCycles2);

                System.out.println("\n" + profile.getCodeSectionInfo() +
                                   " (runtimes are " +
                                   doubleFormat.format(cycles/cpuFrequency) +
                                   "s and " +
                                   doubleFormat.format(cycles2/cpuFrequency) +
                                   "s)");
            }

            System.out.println("===============================================================================");

            double maxVariation = matchingProfile == null ? profile.getVariation() : (profile.getVariation() > matchingProfile.getVariation() ? profile.getVariation() : matchingProfile.getVariation());
            if (maxVariation > 0.2) {
                System.out.println("WARNING: The instruction count variation is " +
                                   doubleFormat.format(maxVariation*100) +
                                   "%, making the results unreliable");
            }
            if (cycles < cpuFrequency) {
                System.out.println("WARNING: The runtime for this code section is too short to gather meaningful measurements");
                continue;
            }

            double cpi = cycles / instructions;
            if (cpi <= dCPIThreshold) {
                System.out.println("The performance of this code section is good");
            }
            
            boolean printRatioHeader = false;
            boolean printPerfHeader = false;
            boolean printPercentHeader = false;
            // Compute each LCPI metric
            for (String LCPI : lcpiConfig.getLCPINames()) {
                String category, subcategory, subsubcategory;
                int indexOfPeriod = LCPI.indexOf(".");
                int indexOfPeriod2 = LCPI.indexOf(".", indexOfPeriod+1);    // If there is a second '.'
                if (0 > indexOfPeriod) {
                    category = "overall";
                    subcategory = LCPI;
                    subsubcategory = null;
                } else {
                    category = LCPI.substring(0, indexOfPeriod);

                    if (0 > indexOfPeriod2) {
                        subcategory = LCPI.substring(indexOfPeriod+1);
                        subsubcategory = null;
                    } else {
                        subcategory = LCPI.substring(indexOfPeriod+1, indexOfPeriod2);
                        subsubcategory = LCPI.substring(indexOfPeriod2+1);
                    }
                }

                String formula = lcpiConfig.getProperties().getProperty(LCPI);
                int index = lcpiTranslation.get(LCPI);

                double result01 = 0;
                try {
                    result01 = doubleFormat.parse(doubleFormat.format(mathParser.parse(formula, profile, machineConfig.getProperties()))).doubleValue();
                }
                catch (edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.ParseException e) {
                    log.error("Error in parsing expression: " + formula +
                              "\nDefaulting value of " + LCPI + " to zero.\n[" +
                              e.getMessage() + "]\n" + e.getStackTrace());
                }
                catch (java.text.ParseException e2) {
                    log.error("Error in parsing expression: " + formula +
                              "\nDefaulting value of " + LCPI + " to zero.\n[" +
                              e2.getMessage() + "]\n" + e2.getStackTrace());
                }

                profile.setLCPI(index, result01);
                log.debug(profile.getCodeSectionInfo() + ": " + category + "." +
                          subcategory + "." + subsubcategory + " = " + result01);

                double result02 = result01;
                if (null != matchingProfile) {
                    try {
                        result02 = doubleFormat.parse(doubleFormat.format(mathParser.parse(formula, matchingProfile, machineConfig.getProperties()))).doubleValue();
                    }
                    catch (edu.utexas.tacc.perfexpert.configuration.hpctoolkit.mathparser.ParseException e) {
                        log.error("Error in parsing expression: " + formula +
                                  "\nDefaulting value of " + LCPI +
                                  " to zero.\n[" + e.getMessage() + "]\n" +
                                  e.getStackTrace());
                    }
                    catch (java.text.ParseException e2) {
                        log.error("Error in parsing expression: " + formula +
                                  "\nDefaulting value of " + LCPI +
                                  " to zero.\n[" + e2.getMessage() + "]\n" +
                                  e2.getStackTrace());
                    }

                    matchingProfile.setLCPI(index, result02);
                    log.debug(matchingProfile.getCodeSectionInfo() + ": " +
                              category + "." + subcategory + " = " + result02);
                }

                String fCategory = category.replaceAll("_", " ");
                String fSubcategory = subcategory.replaceAll("_", " ");

                if (category.regionMatches(true, 0, "ratio", 0, category.length())) {
                    metricType = Metric.METRIC_RATIO;
                    if (false == printRatioHeader) {
                        // Print ratio header
                        System.out.println (String.format("%-25s    %%  0.........25...........50.........75........100", "ratio to total instrns"));
                        printRatioHeader = true;
                    }
                } else if (category.regionMatches(true, 0, "percent", 0,
                                                  category.length())) {
                    metricType = Metric.METRIC_RATIO;
                    if (false == printPercentHeader) {
                        printPercentHeader = true;
                    }

                    // Shift them around
                    category = subcategory;
                    subcategory = subsubcategory;

                    fCategory = category.replaceAll("_", " ");
                    fSubcategory = subcategory.replaceAll("_", " ");
                } else {
                    metricType = Metric.METRIC_LCPI;
                    if (false == printPerfHeader) {
                        // Print perf header
                        System.out.println("-------------------------------------------------------------------------------");
                        System.out.println (String.format("%-25s  LCPI good......okay......fair......poor......bad....", "performance assessment"));
                        printPerfHeader = true;
                    }
                }

                if (subcategory.regionMatches(true, 0, "overall", 0,
                                              subcategory.length())) {
                    // Print the category name
                    System.out.print(String.format("%-25s: ", "* " + fCategory));
                } else {
                    System.out.print(String.format("%-25s: ", "   - " +
                                                   fSubcategory));
                }
                
                if (null == matchingProfile) {
                    if (metricType == Metric.METRIC_RATIO) {
                        // FIXME: L1_DCA / TOT_INS ration going beyond 100% in some cases

                        // Cap values to 100
                        if (1 < result01) {
                            result01 = 1;
                        }
                        if (1 < result02) {
                            result02 = 1;
                        }
                        
                        System.out.print(String.format("%4.0f ", result01*100));
                        printRatioBar(result01*100, result02*100, dCPIThreshold);
                    } else {
                        System.out.print(String.format("%4.1f ", result01));
                        printLCPIBar(result01, result02, dCPIThreshold);
                    }
                } else {
                    if (metricType == Metric.METRIC_RATIO) {
                        // Cap values to 100
                        if (1 < result01) {
                            result01 = 1;
                        }
                        if (1 < result02) {
                            result02 = 1;
                        }

                        System.out.print(String.format("     ", result01*100));
                        printRatioBar(result01*100, result02*100, dCPIThreshold);
                    } else {
                        System.out.print("     ");
                        printLCPIBar(result01, result02, dCPIThreshold);
                    }
                }

                if (category.regionMatches(true, 0, "overall", 0,
                                           category.length())) {
                    // Print line about upper bounds
                    System.out.println("upper bound estimates");
                }
            }
        }
    }

    private static HPCToolkitProfile getMatchingProfile(HPCToolkitProfile needle,
                                                        List<HPCToolkitProfile> smallHaystack) {
        if (null == smallHaystack) {
            return null;
        }
        
        String searchString = needle.getCodeSectionInfo();
        for (HPCToolkitProfile profile : smallHaystack) {
            if (profile.getCodeSectionInfo().equals(searchString)) {
                return profile;
            }
        }

        return null;
    }

    private static void printRatioBar(double value1, double value2,
                                      double cpiThreshold) {
        // Scale to maxBarWidth
        value1 *= maxBarWidth / 100.0;
        value2 *= maxBarWidth / 100.0;

        
        if (1 > value1) {
            value1 = 1;
        }
        if (1 > value2) {
            value2 = 1;
        }
        
        char term = ' ';
        if (value1 > maxBarWidth) {
            term = '+';
            value1 = maxBarWidth-1;
        }

        if (value2 > maxBarWidth) {
            term = '+';
            value2 = maxBarWidth-1;
        }

        double min = value1 < value2 ? value1 : value2;
        value1 -= min;
        value2 -= min;

        while(0.5 < min--) {
            System.out.print('*');
        }
        
        if (0 < value1) {
            value1 += min;
            while (0.5 < value1--) {
                System.out.print(1);
            }
        } else {
            value2 += min;
            while (0.5 < value2--) {
                System.out.print(2);
            }
        }

        if (' ' != term) {
            System.out.print(term);
        }
        
        System.out.print("\n");
    }

    private static void printLCPIBar(double value1, double value2,
                                     double cpiThreshold) {
        value1 *= 10 / cpiThreshold;
        value2 *= 10 / cpiThreshold;

        if (1 > value1) {
            value1 = 1;
        }
        if (1 > value2) {
            value2 = 1;
        }
        
        char term = ' ';
        if (value1 > maxBarWidth) {
            term = '+';
            value1 = maxBarWidth-1;
        }

        if (value2 > maxBarWidth) {
            term = '+';
            value2 = maxBarWidth-1;
        }

        double min = value1 < value2 ? value1 : value2;
        value1 -= min;
        value2 -= min;

        while (0.5 < min--) {
            System.out.print('>');
        }

        if (0 < value1) {
            value1 += min;
            while (0.5 < value1--) {
                System.out.print(1);
            }
        } else {
            value2 += min;
            while (0.5 < value2--) {
                System.out.print(2);
            }
        }

        if (' ' != term) {
            System.out.print(term);
        }
        
        System.out.print("\n");
    }
}
