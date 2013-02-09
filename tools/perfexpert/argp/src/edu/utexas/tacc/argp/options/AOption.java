
package edu.utexas.tacc.argp.options;

public abstract class AOption
{
	long ID;
	String description;
	static long count=0;

	public long getID()
	{
		return ID;
	}

	AOption(String description)
	{
		this.ID = count++;
		this.description = description;
	}

	public String getDescription()
	{
		return description;
	}
}
