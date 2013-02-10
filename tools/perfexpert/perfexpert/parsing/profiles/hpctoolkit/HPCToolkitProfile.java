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

import java.text.DecimalFormat;
import java.text.ParseException;

import org.apache.log4j.Logger;

import edu.utexas.tacc.perfexpert.parsing.profiles.AProfile;

public class HPCToolkitProfile extends AProfile
{
	Logger log = Logger.getLogger( HPCToolkitProfile.class );

	private HPCToolkitProfileConstants profileConstants;

	private DecimalFormat doubleFormat = new DecimalFormat("#.###");
	int LCPICount = 10;

	// Used for measuring the amount of variation
	double maxInstructions = Double.MIN_VALUE, minInstructions = Double.MAX_VALUE;

	double importance = -1;
	double [] perfValues;
	double [] lcpiValues;
	int [] counts;	// For averages
	int loopDepth = 0;
	
	public HPCToolkitProfile(HPCToolkitProfileConstants profileConstants)
	{
		this.profileConstants = profileConstants;
		perfValues = new double [profileConstants.discoveredMetrics];
		counts = new int [profileConstants.discoveredMetrics];
		LCPICount = profileConstants.lcpiTranslation.size();
		lcpiValues = new double [LCPICount];
	}

	public void setLoopDepth(int loopDepth)
	{
		this.loopDepth = loopDepth;
	}

	public int getLoopDepth()
	{
		return this.loopDepth;
	}

	public void setLCPI(int index, double value)
	{
		if (index > LCPICount)
			return;

		lcpiValues[index] = value;
	}

	public double getLCPI(int index)
	{
		if (index > LCPICount)
			return 0;

		return roundToTwoDigitsAfterDecimal(lcpiValues[index]);
	}

	@Override
	public double getMetric(String key)
	{
		Integer PEIndex = profileConstants.perfCounterTranslation.get(key);
		if (PEIndex == null)
		{
			// The metric does not exist
			return 0;
		}

		return getMetricBasedOnPEIndex(PEIndex);
	}

	public double getMetric(int HPCToolkitIndex)
	{
		int PEIndex = profileConstants.HPCToPETranslation.get(HPCToolkitIndex);
		return getMetricBasedOnPEIndex(PEIndex);
	}
	
	public double getMetricBasedOnPEIndex(int PEIndex)
	{
		if (counts[PEIndex] == 0 || perfValues[PEIndex] == 0)
			return 0;
		
		return roundToTwoDigitsAfterDecimal(perfValues[PEIndex]/counts [PEIndex]);
	}

	@Override
	public void setMetric(String key, double value)
	{
		int peIndex = profileConstants.perfCounterTranslation.get(key);
		setMetricBasedOnPEIndex(peIndex, value);
	}

	public void setMetric(int HPCToolkitIndex, double value)
	{
		// Do a quick sanity check
		if (profileConstants.HPCToPETranslation.containsKey(HPCToolkitIndex))
		{
			int peIndex = profileConstants.HPCToPETranslation.get(HPCToolkitIndex);
			setMetricBasedOnPEIndex(peIndex, value);
		}
	}
	
	private void setMetricBasedOnPEIndex(int PEIndex, double value)
	{
		if (counts[PEIndex] != 0)
		{
			// Do a quick check if this value should be counted
			double average = perfValues[PEIndex]/counts[PEIndex];

			double ratio = 1;
			if (value != 0)	ratio = average / value;

			if (ratio < 0.3)
			{
				// Erase other values and use this one from fresh
				perfValues[PEIndex] = value;
				counts[PEIndex] = 1;

				if (PEIndex == profileConstants.indexOfInstructions)
				{
					// Adjust min and max
					minInstructions = value;
					maxInstructions = value;
				}
			}
			else if (ratio > 3)
			{
				; // Ignore this value because our average is higher
				return;
			}
			else
			{
				perfValues[PEIndex] += value;
				counts[PEIndex]++;

				if (PEIndex == profileConstants.indexOfInstructions)
				{
					// Adjust min and max
					if (minInstructions > value)	minInstructions = value;
					if (maxInstructions < value)	maxInstructions = value;
				}
			}
		}
		else
		{
			perfValues[PEIndex] += value;
			counts[PEIndex]++;

			if (PEIndex == profileConstants.indexOfInstructions)
			{
				// Adjust min and max
				if (minInstructions > value)	minInstructions = value;
				if (maxInstructions < value)	maxInstructions = value;
			}
		}
		
		if (PEIndex == profileConstants.indexOfCycles)
		{
			// Set importance
			if (profileConstants.aggregateCycles != 0)
				importance = (perfValues[PEIndex]/counts[PEIndex]) / ((double) profileConstants.aggregateCycles);
			else
			{
				// If everything is correct, this is the record of the aggregate execution
				// Unfortunately, there is no way to distinguish within this limited context
				// So as a safe measure, we just ignore, which is what we would do if everything were right!
			}
		}
	}
	
	public HPCToolkitProfileConstants getConstants()
	{
		return profileConstants;
	}
	
	public double getVariation()
	{
		if (maxInstructions == Double.MIN_VALUE)	// Value was never set
			return 0;

		return roundToTwoDigitsAfterDecimal((maxInstructions-minInstructions)/maxInstructions);
	}
	
	public double getImportance ()
	{
		return roundToTwoDigitsAfterDecimal(importance);
	}
	
	public void setImportance(double importance)
	{
		this.importance = importance;
	}

	// To maintain consistency of results with Perl version
	private double roundToTwoDigitsAfterDecimal(double in)
	{
		try
		{
			return doubleFormat.parse(doubleFormat.format(in)).doubleValue();
		} catch(ParseException e)
		{
			log.error("Failed to parse while rounding \"" + in + "\" to two digits, defaulting to zero");
			return 0;
		}
	}
}
