
#ifndef	ARGP_H_
#define	ARGP_H_

#include <argp.h>
     
struct argp_option options[5] =
{
	{ "debug", 'd', NULL, 0, "Output debug information", 0 },
	{ "pretty-print", 'p', NULL, 0, "Print output in a more friendly format", 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct arg_info
{
	bool pprint;
	char *location;
	short showDebug;
};

static error_t parse_opt(int key, char* arg, struct argp_state *state)
{
	struct arg_info* info = (struct arg_info*) state->input;

	switch(key)
	{
		case 'p':	info->pprint = true;		break;
		case 'd':	info->showDebug = true;		break;

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
