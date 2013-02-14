/* 
 * RangeParser.java - a simple number expression parser
 * 
 * Copyright (c) 2010 Michael Schierl
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *   
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *   
 * - Neither name of the copyright holders nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *   
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND THE CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package edu.utexas.tacc.perfexpert.misc;

import java.util.Arrays;
import java.util.ArrayList;

public class RangeParser
{
	public static final int MAX_THREADS = 128;
	public static String getRegexString(String pattern)
	{
		if (pattern == null || pattern.length() == 0)
			return "(\\d+,)?([\\d]+)";

		StringBuilder regex = new StringBuilder("(\\d+,)?(");
		String[] parts = pattern.toLowerCase().split(",",-1);
		int min = Integer.MAX_VALUE, max = 0;

		for (int i=0; i<parts.length; i++)
		{
			String part = parts[i];

			try
			{
				if (part.matches("\\d+"))
				{
					int value = Integer.parseInt(part);
					if (min > value)	min = value;
					if (max < value)	max = value;

					regex.append(value + "|");
				}
				else if (part.matches("\\d*-\\d*"))
				{
					String[] limits = part.split("-", -1);
					int from = limits[0].length() == 0 ? 0 : Integer.parseInt(limits[0]);
					int to = limits[1].length() == 0 ? MAX_THREADS : Integer.parseInt(limits[1]);

					if (to < from)	throw new IllegalArgumentException("Invalid pattern: " + part);
					if (min > from)	min = from;
					if (max < to)	max = to;

					for (int x=from; x<=to; x++)
						regex.append(x + "|");
				}
				else	throw new IllegalArgumentException("Invalid pattern: " + part);
			}
			catch (NumberFormatException ex)
			{
				throw new IllegalArgumentException("Invalid pattern: " + part);
			}
		}

		regex.replace(regex.length()-1, regex.length(), ")");
		return regex.toString();
	}
}
