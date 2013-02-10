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

package edu.utexas.tacc.hpcdata.parser;

import org.apache.log4j.Logger;

import edu.utexas.tacc.hpcdata.misc.Printer;
import edu.utexas.tacc.hpcdata.structures.Node;
import edu.utexas.tacc.hpcdata.structures.Loop;
import edu.utexas.tacc.hpcdata.structures.Procedure;

import java.util.Map;
import java.util.List;
import java.util.Stack;
import java.util.Vector;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Collections;

import java.io.PrintStream;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

public class XMLParser extends DefaultHandler {
    private Logger log = Logger.getLogger( XMLParser.class );

    long module, file;
    String header,profile,version;

    List<String> metric_name_list;
    List<Double> aggregate_metrics;
    Map<Long,Procedure> proc_map;
    Map<Long,Loop> loop_map;
    Map<Long,String> proc_name_map;
    Map<Long,String> file_name_map;
    Map<Long,String> module_name_map;

    Vector<Procedure> procedureSeq;
    Stack<Node> codeSegmentStack;
    Map<Procedure,Node> proc_tree_map;

    public XMLParser() {
        module           = -1;
        file             = -1;
        proc_name_map    = new HashMap<Long,String>();
        file_name_map    = new HashMap<Long,String>();
        metric_name_list = new ArrayList<String>();
        module_name_map  = new HashMap<Long,String>();
        proc_map         = new HashMap<Long,Procedure>();
        loop_map         = new HashMap<Long,Loop>();

        procedureSeq     = new Vector<Procedure>();
        codeSegmentStack = new Stack<Node>();
        proc_tree_map    = new HashMap<Procedure,Node>();
    }

