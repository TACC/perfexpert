
#ifndef	LIBMRT_H_
#define	LIBMRT_H_

/* Trying to recreate namespaces in C */
/* Based from: http://stackoverflow.com/questions/389827/namespaces-in-c */

typedef struct
{
	int (*const record) (void*);
} __indigo_namespace_struct;

extern __indigo_namespace_struct const __indigo;

#endif	/* LIBMRT_H_ */
