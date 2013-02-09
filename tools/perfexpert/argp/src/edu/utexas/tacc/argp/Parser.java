
package edu.utexas.tacc.argp;

import java.util.Map;
import java.util.List;
import java.util.Arrays;
import java.util.HashMap;
import java.util.ArrayList;

import edu.utexas.tacc.argp.options.AOption;
import edu.utexas.tacc.argp.options.ArgType;
import edu.utexas.tacc.argp.options.INamedOption;
import edu.utexas.tacc.argp.options.IValuedOption;
import edu.utexas.tacc.argp.options.FlaggedOption;
import edu.utexas.tacc.argp.options.UnflaggedOption;

public class Parser
{
	Map<Long, Object> arguments;
	List<AOption> registeredNamedOptions;
	List<AOption> registeredUnnamedOptions;

	public String getUsage(String appname)
	{
		StringBuilder sb = new StringBuilder();
		sb.append("USAGE: " + appname);

		if (registeredNamedOptions != null)
			sb.append(" [OPTIONS]");

		if (registeredUnnamedOptions != null)
		{
			for (AOption o : registeredUnnamedOptions)
			{
				UnflaggedOption u = (UnflaggedOption) o;
				sb.append(" " + (u.isMandatory() ? "<" + u.getName() + ">" : "[<" + u.getName() + ">]"));
			}
		}

		sb.append("\n\n");

		if (registeredNamedOptions != null)
		{
			for (AOption o : registeredNamedOptions)
			{
				INamedOption n = (INamedOption) o;
				StringBuilder lb = new StringBuilder();

				lb.append(" ");
				if (n.takesValue())
				{
					FlaggedOption f = (FlaggedOption) o;
					if(n.getShortName() != null && n.getLongName() != null)
					{
						lb.append("-" + f.getShortName());
						if (f.isValueMandatory())	lb.append("=<" + f.getValueName() + ">");
						else				lb.append("[=<" + f.getValueName() + ">]");

						lb.append(",--" + f.getLongName());
						if (f.isValueMandatory())	lb.append("=<" + f.getValueName() + ">");
						else				lb.append("[=<" + f.getValueName() + ">]");
					} else if (f.getShortName() != null)
					{
						lb.append("-" + f.getShortName());
						if (f.isValueMandatory())	lb.append("=<" + f.getValueName() + ">");
						else				lb.append("[=<" + f.getValueName() + ">]");
					} else
					{
						lb.append("--" + f.getLongName());
						if (f.isValueMandatory())	lb.append("=<" + f.getValueName() + ">");
						else				lb.append("[=<" + f.getValueName() + ">]");
					}
				}
				else
				{
					if(n.getShortName() != null && n.getLongName() != null)
						lb.append("-" + n.getShortName() + ",--" + n.getLongName());
					else if (n.getShortName() != null)
						lb.append("-" + n.getShortName());
					else
						lb.append("--" + n.getLongName());
				}

				int l=lb.length();
				for (int i=0; i<40-l; i++)	lb.append(" ");

				lb.append(o.getDescription() + "\n");
				sb.append(lb);
			}
		}

		sb.append("\n");
		if (registeredUnnamedOptions != null)
		{
			for (AOption o : registeredUnnamedOptions)
			{
				StringBuilder lb = new StringBuilder();
				UnflaggedOption u = (UnflaggedOption) o;
				lb.append(" " + (u.isMandatory() ? "<" + u.getName() + ">" : "[<" + u.getName() + ">]"));

				int l=lb.length();
				for (int i=0; i<40-l; i++)	lb.append(" ");

				lb.append(o.getDescription() + "\n");
				sb.append(lb);
			}
		}

		return sb.toString();
	}

	public boolean isSet(AOption o)
	{
		if (arguments != null && arguments.containsKey(o.getID()))
			return true;

		return false;
	}

	public Object getValue(AOption o)
	{
		if (arguments != null && arguments.containsKey(o.getID()))
			return arguments.get(o.getID());

		return null;
	}

	public void registerOption(AOption o)
	{
		if (o instanceof UnflaggedOption)
		{
			if (registeredUnnamedOptions == null)
				registeredUnnamedOptions = new ArrayList<AOption>();

			registeredUnnamedOptions.add(o);
		}
		else
		{
			if (registeredNamedOptions == null)
				registeredNamedOptions = new ArrayList<AOption>();

			registeredNamedOptions.add(o);
		}
	}

	public void parse(String args[]) throws ArgPException
	{
		parse(implode(args, " "));
	}

	private String implode(String args[], String glue)
	{
		StringBuilder sb = new StringBuilder();

		if (args.length >= 1)
			sb.append(args[0]);

		for (int i=1; i<args.length; i++)
			sb.append(glue + args[i]);

		return sb.toString();
	}

