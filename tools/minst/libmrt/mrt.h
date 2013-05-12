
#ifndef	LIBMRT_H_
#define	LIBMRT_H_

#include <signal.h>

#include "record.h"

#define	DATANODE_SIZE	1024
#define	AWAKE_SEC		0

#ifndef	AWAKE_USEC
#define	AWAKE_USEC		500000
#endif

static long numCores = 0;
static __thread int coreID=-1;
static char szFilename[256]={0};
static volatile sig_atomic_t sleeping=0, access_count=0;
static int fd=-1, sleep_sec=0, new_sleep_sec=1, *intel_apic_mapping=NULL;
static node_t terminal_node;

void indigo__init_();
static void indigo__exit();

static inline int getCoreID();

static inline void signalHandler(int sig);
static inline void fill_struct(int read_write, int line_number, size_t p, int var_idx);

inline void indigo__record_c(int read_write, int line_number, void* addr, int var_idx);
inline void indigo__record_f_(int* read_write, int* line_number, void* addr, int* var_idx);

#endif	/* LIBMRT_H_ */
