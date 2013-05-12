
#ifndef COMMON_H_
#define COMMON_H_

enum { MSG_TERMINAL=0, MSG_MEM_INFO };

typedef struct
{
	unsigned short read_write:2;
	size_t address;
	unsigned long var_idx;
	int line_number;
} mem_info_t;

typedef struct
{
	unsigned short coreID;
	unsigned short type_message;
	mem_info_t mem_info;
} node_t;

#endif