	public void parse(String cmdLine) throws ArgPException
	{
		if (cmdLine == null)
			return;

		List<AOption> copyList=null;
		if (registeredUnnamedOptions != null)
			copyList = new ArrayList<AOption>(registeredUnnamedOptions);

		// Sanitize the input
		// 01: Replace tabs with spaces
		cmdLine.replace("\t", " ");

		// 02: Replace "[ ]+" with " "
		cmdLine.replaceAll("[' ']+", " ");

		// 03: Replace "[ ]?=[ ]?" with "="
		cmdLine.replaceAll("[' ']?=[' ']?", "=");

		arguments = new HashMap<Long, Object>();
		List<String> positionalParams = new ArrayList<String>();
		List<String> tokens = Arrays.asList(cmdLine.split(" "));

		for (String s : tokens)
		{
			if (s == null || s.length() == 0)	continue;

			if (s.startsWith("-"))
				processNamedOption(s);
			else	// Unflagged option
				processUnnamedOption(s, copyList);
		}

		// Check if there are any mandatory unflagged options remaining to be processed
		if (copyList != null)
			for (AOption o : copyList)
				if (o != null)
					if (((UnflaggedOption) o).isMandatory())
						throw new ArgPException("Argument empty for unflagged option: " + o.getDescription());
	}

	private void processUnnamedOption(String str, List<AOption> copyList) throws ArgPException
	{
		if (copyList != null)
		{
			for (AOption o : copyList)
			{
				if (o != null)
				{
					saveArgumentByType(null, str, o.getID(), ((IValuedOption) o).getType());

					// Positional parameter consumed, so remove it from the registered parameters
					// XXX: Destructive
					copyList.remove(o);
					return;
				}
			}
		}

		// Unrecognized option
		throw new ArgPException("Unrecognized argument: " + str);
	}

	private void processNamedOption(String str) throws ArgPException
	{
		if (registeredNamedOptions == null)
			throw new ArgPException("Unrecognized argument: " + str);

		int vIndex = str.indexOf("=");
		boolean longOption = str.startsWith("--");

		if (!longOption)
		{
			// Check if we have multiple clubbed short options
			if (str.length() > 2)	// '-' and the option char
			{
				if (str.charAt(2) != '=')
				{
					processNamedOption("-" + str.charAt(1));
					processNamedOption("-" + str.substring(2));
					return;
				}
			}
		}

		String optionString=null, valueString=null;
		optionString = str.substring(longOption ? 2 : 1, vIndex==-1 ? str.length() : vIndex);
		valueString  = str.substring(vIndex==-1 ? str.length() : vIndex+1, str.length());

		// Find which `AOption' does this belong to
		AOption o = searchOptions(optionString, longOption);
		if (o == null)	// Unrecognized option
			throw new ArgPException("Unrecognized argument: " + str);

		if (((INamedOption) o).takesValue())
		{
			ArgType type = ((IValuedOption) o).getType();
			saveArgumentByType(optionString, valueString, o.getID(), type);
		}
		else
		{
			// Check that we don't have any value passed to this option
			if (valueString != null && valueString.length() != 0)
				throw new ArgPException("Option '" + (optionString.length() == 1 ? "-" + optionString : "--" + optionString) + "' does not accept value '" + valueString + "'");
			arguments.put(o.getID(), null);
		}
	}

	private AOption searchOptions(String optionString, boolean longOption)
	{
		INamedOption n=null;
		for (AOption o : registeredNamedOptions)
		{
			n = (INamedOption) o;
			if ((!longOption && n.getShortName().equals(optionString.charAt(0))) || (longOption && n.getLongName().equals(optionString)))
				return o;
		}

		return null;
	}

	private String errMsg(String option, String value)
	{
		return "Invalid value '" + value + "'" + (option != null ? " passed to" + (option.length() == 1 ? "-" + option : "--" + option) : "");
	}

	private void saveArgumentByType(String option, String value, long ID, ArgType type) throws ArgPException
	{
		// Short cut
		if (value == null || value.length() == 0)
		{
			arguments.put(ID, null);
			return;
		}

		switch (type)
		{
			case CHARACTER:
				Character value_c = value.charAt(0);
				arguments.put(ID, value_c);
				break;

			case SHORT:
				Short value_s = null;

				try {
					value_s = Short.parseShort(value);
				}
				catch (NumberFormatException e)	{
					throw new ArgPException(errMsg(option, value));
				}

				arguments.put(ID, value_s);
				break;

			case INTEGER:
				Integer value_i = null;

				try {
					value_i = Integer.parseInt(value);
				}
				catch (NumberFormatException e) {
					throw new ArgPException(errMsg(option, value));
				}

				arguments.put(ID, value_i);
				break;

			case LONG:
				Long value_l = null;

				try {
					value_l = Long.parseLong(value);
				}
				catch (NumberFormatException e) {
					throw new ArgPException(errMsg(option, value));
				}

				arguments.put(ID, value_l);
				break;

			case FLOAT:
				Float value_f = null;

				try {
					value_f = Float.parseFloat(value);
				}
				catch (NumberFormatException e) {
					throw new ArgPException(errMsg(option, value));
				}

				arguments.put(ID, value_f);
				break;

			case DOUBLE:
				Double value_d = null;

				try {
					value_d = Double.parseDouble(value);
				}
				catch (NumberFormatException e) {
					throw new ArgPException(errMsg(option, value));
				}

				arguments.put(ID, value_d);
				break;

			case BOOLEAN:
				Boolean value_b = false;
				if (value.toLowerCase().equals("true") || value.equals("1"))
					value_b = true;

				arguments.put(ID, value_b);
				break;

			case STRING:
				String value_str = value;
				arguments.put(ID, value_str);
				break;
		}
	}
}
