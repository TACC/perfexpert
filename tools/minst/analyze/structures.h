
#ifndef	STRUCTURES_H_
#define	STRUCTURES_H_

#define NUM_CORES_MAX		64
#define	MAX_LEADER_RECORDS	3

#include <ext/hash_map>

#include "cache_config.h"
#include "record.h"

#define	STRING_LEN	32
#define CHUNK_SIZE	10240
#define	EXT_VAR_LEN	(2*VAR_LENGTH+8)

#include <ext/hash_map>

// #include <google/sparse_hash_set>
#include <google/sparse_hash_map>
using namespace __gnu_cxx;

struct eqstr
{
	bool operator()(const char* k1, const char* k2) const
	{
		return k1 == k2 || (k1 && k2 && strcmp(k1, k2) == 0);
	}
};

struct eqlng
{
	bool operator()(const long k1, const long k2) const
	{
		return k1 == k2;
	}
};

struct eqsize_t
{
	bool operator()(const size_t k1, const size_t k2) const
	{
		return k1 == k2;
	}
};

/* AVL declarations */
typedef struct
{
	int lineNumber; long ts; size_t address; unsigned short coreID; short core_home;
	long l1_child_count[NUM_L1], l2_child_count[NUM_L2], l3_child_count[NUM_L3];
	unsigned short type_access:2, local_var:2;
	unsigned long vIndex, fIndex;
	// char var_name[EXT_VAR_LEN];
	// char func_name[FUNC_LENGTH];
} mynode;

static int valcmp(void* i1, void* i2, NODE n)
{
	return (long) ((mynode*) i1)->ts - (long) ((mynode*) i2)->ts;
}

static unsigned int nodesize(void* i)
{
	return  sizeof(mynode);
}

/* Reuse records declarations */
typedef struct tagVarList
{
	unsigned long vIndex;
	size_t last_access[NUM_CORES_MAX];
	unsigned long t_l[3];
	google::sparse_hash_map<long, long, hash<long>, eqlng> reuse_count[3];	// {reuse,count}

	google::sparse_hash_map<const char*, long, hash<const char*>, eqstr> reuse_hash;
	google::sparse_hash_map<long, long, hash<long>, eqlng> stride_hash;

	struct tagVarList* next;
} var_list;

typedef struct tagChunk
{
	mynode nodes[CHUNK_SIZE];
	int fillCount;

	struct tagChunk* next;
} chunk;

typedef struct tagMetrics
{
	long program_distance, program_count;
	double program_latency;
	double l1_latency_count;
	double l2_latency_count;
	double l3_latency_count;
	double mem_latency_count;

	unsigned long l1_hit, l1_miss;
	unsigned long l2_hit, l2_miss;
	unsigned long numa_hit, numa_miss;
	long copyInSizeMin, copyInSizeMax;
	long l1_reuse[3], l2_reuse[3], l3_reuse[3];
	long l1_reuse_count[3], l2_reuse_count[3], l3_reuse_count[3];
} metrics;

#define MAX_STRIDE_VALUES	5
typedef struct tagVarMetrics
{
	// char var_name[EXT_VAR_LEN];
	unsigned long vIndex;
	double tot_dist, tot_lat;
	long tot_count, dominant_stride[MAX_LEADER_RECORDS], stride_count[MAX_LEADER_RECORDS];
	google::sparse_hash_map<long, long, hash<long>, eqlng> stride_map;
	unsigned long l1_hit, l1_miss;
	unsigned long l2_hit, l2_miss;
	unsigned long numa_hit, numa_miss;

	long reuse[3];
	long reuse_count[3];

	struct tagVarMetrics* next;
} varMetrics;

typedef struct tagSet
{
	int residue;
	unsigned short attempts;
} set;

inline short getCommonCacheLevel(int id1, int id2, short type1, short type2)
{
	// Reads happen via L1 (if distance is not very high)
	if (type1 == type2 && type1 == TYPE_READ)	return 1;

	// If any one of them is a write, go through the usual checks
	if (id1 == id2)	return 1;		// L1 cache
	if (id1>>1 == id2>>1)	return 2;	// L2
	if (id1>>2 == id2>>2)	return 3;	// L3

	return 4;				// memory
}

#endif	/* STRUCTURES_H_ */
