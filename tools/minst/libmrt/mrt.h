
#ifndef	LIBMRT_H_
#define	LIBMRT_H_

/* Trying to recreate namespaces in C */
/* Based from: http://stackoverflow.com/questions/389827/namespaces-in-c */

typedef struct
{
	int (*const record) (void*);
} __indigo_namespace_struct;

extern __indigo_namespace_struct const __indigo;

#if 0
class __indigo
{
	public:
	enum { SUCCESS=0, ERR_PARAMS, ERR_MEM };

	static int record(void*);
};
#endif

#endif	/* LIBMRT_H_ */
