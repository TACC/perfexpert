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

package edu.utexas.tacc.hpcdata.misc;

import java.io.PrintStream;

public class Printer {
    private static PrintStream outStream = null;

    public static void setStream(PrintStream o) {
        outStream = o;
    }

    public static void putHeader(String version, String header,
                                 String profile) {
        outStream.print("<HPCToolkitExperiment version=\"" + version +
                        "\">\n<Header n=\"" + header +
                        "\">\n  <Info/>\n</Header>\n<SecFlatProfile i=\"0\" n=\""
                        + profile + "\">\n<SecHeader>\n <MetricTable>\n");
    }

    public static void putMetric(long i, String metric) {
        outStream.print("    <Metric i=\"" + i + "\" n=\"" + metric +
                        " (E)\" v=\"final\" t=\"exclusive\" s=\"1\"> </Metric>\n");
    }

    public static void putIntermediate() {
        outStream.print(" </MetricTable>\n</SecHeader>\n<SecFlatProfileData>\n");
    }

    public static void putM(long i, double value) {
        outStream.print("<M n=\"" + i + "\" v=\"" + value + "\"/>");
    }

    public static void endProcedures() {
        outStream.print("</F></LM>");
    }

    public static void putAppendix() {
        outStream.print("</SecFlatProfileData></SecFlatProfile></HPCToolkitExperiment>\n");
    }

    public static void putProcFrame(long n, String name, long l) {
        outStream.print("<C i=\"0\" l=\"0\"><PF i=\"" + n + "\" n=\"" + name +
                        "\" l=\"" + l + "\"/></C>");
    }

    public static void endL() {
        outStream.print("\n</L>");
    }

    public static void endP() {
        outStream.print("\n</P>");
    }

    public static void endF() {
        outStream.print("</F>");
    }

    public static void endLM() {
        outStream.print("</LM>");
    }

    public static void putLM(long lm, String name) {
        outStream.print ("\n<LM i=\"" + lm + "\" n=\"Load module " + name +
                         "\">");
    }

    public static void putF(long f, String name) {
        outStream.print ("\n <F i=\"" + f + "\" n=\"" + name + "\">");
    }

    public static void putP(long n, String name, long l) {
        outStream.print ("\n <P i=\"" + n + "\" n=\"" + name + "\" l=\"" + l +
                         "\">\n");
    }

    public static void putL(long i, long s, long l) {
        outStream.print ("\n<L i=\"" + i + "\" s=\"" + s + "\" l=\"" + l +
                         "\">\n");
    }
}
