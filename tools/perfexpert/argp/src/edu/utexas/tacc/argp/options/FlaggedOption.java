
package edu.utexas.tacc.argp.options;

import edu.utexas.tacc.argp.options.ArgType;

public class FlaggedOption extends AOption implements INamedOption, IValuedOption
{
	ArgType type;
	Character shortName;
	boolean mandatoryValue;
	String longName, valueName;

	public FlaggedOption(Character shortName, String longName, String description, String valueName, boolean mandatoryValue, ArgType type)
	{
		super(description);

		if (shortName != null || longName != null)
		{
			this.type = type;
			this.longName = longName;
			this.shortName = shortName;
			this.valueName = valueName;
			this.mandatoryValue = mandatoryValue;
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

	public ArgType getType()
	{
		return type;
	}

	public boolean takesValue()
	{
		return true;
	}

	public String getValueName()
	{
		return valueName;
	}

	public boolean isValueMandatory()
	{
		return mandatoryValue;
	}
}
