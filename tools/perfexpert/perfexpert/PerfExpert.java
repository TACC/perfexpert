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

package edu.utexas.tacc.perfexpert;

import java.io.PrintWriter;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import org.apache.log4j.Logger;

import edu.utexas.tacc.argp.Parser;
import edu.utexas.tacc.argp.ArgPException;
import edu.utexas.tacc.argp.options.Switch;
import edu.utexas.tacc.argp.options.ArgType;
import edu.utexas.tacc.argp.options.FlaggedOption;
import edu.utexas.tacc.argp.options.UnflaggedOption;

import edu.utexas.tacc.perfexpert.misc.RangeParser;
import edu.utexas.tacc.perfexpert.autoscope.AutoSCOPE;
import edu.utexas.tacc.perfexpert.configuration.LCPIConfigManager;
import edu.utexas.tacc.perfexpert.configuration.MachineConfigManager;
import edu.utexas.tacc.perfexpert.parsing.hpctoolkit.HPCToolkitParser;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;
import edu.utexas.tacc.perfexpert.presentation.HPCToolkitPresentation;

public class PerfExpert {
    public static void printHelp(Parser p) {
        System.out.print(p.getUsage("perfexpert"));
    }

    public static void main(String[] args) throws Exception {
        Logger log = Logger.getLogger( PerfExpert.class );

        String PERFEXPERT_HOME = "";
        ClassLoader loader = PerfExpert.class.getClassLoader();
        String regex = "jar:file:(.*)/perfexpert.jar!/edu/utexas/tacc/perfexpert/PerfExpert.class";
        String jarURL = loader.getResource("edu/utexas/tacc/perfexpert/PerfExpert.class").toString();

        Pattern p = Pattern.compile(regex);
        Matcher m = p.matcher(jarURL);

        if (m.find()) {
            PERFEXPERT_HOME = m.group(1);
        } else {
            log.error("Could not extract location of PerfExpert jar from URL: (" +
                      jarURL + "), was using the regex: \"" + regex + "\"");
            System.exit(1);
        }

        Switch swHelp = new Switch('h', "help", "Show this screen");
        Switch swAgg = new Switch('a', "aggregate", "Show whole-program information only");
        FlaggedOption flRecommend = new FlaggedOption('r',
                                                      "recommend",
                                                      "Recommend suggestions for optimization",
                                                      "limit",
                                                      false, ArgType.INTEGER);
        FlaggedOption flThreads = new FlaggedOption('t', "threads",
                                                    "Show information for specific threads",
                                                    "thread#",
                                                    true, ArgType.STRING);
        UnflaggedOption ufThreshold = new UnflaggedOption("threshold",
                                                          "Threshold between 0 and 1",
                                                          ArgType.DOUBLE);
        UnflaggedOption ufExp01 = new UnflaggedOption("experiment.xml",
                                                      "experiment.xml file generated using `perfexpert_run_exp'",
                                                      ArgType.STRING);
        UnflaggedOption ufExp02 = new UnflaggedOption("experiment.xml",
                                                      "Second experiment.xml file, for comparison only. Not valid with -r,--recommend",
                                                      ArgType.STRING);
        ufExp02.setMandatory(false);

        Parser parser = new Parser();
        parser.registerOption(swHelp);
        parser.registerOption(swAgg);
        parser.registerOption(flRecommend);
        parser.registerOption(flThreads);
        parser.registerOption(ufThreshold);
        parser.registerOption(ufExp01);
        parser.registerOption(ufExp02);

        try {
            parser.parse(args);
        }
        catch (ArgPException e) {
            printHelp(parser);
            return;
        }

        // Sanity check
        if ((parser.isSet(flRecommend) && parser.isSet(ufExp02)) ||
            parser.isSet(swHelp)) {
            printHelp(parser);
            return;
        }

        Double threshold = 0.1;
        Integer max_suggestions = 0;
        boolean recommend = false, aggregateOnly = false;
        String filename01 = null, filename02 = null, threads = null;

        threshold = (Double) parser.getValue(ufThreshold);
        recommend = parser.isSet(flRecommend);
        if (recommend) {
            max_suggestions=(Integer) parser.getValue(flRecommend);
        }
        if (null == max_suggestions) {
            max_suggestions=0;
        }
        aggregateOnly = parser.isSet(swAgg);
        filename01 = (String) parser.getValue(ufExp01);
        filename02 = (String) parser.getValue(ufExp02);
        threads = (String) parser.getValue(flThreads);

        log.debug("PerfExpert invoked with arguments: " + threshold + ", " +
                  filename01 + ", " + filename02);

        String HPCDATA_LOCATION = System.getenv("PERFEXPERT_HPCDATA_HOME");
        if ((null == HPCDATA_LOCATION) || (0 == HPCDATA_LOCATION.length())) {
            log.error("The environment variable ${PERFEXPERT_HPCDATA_HOME} was not set, cannot proceed");
            return;
        }

        LCPIConfigManager lcpiConfig = new LCPIConfigManager("file://" +
                                                             PERFEXPERT_HOME +
                                                             "/../etc/lcpi.properties");
        if (lcpiConfig.readConfigSource() == false) {
            // Error while reading configuraiton, handled inside method
            return;
        }

        MachineConfigManager machineConfig = new MachineConfigManager("file://" +
                                                                      PERFEXPERT_HOME +
                                                                      "/../etc/machine.properties");
        if (false == machineConfig.readConfigSource()) {
            // Error while reading configuraiton, handled inside method
            return;
        }

        String thread_regex = RangeParser.getRegexString(threads);

        List<HPCToolkitProfile> profiles01 = null, profiles02 = null;
        HPCToolkitParser parser01 = new HPCToolkitParser(HPCDATA_LOCATION,
                                                         threshold,
                                                         "file://" + filename01,
                                                         lcpiConfig.getProperties());
        profiles01 = parser01.parse(aggregateOnly, thread_regex);

        if (null != filename02) {
            HPCToolkitParser parser02 = new HPCToolkitParser(HPCDATA_LOCATION,
                                                             0,
                                                             "file://" + filename02,
                                                             lcpiConfig.getProperties());
            profiles02 = parser02.parse(aggregateOnly, thread_regex);
        }

        if (!recommend) {
            HPCToolkitPresentation.presentSummaryProfiles(profiles01,
                                                          profiles02,
                                                          lcpiConfig,
                                                          machineConfig,
                                                          filename01,
                                                          filename02,
                                                          aggregateOnly);
        } else {
            HPCToolkitPresentation.presentRecommendations(profiles01,
                                                          lcpiConfig,
                                                          machineConfig,
                                                          filename01,
                                                          aggregateOnly,
                                                          max_suggestions);
        }
    }
}