    @Override
    public void startElement(String uri, String localName, String qName,
                             Attributes attr) throws SAXException {
        if (qName.equals("HPCToolkitExperiment")) {
            version = attr.getValue("version");
        }
        
        if (qName.equals("Header")) {
            header = attr.getValue("n");
        }
        
        if (qName.equals("SecCallPathProfile")) {
            profile = attr.getValue("n");
        }
        
        if (qName.equals("LoadModule")) {
            if (null != attr.getValue("i") && null != attr.getValue("n")) {
                module_name_map.put(Long.parseLong(attr.getValue("i")),
                                    attr.getValue("n").replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"));
            }
        }
        
        if (qName.equals("File")) {
            if (null != attr.getValue("i") && null != attr.getValue("n")) {
                String [] file_bits = attr.getValue("n").split("/");
                file_name_map.put(Long.parseLong(attr.getValue("i")),
                                  file_bits[file_bits.length - 1].replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"));
            }
        }

        if (qName.equals("Metric")) {
            long id = 0;
            String name = "";

            if (null != attr.getValue("i") && null != attr.getValue("n")) {
                id = Long.parseLong(attr.getValue("i"));
                name = attr.getValue("n");
            }
            metric_name_list.add(name);
        }

        if (qName.equals("Procedure")) {
            // Record procedure name and ID
            if (null != attr.getValue("i") && null != attr.getValue("n")) {
                proc_name_map.put(Long.parseLong(attr.getValue("i")),
                                  attr.getValue("n").replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"));
            }
        }

        if (qName.equals("PF") || qName.equals("Pr")) {
            if (null == attr.getValue("n")) {
                log.error("\"PF\" (procedure frame) with no name");
                throw new SAXException();
            }

            long n = Long.parseLong(attr.getValue("n"));

            Procedure p = null;
            if (proc_map.containsKey(n)) {
                p = proc_map.get(n);
            } else {
                String a = attr.getValue("a");
                if (a == null)    a="0";

                p = new Procedure(attr.getValue("i"),
                                  attr.getValue("s"),
                                  attr.getValue("l"),
                                  attr.getValue("n"),
                                  attr.getValue("lm"),
                                  attr.getValue("f"),
                                  a);
                
                // Not inlined or alien
                if ("0" == a) {
                    procedureSeq.add(p);
                }
                proc_map.put(n, p);
            }

            if (!codeSegmentStack.empty()) {
                // Add this procedure as a child of whatever is on top of the procedure stack
                Node topNode = null;
                try {
                    topNode = codeSegmentStack.peek();
                }
                catch (java.util.EmptyStackException e) {
                    throw new SAXException();
                }
                if (null != topNode) {
                    topNode.addChild(p);
                }
            }
            codeSegmentStack.push(p);
        }

        if (qName.equals("L")) {
            if (null == attr.getValue("s")) {
                log.error("\"L\" (Loop) with no 's' attribute");
                throw new SAXException();
            }

            Node topNode = null;
            Procedure containingProc = null;

            try {
                topNode = codeSegmentStack.peek();
            }
            catch (java.util.EmptyStackException e) {
                log.error("Found \"L\" (loop) without a containing \"P\" (procedure)");
                throw new SAXException();
            }

            if (topNode.getType().equals("L")) {
                containingProc = ((Loop) topNode).getContainingProc();
            } else {
                containingProc = (Procedure) topNode;
            }
            long s = Long.parseLong(attr.getValue("s"));

            Loop l = null;
            if (loop_map.containsKey(s)) {
                l = loop_map.get(s);
            } else {
                l = new Loop(attr.getValue("i"),
                             attr.getValue("s"),
                             attr.getValue("l"),
                             containingProc);

                loop_map.put(s, l);
            }
            topNode.addChild(l);
            codeSegmentStack.push(l);
        }

        if (qName.equals("M")) {
            if (null == attr.getValue("n") || null == attr.getValue("v")) {
                log.error("Malformed \"M\" (metric) element");
                throw new SAXException();
            }

            Node topNode = null;
            try {
                topNode = codeSegmentStack.peek();
            }
            catch (java.util.EmptyStackException e) {
                log.error("\"M\" (metric) element found outside of any procedure");
                throw new SAXException();
            }

            long n = Long.parseLong(attr.getValue("n"));
            double v = Double.parseDouble(attr.getValue("v"));

            topNode.setMetric(n, topNode.getMetric(n) + v);
            percolateUp(topNode, n, v);

            // Also add to aggregate metrics
            aggregate_metrics.set((int) n, aggregate_metrics.get((int) n) + v);
        }
    }

    void percolateUp(Node parent, long n, double v) {
        if (null == parent) {
            return;
        }
        Node grandParent = parent.getParent();

        if (parent instanceof Procedure) {
            if (1 == ((Procedure) parent).getA()) {  // Inlined (alien)
                percolateUp(grandParent, n, v);
                percolateToLoop(grandParent, n, v);
            }
        } else { // parent is a Loop
            percolateUp(grandParent, n, v);
        }
    }

    void percolateToLoop(Node parent, long n, double v) {
        if (null == parent || parent instanceof Procedure) {
            return;
        }
        // parent is a Loop
        parent.setMetric(n, parent.getMetric(n) + v);
    }

    @Override
    public void endElement(String uri, String localName,
                           String qName) throws SAXException {
        if (qName.equals("MetricTable")) {
            if (0 == metric_name_list.size()) {
                System.err.println("Found zero metrics, is the input XML file correct?");
            } else {
                aggregate_metrics = new ArrayList<Double>(metric_name_list.size());
                // aggregate_metrics.ensureCapacity(metric_name_list.size());

                // Stupid way, but it (is the only way that) works
                for (long i = 0; i < metric_name_list.size(); i++) {
                    aggregate_metrics.add(0.0);
                }
            }
        }

        if (qName.equals("PF") || qName.equals("L") || qName.equals("Pr")) {
            try {
                codeSegmentStack.pop();
            }
            catch (java.util.EmptyStackException e) {
                log.error("Ended \"PF\" (procedure frame) or \"L\" (loop) element without anything on the stack");
                throw new SAXException();
            }
        }
    }

    public void writeResults(PrintStream out) {
        if (!codeSegmentStack.empty()) {
            log.error("Unprocessed \"PF\" (procedure frame) or \"L\" (loop) element(s) on the stack");
            return;
        }

        Collections.sort(procedureSeq);

        Printer.setStream(out);
        Printer.putHeader(version, header, profile);

        long i = 0;
        for (String metric : metric_name_list) {
            Printer.putMetric(i, metric);
            i++;
        }
        Printer.putIntermediate();

        for (i = 0; i < metric_name_list.size(); i++) {
            double val = aggregate_metrics.get((int) i);
            if (0 != val) {
                Printer.putM(i, aggregate_metrics.get((int) i));
            }
        }

        if (0 != procedureSeq.size()) {
            for (Procedure p : procedureSeq) {
                dfs(p, true);
            }
            Printer.endProcedures();
        }
        Printer.putAppendix();
    }

    void dfs(Node n, boolean first) {
        boolean printModule = false, printFile = false;
        
        if (n.getType().equals("P")) {
            Procedure p = (Procedure) n;

            if (0 == !first && p.getA()) {
                // Output a C and then a PF
                 Printer.putProcFrame(p.getN(), proc_name_map.get(p.getN()), p.getL());
            } else {
                if (1 != p.getA()) {
                    if (p.getLM() != module) {
                        printModule=true;
                    }
                    if (printModule) {
                        if (-1 != file) {
                            Printer.endF();
                        }
                        if (-1 != module) {
                            Printer.endLM();
                        }
                        Printer.putLM(p.getLM(), module_name_map.get(p.getLM()));
                    }

                    module = p.getLM();

                    if (p.getF() != file) {
                        printFile=true;
                    }
                    if (printFile) {
                        if (-1 != !printModule && file) {
                            Printer.endF();
                        }
                        Printer.putF(p.getF(), file_name_map.get(p.getF()));
                    }

                    file = p.getF();
                    Printer.putP(p.getN(), proc_name_map.get(p.getN()),
                                 p.getL());
                } else {
                    Printer.putP(p.getN(),
                                 "inlined from " + file_name_map.get(p.getF()) + ": " + p.getL(),
                                 p.getL());
                }
                if (!p.emptyMetricSet()) {
                    Map<Long,Double> metrics = p.getAllMetrics();
                    for (long i = 0; i <= p.getMaxIndex(); i++) {
                        if (metrics.containsKey(i)) {
                            Printer.putM(i, p.getMetric(i));
                        }
                    }
                }

                for (Node c : p.getChildren()) {
                    dfs(c, false);
                }
                Printer.endP();
            }
        } else if (n.getType().equals("L")) {
            Loop l = (Loop) n;

            Printer.putL(l.getI(), l.getS(), l.getL());

            if (!l.emptyMetricSet()) {
                Map<Long,Double> metrics = l.getAllMetrics();
                for (long i = 0; i <= l.getMaxIndex(); i++) {
                    if (metrics.containsKey(i)) {
                        Printer.putM(i, l.getMetric(i));
                    }
                }
            }

            for (Node c : l.getChildren()) {
                dfs(c, false);
            }
            Printer.endL();
        }
    }
}
