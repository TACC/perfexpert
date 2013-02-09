
package edu.utexas.tacc.argp;

import java.text.ParseException;

public class ArgPException extends ParseException
{
	// XXX: Don't know if this will completely fix the problem
	// although it makes the warning go away

	public static final long serialVersionUID = 24362462L;
	public ArgPException(String msg)
	{
		super(msg, 0);
	}

	public ArgPException(String msg, int offset)
	{
		super(msg, offset);
	}
}
