
package edu.utexas.tacc.argp.options;

public class UnflaggedOption extends AOption implements IValuedOption
{
	String name;
	ArgType type;
	boolean mandatory;

	public UnflaggedOption(String name, String description, ArgType type)
	{
		super(description);
		this.name = name;
		this.type = type;
		this.mandatory = true;
	}

	public ArgType getType()
	{
		return type;
	}

	public String getName()
	{
		return name;
	}

	public boolean isMandatory()
	{
		return mandatory;
	}

	public void setMandatory(boolean mandatory)
	{
		this.mandatory = mandatory;
	}
}
