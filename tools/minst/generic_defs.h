
#ifndef	GENERIC_DEFS_H_
#define	GENERIC_DEFS_H_

enum { FALSE=0, TRUE=1 };
enum { SUCCESS=0, ERR_PARAMS };
enum { TYPE_UNKNOWN=0, TYPE_READ, TYPE_WRITE, TYPE_READ_AND_WRITE };
enum { ACTION_INSTRUMENT=0, ACTION_SPLIT_LOOP };

typedef	unsigned char	BOOL;

typedef struct
{
	char* function;
} function_info_t;

typedef struct
{
	char* function;
	int line_number;
} loop_info_t;

typedef struct
{
	short action;
	loop_info_t loopInfo;
	function_info_t functionInfo;
} options_t;

class attrib
{
	public:
	char* inst_func;
	int line_number;
	BOOL read, skip, atomic;

	attrib(BOOL _read, BOOL _skip, char* _inst_func, int _line_number)
	{
		atomic=false, read=_read, skip=_skip, inst_func=_inst_func, line_number=_line_number;
	}
};

class attrib_temp
{
	public:
	char* inst_func;
	int line_number;
	BOOL read, skip;

	SgNode* currNode;
	std::string var_name;

	attrib_temp(BOOL _read, BOOL _skip, char* _inst_func, int _line_number)
	{
		var_name="", currNode=NULL, read=_read, skip=_skip, inst_func=_inst_func, line_number=_line_number;
	}
};

#endif	/* GENERIC_DEFS_H_ */
