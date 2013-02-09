/*
    Copyright (C) 2013 The University of Texas at Austin

    This file is part of PerfExpert.

    PerfExpert is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PerfExpert is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.

    Author: Ashay Rane
*/

package edu.utexas.tacc.perfexpert.parsing.hpctoolkit;

import java.util.ArrayList;
import java.util.List;
import java.util.Properties;
import java.util.Stack;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.log4j.Logger;
import org.xml.sax.Attributes;
import org.xml.sax.helpers.DefaultHandler;

import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfile;
import edu.utexas.tacc.perfexpert.parsing.profiles.hpctoolkit.HPCToolkitProfileConstants;

public class HPCToolkitXMLParser extends DefaultHandler
{
	Logger log = Logger.getLogger( HPCToolkitXMLParser.class );

	boolean inCallSite = false;
	boolean aggregateOnly = false;
	boolean aggregateRecorded = false;

	Properties LCPIConfig;
	double threshold = 0.1;
	String filename = null;
	HPCToolkitProfile profile;
	int metrics=0, loopDepth=0;
	String loadedModule = null;

	String threadRegex = "(\\d+,)?([\\d]+)";
	Stack<String> procedureStack = new Stack<String>();

	// Arbitrarily  selected default initial capacity of 50
	List<HPCToolkitProfile> profileList = new ArrayList<HPCToolkitProfile>(50);
	HPCToolkitProfileConstants profileConstants = new HPCToolkitProfileConstants();

	public void setThreshold(double threshold)
	{
		this.threshold = threshold;
	}

	public void setLCPIConfig(Properties LCPIConfig)
	{
		this.LCPIConfig = LCPIConfig;
	}

	public void setIfAggregateOnly(boolean aggregateOnly)
	{
		this.aggregateOnly = aggregateOnly;
	}

	public void setThreadRegex(String threadRegex)
	{
		this.threadRegex = threadRegex;
	}

	@Override
	public void startDocument()
	{
		profileConstants.setUpLCPITranslation(LCPIConfig);
	}

