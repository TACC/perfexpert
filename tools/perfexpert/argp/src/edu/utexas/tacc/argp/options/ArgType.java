
package edu.utexas.tacc.argp.options;

public enum ArgType
{
        CHARACTER(0), STRING(1), SHORT(2), INTEGER(3), LONG(4), FLOAT(5), DOUBLE(6), BOOLEAN(6);
	private int value;

	ArgType(int value)
	{
		this.value = value;
	}
}
