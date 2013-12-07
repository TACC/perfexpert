/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 *
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * This file is part of PerfExpert.
 *
 * PerfExpert is free software: you can redistribute it and/or modify it under
 * the terms of the The University of Texas at Austin Research License
 * 
 * PerfExpert is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.
 * 
 * Authors: Leonardo Fialho and Ashay Rane
 *
 * $HEADER$
 */

#ifndef	ARGP_H_
#define	ARGP_H_

#include <argp.h>

struct argp_option options[4] =
{
	{ "debug", 'd', NULL, 0, "Output debug information", 0 },
	{ "iamabot", 'b', NULL, 0, "Print output in an easy-to-parse format", 0 },
	{ "stream-names", 's', NULL, 0, "Print all streams in the output, even if there are more than 5 streams", 0 },
	{ 0, 0, 0, 0, 0, 0 }
};

struct arg_info
{
	float threshold;
	char *arg1, *arg2, *location;
	bool bot, showDebug, stream_names;
};

static error_t parse_opt(int key, char* arg, struct argp_state *state)
{
	struct arg_info* info = (struct arg_info*) state->input;

	switch(key)
	{
		case 'b':	info->bot = true;		break;
		case 'd':	info->showDebug = true;		break;
		case 's':	info->stream_names = true;		break;

		case ARGP_KEY_ARG:
			if (state->arg_num >= 2)
				argp_usage(state);

			state->arg_num == 0 ? info->arg1 = arg : info->arg2 = arg;
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

struct argp argp = { options, parse_opt, "[threshold] macpo.out", "Program to process reuse distances", 0, 0, 0 };

#endif /* ARGP_H_ */
