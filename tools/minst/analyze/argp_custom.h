
#ifndef	ARGP_H_
#define	ARGP_H_

#include <argp.h>
     
struct argp_option options[6] =
{
	{ "vars", 'v', NULL, 0, "Print reuse distances of variables", 0 },
	{ "stride", 's', NULL, 0, "Analyze strides of variables", 0 },
	{ "debug", 'd', NULL, 0, "Output debug information", 0 },
	{ "callgraph", 'c', "DOT_FILE", 0, "Specify DOT file containing call graph", 0 },
	{ "function", 'f', "FUNCTION", 0, "Analyze FUNCTION and its children", 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct arg_info
{
	char *location, *callgraph, *function;
	short showVars, showDebug, showStride;
};

static error_t parse_opt(int key, char* arg, struct argp_state *state)
{
	struct arg_info* info = (struct arg_info*) state->input;

	switch(key)
	{
		case 'c':	info->callgraph = arg;	break;
		case 'f':	info->function = arg;	break;
		case 'v':	info->showVars = 1;	break;
		case 'd':	info->showDebug = 1;	break;
		case 's':	info->showStride = 1;	break;

		case ARGP_KEY_ARG:
			if (state->arg_num >= 1)
				argp_usage(state);

			info->location = arg;
			break;

		case ARGP_KEY_END:
			if (state->arg_num < 1)
				argp_usage(state);

			break;

		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

struct argp argp = { options, parse_opt, "reuser.out", "Program to process reuse distances", 0, 0, 0 };

#endif /* ARGP_H_ */
