
package edu.utexas.tacc.argp.options;

public class Switch extends AOption implements INamedOption
{
	String longName;
	Character shortName;

	public Switch(Character shortName, String longName, String description)
	{
		super(description);
		if (shortName != null || longName != null)
		{
			this.shortName = shortName;
			this.longName = longName;
		}
	}

	public Character getShortName()
	{
		return shortName;
	}

	public String getLongName()
	{
		return longName;
	}

	public boolean takesValue()
	{
		return false;
	}
}