	@Override
	public void startElement(String uri, String localName, String qName, Attributes attr) throws XMLParsingDoneException
	{
		if (inCallSite)	// Everything to be ignored till we see a ending "C" element
		{
			log.debug("Skipping element \"" + qName + "\" because we are currently within a callsite (C) element");
			return;
		}

		// Procedure
		if (qName.equals("P"))
		{
			profile = null;
			aggregateRecorded = true;
			setAggregateCyclesFromRootProfile();

			if (aggregateOnly)	throw new XMLParsingDoneException();

			int lineNumber = Integer.parseInt(attr.getValue("l"));
			String procedureName = attr.getValue("n");

			procedureStack.push(procedureName);
			String codeSection = formatCodeSection(lineNumber, true);

			log.debug("Found a \"P\" element for \"" + procedureName + "\"" + codeSection + ", starting to record a new Profile object");

			profile = new HPCToolkitProfile(profileConstants);
			profile.setCodeSectionInfo("Function " + procedureName + (procedureName.contains("(") ? "" : "()") + codeSection);
            
            // Fialho: adding some info to make my life easier.
            profile.setCodeFilename(filename);
            profile.setCodeLineNumber(Integer.parseInt(attr.getValue("l")));
            profile.setCodeExtraInfo(attr.getValue("n"));
            profile.setCodeType("function");

			profileList.add(profile);
			return;
		}

		// Loop
		if (qName.equals("L"))
		{
			profile = null;
			aggregateRecorded = true;
			setAggregateCyclesFromRootProfile();

			if (aggregateOnly)	throw new XMLParsingDoneException();

			int lineNumber = Integer.parseInt(attr.getValue("l"));
			String codeSection = formatCodeSection(lineNumber, false);

			loopDepth++;
			log.debug("Found an \"L\" element for" + codeSection + ", starting to record a new Profile object");

			profile = new HPCToolkitProfile(profileConstants);
			profile.setLoopDepth(loopDepth);
			profile.setCodeSectionInfo("Loop" + codeSection);

            // Fialho: adding some info to make my life easier.
            profile.setCodeFilename(filename);
            profile.setCodeLineNumber(Integer.parseInt(attr.getValue("l")));
            profile.setCodeType("loop");
            profile.setCodeExtraInfo(Integer.toString(loopDepth));

            profileList.add(profile);
			return;
		}

		// If it a metric itself, record code section and add to Profile values
		if (qName.equals("M"))
		{
			if (aggregateRecorded == true && profile == null)
			{
				log.debug("Found an \"M\" element but ignored, looks like it was under an LM, S, or F");
				return;	// Ignore because this metric must be under an LM or S or F
			}

			log.debug("Found an \"M\" element");
			if (aggregateRecorded == false && profile != null)
				log.warn("Some other metric seems to have been recorded before the aggregate data was collected, trying to proceeed as normal...");

			int index = Integer.parseInt(attr.getValue("n"));
			double value = Double.parseDouble(attr.getValue("v"));

			// Check if this is aggregated program information
			if (aggregateRecorded == false && profile == null)
			{
				log.debug("This metric marks the beginning of recording the aggregate information");

				profile = new HPCToolkitProfile(profileConstants);
				profile.setCodeSectionInfo("Aggregate");
				profileList.add(profile);

				aggregateRecorded = true;
			}

			profile.setMetric(index, value);

			// The above block (and other similar blocks) can be optimized for performance but leaving to to the compiler
			// Trying to keep the code readable

			return;
		}

		// Record filename (bonus!)
		if (qName.equals("F"))
		{
			profile = null;
			aggregateRecorded = true;
			setAggregateCyclesFromRootProfile();

			if (aggregateOnly)	throw new XMLParsingDoneException();

			filename = attr.getValue("n");
			log.debug("Found a new \"F\" element for \"" + filename + "\"");
			return;
		}

		if (qName.equals("LM"))
		{
			profile = null;
			aggregateRecorded = true;
			setAggregateCyclesFromRootProfile();

			if (aggregateOnly)	throw new XMLParsingDoneException();

			loadedModule = attr.getValue("n");
			log.debug("Found new loaded module " + loadedModule);
			return;
		}

		// Don't process S (statement) elements
		if (qName.equals("S"))
		{
			profile = null;
			aggregateRecorded = true;
			setAggregateCyclesFromRootProfile();

			if (aggregateOnly)	throw new XMLParsingDoneException();

			log.debug("Not processing \"S\" element");
			return;
		}

		// If it is a C (callsite), ignore till we see an ending C element
		if (qName.equals("C"))
		{
			profile = null;
			aggregateRecorded = true;
			setAggregateCyclesFromRootProfile();

			if (aggregateOnly)	throw new XMLParsingDoneException();

			log.debug("Found a \"C\" element, skipping till corresponding ending element is found");
			inCallSite = true;

			return;
		}

		// If it is an entry from the metric table, record the name and the index
		if (qName.equals("Metric"))
		{
			String metricName = attr.getValue("n");
			int HPCToolkitIndex = Integer.parseInt(attr.getValue("i"));

			String regex = "^(|\\d+\\.)([\\w:]+)\\.\\[(\\d+,)?(\\d+)\\](|\\.\\d+) \\((\\w)\\)$";
			Pattern p = Pattern.compile(regex);
			Matcher m = p.matcher(metricName);

			// First-level match, to find if we have a valid metric string
			if (m.find())
			{
				regex = "^(|\\d+\\.)([\\w:]+)\\.\\[" + this.threadRegex + "\\](|\\.\\d+) \\((\\w)\\)$";
				p = Pattern.compile(regex);
				m = p.matcher(metricName);

				// Second level match, to extract metrics for specific threads only
				if (m.find())
				{
					String revisedMetricName = m.group(2) + (m.group(6).equals("I") ? "_I" : "");
					profileConstants.registerMetric(HPCToolkitIndex, revisedMetricName);
				}
				else
					; // Ignore
			}
			else
				log.debug("Encountered \"Metric\" element that did not match the regex: " + regex + "\n"
						+ "Attribute list: " + metricName);

			return;
		}
	}

