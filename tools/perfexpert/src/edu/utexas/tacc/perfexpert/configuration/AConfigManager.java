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

package edu.utexas.tacc.perfexpert.configuration;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.apache.log4j.Logger;

public abstract class AConfigManager
{
	protected String sourceURI = null;
	protected LinkedProperties props = new LinkedProperties();
	Logger log = Logger.getLogger( AConfigManager.class );
	
	AConfigManager()
	{
		;
	}
	
	AConfigManager(String sourceURI)
	{
		this.sourceURI = sourceURI;
	}
	
	// Standard file:// or data:// -based URI
	public void setSourceURI(String sourceURI)
	{
		this.sourceURI = sourceURI;
	}
	
	public Properties getProperties()
	{
		return props;
	}
	
	// To parse the configuration source now
	public boolean readConfigSource()
	{
		int fileIndex = sourceURI.indexOf("://");
		if (sourceURI.substring(0, fileIndex).equals("file"))
		{
			String filename = sourceURI.substring("file://".length());
			try
			{
				props.load(new FileInputStream(filename));
			}
			catch (FileNotFoundException e)
			{
				log.error("Configuration not found in: " + sourceURI);
				return false;
			}
			catch (IOException e)
			{
				log.error("Error loading configuration from " + sourceURI);
				e.printStackTrace();
				return false;
			}
			
			return true;
		}
		else
		{
			log.error("Unsupported URI type for configuration file in \"" + sourceURI + "\"");
			return false;
		}
	}
	
	// Since the manner in which the configuration is saved could change,
	// we version it.
	public String getVersion()
	{
		// Default to 1.0
		return props.getProperty("version", "1.0"); 
	}
}
