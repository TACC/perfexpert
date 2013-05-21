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

package edu.utexas.tacc.hpcdata;

import org.xml.sax.SAXException;

import org.apache.log4j.Logger;

import edu.utexas.tacc.hpcdata.parser.XMLParser;

import edu.utexas.tacc.argp.Parser;
import edu.utexas.tacc.argp.ArgPException;
import edu.utexas.tacc.argp.options.ArgType;
import edu.utexas.tacc.argp.options.Switch;
import edu.utexas.tacc.argp.options.FlaggedOption;
import edu.utexas.tacc.argp.options.UnflaggedOption;

import java.io.PrintStream;
import java.io.FileOutputStream;
import javax.xml.parsers.SAXParserFactory;

public class HPCData {
    public static void printHelp(Parser parser) {
        System.out.print(parser.getUsage("hpcdata.sh"));
    }

    public static void main(String args[]) throws Exception {
        Logger log = Logger.getLogger( HPCData.class );

        XMLParser xmlParser = new XMLParser();
        SAXParserFactory parserFactory = SAXParserFactory.newInstance();

        Parser parser = new Parser();
        Switch swHelp = new Switch('h', "help", "Show this help screen");
        FlaggedOption flOutput = new FlaggedOption('o', "output", "Output file",
                                                   "file", true, ArgType.STRING);
        UnflaggedOption ufExp = new UnflaggedOption("experiment.xml",
                                                    "experiment.xml file to be converted",
                                                    ArgType.STRING);

        parser.registerOption(swHelp);
        parser.registerOption(flOutput);
        parser.registerOption(ufExp);

        try {
            parser.parse(args);
        }
        catch(ArgPException e) {
            printHelp(parser);
            return;
        }

        if (parser.isSet(swHelp)) {
            printHelp(parser);
            return;
        }

        String xmlFile = (String) parser.getValue(ufExp);
        String output = (String) parser.getValue(flOutput);

        PrintStream outStream = null;
        if (null != output) {
            FileOutputStream out = new FileOutputStream(output);
            outStream = new PrintStream(out);
        } else {
            outStream = System.out;
        }

        try {
            javax.xml.parsers.SAXParser saxParser = parserFactory.newSAXParser();
            saxParser.parse(xmlFile, xmlParser);
        }
        catch (Exception e) {
            log.error("Error parsing converted file: " + xmlFile + "\n" +
                      e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }

        xmlParser.writeResults(outStream);
    }
}
