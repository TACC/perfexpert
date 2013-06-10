
#include <rose.h>
#include <stdio.h>

#include "macpo_record.h"
#include "minst.h"

int parseMinstArgument(char* arg, options_t& options)
{
	char* eq = strchr(arg, '=');

	char opt[16];
	char* colon = strchr(arg, ':');
	assert (colon && *(++colon) && "Malformed option string");

	if (!eq)	eq = arg + strlen(arg);

	strncpy(opt, colon, eq-colon);
	opt[eq-colon] = '\0';

	// Do something with 'opt' and 'val'
	if (strcmp(opt, "function") == 0)
	{
		if (!eq || !*(++eq))	return -ERR_PARAMS;	// No '=' sign
		options.functionInfo.function = eq;
	}
	else if (strcmp(opt, "loop") == 0)
	{
		if (!eq || !*(++eq))	return -ERR_PARAMS;	// No '=' sign

		// Split the value into <function>:<line#>
		char* function = eq;
		char* lineNum = strchr(eq, ':');
		if (lineNum == NULL || !*(++lineNum))
			return -ERR_PARAMS;

		// XXX: UNSAFE! Modifying the argument list itself but better than allocating handful of bytes and having to free it later
		*(lineNum-1) = '\0';

		options.loopInfo.function = function;
		options.loopInfo.line_number = atoi(lineNum);
	}
	else if (strcmp(opt, "instrument") == 0)
		options.action = ACTION_INSTRUMENT;
	else if (strcmp(opt, "loopsplit") == 0)
		options.action = ACTION_SPLIT_LOOP;
	else if (strcmp(opt, "noinst") == 0)
		options.action = ACTION_NONE;
	else	return -ERR_PARAMS;

	return SUCCESS;
}

int main (int argc, char *argv[])
{
	options_t options={0};

	// Default action is to instrument the code
	options.action = ACTION_INSTRUMENT;

	std::vector<std::string> arguments;
	arguments.push_back(argv[0]);

	for (int i=1; i<argc; i++)
	{
		if (strstr(argv[i], "--macpo:") == argv[i])	// If "--macpo:" was found at the start of argv[i]
		{
			if (parseMinstArgument(argv[i], options) == -ERR_PARAMS)
			{
				fprintf (stdout, "Unknown parameter: %s, aborting...\n", argv[i]);
				return -ERR_PARAMS;
			}
		}
		else	arguments.push_back(argv[i]);
	}

	// Check if we have explicit instructions not to change anything
	if (options.action != ACTION_NONE)
	{
		// Check if at least a function or a loop was specified on the command line
		if (!options.functionInfo.function && !options.loopInfo.function)
		{
			fprintf (stderr, "USAGE: %s <options>\n", argv[0]);
			fprintf (stderr, "Did not find valid options on the command line\n");
			return -ERR_PARAMS;
		}

		char* inst_function = options.loopInfo.function ? options.loopInfo.function : options.functionInfo.function;

		SgProject *project = frontend (arguments);
		ROSE_ASSERT (project != NULL);

		// Initializing attributes
		attrib attr(TYPE_UNKNOWN, FALSE, inst_function, options.loopInfo.line_number);

		// Loop over each file
		SgFilePtrList files = project->get_fileList();
		for (SgFilePtrList::iterator it=files.begin(); it!=files.end(); it++)
		{
			SgSourceFile* file = isSgSourceFile(*it);
			std::string filename = file->get_file_info()->get_filenameString();
			std::string basename = filename.substr(filename.find_last_of("/"));

			// Start the traversal!
			MINST traversal (options.action, options.loopInfo.line_number, inst_function);
			traversal.traverseWithinFile (file, attr);
		}

		// FIXME: ROSE tests seem to be broken, operand of AddressOfOp is not allowed to be an l-value
		// AstTests::runAllTests (project);
		return backend (project) == 0 ? 0 : 1;
	}
	else
	{
		SgProject *project = frontend (arguments);
		return backend (project) == 0 ? 0 : 1;
	}
}