	@Override
	public void endElement(String uri, String localName, String qName) throws XMLParsingDoneException
	{
		// If we finished parsing the metric table, check if we have non-zero metrics recorded
		if (qName.equals("MetricTable"))
		{
			if (profileConstants.getPerfCounterTranslation().isEmpty())
			{
				log.error ("PerfExpert did not record any metrics. If '--thread' or '-t' option was passed to PerfExpert, was it correct?");
				throw new XMLParsingDoneException();
			}
		}

		// If it is a closing file element, reset the filename
		if(qName.equals("F"))
		{
			profile = null;
			log.debug("Found ending \"F\" element");
			filename = null;
			return;
		}

		// If it is a closing loaded-module element, reset the filename
		if(qName.equals("LM"))
		{
			profile = null;
			log.debug("Found ending \"LM\" element");
			loadedModule = null;
			return;
		}

		// If it is a procedure or a loop that has ended, reset the Profile object so that we don't (even by mistake) incorrectly record data
		if (qName.equals("P"))
		{
			log.debug("Found ending \"" + qName + "\" element");
			procedureStack.pop();

			profile = null;
			return;
		}

		if (qName.equals("L"))
		{
			log.debug("Found ending \"" + qName + "\" element");
			loopDepth--;

			profile = null;
			return;
		}

		// If ending C element, start processing again as normal
		if (qName.equals("C"))
		{
			profile = null;
			aggregateRecorded = true;
			// Should the following line be uncommented?
			// setAggregateCyclesFromRootProfile();

			log.debug("Found an ending \"C\" element, resuming parsing as normal");
			inCallSite = false;
			return;
		}
	}

	@Override
	public void endDocument()
	{
		// If there was only one useful metric in the input script, we would surely not have set the aggregate cycles,
		// which are used for calculation of importance, hence an additional call

		setAggregateCyclesFromRootProfile();
	}

	void setAggregateCyclesFromRootProfile()
	{
		if (profileList.size() == 0)
		{
			log.error("Aggregate cycles are being adjusted when there is nothing in the profile list");
			return;
		}

		if (profileConstants.getAggregateCycles() == 0)
		{
			// Adjust the aggregate cycles and recalculate importance of root profile (aggregate), which will be 1.0
			profileConstants.setAggregateCycles((long) profileList.get(0).getMetricBasedOnPEIndex(profileConstants.getIndexOfCycles()));

			HPCToolkitProfile rootProfile = profileList.get(0);
			rootProfile.setImportance(1.0);
		}

		// Check if the topmost item in the list of profiles is "important"
		// If not, chuck it out
		int lastIndex = profileList.size()-1;
		HPCToolkitProfile lastProfile = profileList.get(lastIndex);

		if (lastProfile.getImportance() == -1)
		{
			// Skip this one for now because its importance has not yet been calculated
			return;
		}

		if (lastProfile.getImportance() < threshold)
		{
			log.debug("Removing profile \"" + lastProfile.getCodeSectionInfo() + "\" because its importance is " + lastProfile.getImportance() + " and threshold is " + threshold);
			profileList.remove(lastIndex);
		}
	}

	// Formats the given information into a string of the form: [ in function foo()] at foobar.c:90
	String formatCodeSection(int lineNumber, boolean newProcedure)
	{
		// TODO: Needs some refactoring to leverage the modular methods for each component

		String location;
		String shortModuleName = loadedModule.substring(loadedModule.lastIndexOf('/')+1);
		String procedureName = procedureStack.isEmpty() ? null : procedureStack.peek();

		if (lineNumber == 0 && procedureName == null && (filename == null || filename.equals("~unknown-file~"))) 
			location = "~unknown-location~";
		else if (filename != null && !filename.equals("~unknown-file~") && lineNumber != 0)
			location = (newProcedure == false ? (procedureName != null ? " in function " + procedureName + (procedureName.contains("(") ? "" : "()") : "") : "") + " at " + filename + ":" + lineNumber;
		else location = (lineNumber != 0) ? " line " + lineNumber : "" + (newProcedure == false ? (procedureName != null ? " in function " + procedureName + (procedureName.contains("(") ? "" : "()") : "") : "") + (filename != null && !filename.equals("~unknown-file~") ? " in file \"" + filename + "\"" : "");

		return location; // + " in " + shortModuleName;
	}

	String formatFilename(String filename, int lineNumber)
	{
		if (lineNumber == 0 && isFilenameEmpty(filename))
			return null;
		else if (lineNumber != 0 && isFilenameEmpty(filename))
			return filename + ":" + lineNumber;

		return isFilenameEmpty(filename) ? formatLineNumber(lineNumber) : "in " + filename;  
	}

	String formatFunctionName(String functionName)
	{
		return "in function " + functionName + (functionName.contains("inlined") ? "" : "()");
	}

	String formatLineNumber(int lineNumber)
	{
		if (lineNumber != 0)
			return "at line " + lineNumber;

		return null;
	}

	boolean isFilenameEmpty(String filename)
	{
		return filename == null || filename.equals("~unknown-file~");
	}

	// So that we can retrieve all of the collected information at the end of the parsing
	List<HPCToolkitProfile> getProfileList()
	{
		return profileList;
	}
}
