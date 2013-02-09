
package edu.utexas.tacc.argp.options;

public interface INamedOption
{
	public Character getShortName();
	public String getLongName();
	public boolean takesValue();
}
