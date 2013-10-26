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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <math.h>
#include <sys/stat.h>
#include <set>
#include <map>
#include <list>

#include <omp.h>

#include <ext/hash_map>

#include "macpo_record.h"

#include "avl.h"
#include "avl_code.h"
#include "argp_custom.h"
#include "structures.h"

#include "cache_config.h"

#include "set-associative-mapping.h"

#define	STREAM_PRINT_THRESHOLD	5

typedef std::pair<long,long> stride_info_t;
typedef std::vector<stride_info_t> stride_list_t;

omp_lock_t var_lock;
varMetrics* varMetricsHead = NULL;

std::map<long, std::string> var_idx;
metadata_info_t metadata_info={0};

bool sort_function(stride_info_t v1, stride_info_t v2)
{
	// Compare distance values
	// Save in descending order
	return v1.second > v2.second;
}

void recordHome (bool debugFlag, var_list** listHead, mynode* new_access)
{
	var_list* listTraversal = *listHead;

	while (listTraversal && listTraversal->vIndex != new_access->vIndex)
		listTraversal = listTraversal->next;

	if (listTraversal)
	{
		// Home for this data structure has already been recorded
		return;
	}

	// Add one to the beginning of the list
	var_list* temp = (var_list*) calloc (sizeof (var_list), 1);
	if (!temp)	assert(0);

	temp->vIndex = new_access->vIndex;
	temp->reuse_hash = google::sparse_hash_map<const char*, long, hash<const char*>, eqstr> ();
	temp->reuse_hash.set_deleted_key(NULL);

	temp->reuse_count[0] = google::sparse_hash_map<long, long, hash<long>, eqlng>();
	temp->reuse_count[1] = google::sparse_hash_map<long, long, hash<long>, eqlng>();
	temp->reuse_count[2] = google::sparse_hash_map<long, long, hash<long>, eqlng>();

	temp->reuse_count[0].set_deleted_key(0);
	temp->reuse_count[1].set_deleted_key(0);
	temp->reuse_count[2].set_deleted_key(0);

	temp->t_l[0] = 0;
	temp->t_l[1] = 0;
	temp->t_l[2] = 0;

	temp->stride_hash = google::sparse_hash_map<long, long, hash<long>, eqlng> ();
	temp->stride_hash.set_deleted_key(-1);

	memset (temp->last_access, 0, NUM_CORES_MAX*sizeof(size_t));

	temp->next = *listHead;
	*listHead = temp;
}

void record (bool debugFlag, var_list** listHead, mynode* new_access, mynode* old_access, set** l1_sets, set** l2_sets, set** l3_sets, bool silent=false)
{
	int old_lineNumber = old_access != NULL ? old_access->lineNumber : 0;

	var_list* listTraversal = *listHead;
	unsigned long vIndex;

	vIndex = new_access->vIndex;

	int minCacheLevel = old_access != NULL ? getCommonCacheLevel(old_access->coreID, new_access->coreID, old_access->type_access, new_access->type_access) : 4;

	long distance = 0;
	if (minCacheLevel == 1)	distance = old_access->l1_child_count[old_access->coreID];
	if (minCacheLevel == 2)	distance = old_access->l2_child_count[old_access->coreID >> 1];
	if (minCacheLevel == 3)	distance = old_access->l3_child_count[old_access->coreID >> 2];

	// Check that we won't segfault
	assert (new_access->coreID < NUM_L1);

	while (listTraversal && listTraversal->vIndex != vIndex)
		listTraversal = listTraversal->next;

	// This cache line was seen before but was associated with a different data structure
	if (!listTraversal)
	{
		// Add one to the beginning of the list
		var_list* temp = (var_list*) calloc (sizeof (var_list), 1);
		if (!temp)	assert(0);

		temp->vIndex = vIndex;

		temp->reuse_hash = google::sparse_hash_map<const char*, long, hash<const char*>, eqstr> ();
		temp->reuse_hash.set_deleted_key(NULL);

		temp->reuse_count[0] = google::sparse_hash_map<long, long, hash<long>, eqlng>();
		temp->reuse_count[1] = google::sparse_hash_map<long, long, hash<long>, eqlng>();
		temp->reuse_count[2] = google::sparse_hash_map<long, long, hash<long>, eqlng>();

		temp->reuse_count[0].set_deleted_key(0);
		temp->reuse_count[1].set_deleted_key(0);
		temp->reuse_count[2].set_deleted_key(0);

		temp->t_l[0] = 0;
		temp->t_l[1] = 0;
		temp->t_l[2] = 0;

		temp->stride_hash = google::sparse_hash_map<long, long, hash<long>, eqlng> ();
		temp->stride_hash.set_deleted_key(-1);

		memset (temp->last_access, 0, NUM_CORES_MAX*sizeof(size_t));

		temp->next = *listHead;
		*listHead = temp;

		listTraversal = temp;
	}

	// Find if both cores are in the same package
	unsigned int l1_hit=0, l2_hit=0, numa_hit=0;

	if (old_access == NULL)
	{
		l1_hit=1, l2_hit=1, numa_hit=1;
		new_access->core_home = new_access->coreID;
	}
	else
	{
		assert(old_access->core_home != -1);
		new_access->core_home = old_access->core_home;

		// Notice that we treat RR, RW and WW conflicts in the same manner here
		if ((old_access->type_access == TYPE_READ && new_access->type_access == TYPE_READ) || (old_access->core_home >> 0) == (new_access->coreID >> 0))	l1_hit	 = 1;
		if ((old_access->type_access == TYPE_READ && new_access->type_access == TYPE_READ) || (old_access->core_home >> 1) == (new_access->coreID >> 1))	l2_hit	 = 1;
		if ((old_access->type_access == TYPE_READ && new_access->type_access == TYPE_READ) || (old_access->core_home >> 2) == (new_access->coreID >> 2))	numa_hit = 1;
	}

	char* new_key = (char*) malloc (sizeof(char) * STRING_LEN);
	if (!new_key)	assert(0);

	sprintf (new_key, "%ld_%d_%d_%d_%d_%d_%d", distance, minCacheLevel, old_lineNumber, new_access->lineNumber, l1_hit, l2_hit, numa_hit);

	if (!silent)
	{
		(listTraversal->reuse_hash[new_key])++;

		if (listTraversal->last_access[new_access->coreID] != 0)
		{
			long stride = new_access->address - listTraversal->last_access[new_access->coreID];
			if (stride >= 0)
				listTraversal->stride_hash[stride]++;
		}

		listTraversal->last_access[new_access->coreID] = new_access->address;
	}
}

#define	MEM_LATENCY	(numa_hit == 1 ? LOCAL_MEM_LATENCY : REMOTE_MEM_LATENCY)

void freeRecords(bool debugFlag, var_list* listHead)
{
	var_list* temp;

	unsigned int l1_hit, l2_hit;
	typedef struct
	{
		long count, distance;
		double latency;
		int use_lineNumber, reuse_lineNumber;
		unsigned long fIndex;
	} values;

	typedef struct
	{
		long stride, count;
	} stride_values;

	if (!listHead && debugFlag)
		fprintf (stderr, "List head is NULL!\n");

	unsigned long fIndex;

	while (listHead)
	{
		int i;
		values max_values[MAX_LEADER_RECORDS];
		memset (max_values, 0, sizeof(values)*MAX_LEADER_RECORDS);

		temp = listHead->next;
		metrics localMetrics;
		memset (&localMetrics, 0, sizeof(metrics));

		google::sparse_hash_map<const char*, long, hash<const char*>, eqstr>::iterator it;
		for (it = listHead->reuse_hash.begin(); it != listHead->reuse_hash.end(); it++)
		{
			long distance;
			double local_latency=0;
			unsigned int numa_hit=0, minCacheLevel;
			int use_lineNumber, reuse_lineNumber;

			sscanf(it->first, "%ld_%u_%d_%d_%u_%u_%u", &distance, &minCacheLevel, &use_lineNumber, &reuse_lineNumber, &l1_hit, &l2_hit, &numa_hit);

			float L1HitProb = hit_probability(distance*LINE_SIZE, L1_SIZE, L1_ASSOC, LINE_SIZE);
			float L2HitProb = hit_probability(distance*LINE_SIZE, L2_SIZE, L2_ASSOC, LINE_SIZE);
			float L3HitProb = hit_probability(distance*LINE_SIZE, L3_SIZE, L3_ASSOC, LINE_SIZE);

			if (it->second > 0)
			{
				enum cacheLevels { _L1=0, _L2=1, _L3=2 };
				if (L1HitProb > 0.5)
				{
					if (listHead->t_l[_L1] == 0)	// Some other reuse is likely ``unclean''
					{
						if (listHead->t_l[_L2] != 0)
						{
							(listHead->reuse_count[_L2][listHead->t_l[_L2]])++;
							listHead->t_l[_L2] = 0;
						}

						if (listHead->t_l[_L3] != 0)
						{
							(listHead->reuse_count[_L3][listHead->t_l[_L3]])++;
							listHead->t_l[_L3] = 0;
						}
					}

					if (listHead->t_l[_L1] < ULLONG_MAX && listHead->t_l[_L1]+it->second > listHead->t_l[_L1])
						listHead->t_l[_L1] += it->second;
				}
				else if (L2HitProb > 0.5)
				{
					if (listHead->t_l[_L2] == 0)	// Some other reuse is likely ``unclean''
					{
						if (listHead->t_l[_L1] != 0)
						{
							(listHead->reuse_count[_L1][listHead->t_l[_L1]])++;
							listHead->t_l[_L1] = 0;
						}

						if (listHead->t_l[_L3] != 0)
						{
							(listHead->reuse_count[_L3][listHead->t_l[_L3]])++;
							listHead->t_l[_L3] = 0;
						}
					}

					if (listHead->t_l[_L2] < ULLONG_MAX && listHead->t_l[_L2]+it->second > listHead->t_l[_L2])
						listHead->t_l[_L2] += it->second;
				}
				else if (L3HitProb > 0.5)
				{
					if (listHead->t_l[_L3] == 0)	// Some other reuse is likely ``unclean''
					{
						if (listHead->t_l[_L1] != 0)
						{
							(listHead->reuse_count[_L1][listHead->t_l[_L1]])++;
							listHead->t_l[_L1] = 0;
						}

						if (listHead->t_l[_L2] != 0)
						{
							(listHead->reuse_count[_L2][listHead->t_l[_L2]])++;
							listHead->t_l[_L2] = 0;
						}
					}

					if (listHead->t_l[_L3] < ULLONG_MAX && listHead->t_l[_L3]+it->second > listHead->t_l[_L3])
						listHead->t_l[_L3] += it->second;
				}

				if (minCacheLevel == 1)
				{
					local_latency += (L1_LATENCY * L1HitProb);
					local_latency += (L2_LATENCY * (1-L1HitProb)*L2HitProb);
					local_latency += (L3_LATENCY * (1-L1HitProb)*(1-L2HitProb)*L3HitProb);
					local_latency += (MEM_LATENCY * (1-L1HitProb)*(1-L2HitProb)*(1-L3HitProb));

					localMetrics.l1_latency_count += L1HitProb*it->second;
					localMetrics.l2_latency_count += (1-L1HitProb)*L2HitProb*it->second;
					localMetrics.l3_latency_count += (1-L1HitProb)*(1-L2HitProb)*L3HitProb*it->second;
					localMetrics.mem_latency_count += (1-L1HitProb)*(1-L2HitProb)*(1-L3HitProb)*it->second;
				}
				else if (minCacheLevel == 2)
				{
					local_latency += L2_LATENCY * L2HitProb;
					local_latency += L3_LATENCY * (1-L2HitProb)*L3HitProb;
					local_latency += MEM_LATENCY * (1-L2HitProb)*(1-L3HitProb);

					localMetrics.l2_latency_count += L2HitProb*it->second;
					localMetrics.l3_latency_count += (1-L2HitProb)*L3HitProb*it->second;
					localMetrics.mem_latency_count += (1-L2HitProb)*(1-L3HitProb)*it->second;
				}
				else if (minCacheLevel == 3)
				{
					local_latency += L3_LATENCY * L3HitProb;
					local_latency += MEM_LATENCY * (1-L3HitProb);

					localMetrics.l3_latency_count += L3HitProb*it->second;
					localMetrics.mem_latency_count += (1-L3HitProb)*it->second;
				}
				else if (minCacheLevel == 4)
				{
					local_latency += MEM_LATENCY * 1-L3HitProb;
					localMetrics.mem_latency_count += (1-L3HitProb)*it->second;
				}

				localMetrics.program_count += it->second;
				localMetrics.program_distance += distance*it->second;
				localMetrics.program_latency += local_latency*it->second;

				localMetrics.l1_hit += l1_hit;
				localMetrics.l2_hit += l2_hit;
				localMetrics.numa_hit += numa_hit;

				localMetrics.l1_miss += 1-l1_hit;
				localMetrics.l2_miss += 1-l2_hit;
				localMetrics.numa_miss += 1-numa_hit;
			}

			for (i=0; i<MAX_LEADER_RECORDS; i++)
				if (max_values[i].count < it->second)
					break;

			if (i != MAX_LEADER_RECORDS)
			{
				// New leader, move the rest to the right
				for (int k=MAX_LEADER_RECORDS-1; k>i; k--)
					max_values[k] = max_values[k-1];

				max_values[i].latency = local_latency;
				max_values[i].distance = distance;
				max_values[i].count = it->second;
				max_values[i].use_lineNumber = use_lineNumber;
				max_values[i].reuse_lineNumber = reuse_lineNumber;

				max_values[i].fIndex = fIndex;
			}
		}

		// Finished iterating over listHead->reuse_hash
		// Now iterate over reuse_count to accumulate into average

		enum cacheLevels { _L1=0, _L2, _L3 };
		// First, clean up...
		if (listHead->t_l[_L1] != 0)	listHead->reuse_count[_L1][listHead->t_l[_L1]]++;
		if (listHead->t_l[_L2] != 0)	listHead->reuse_count[_L2][listHead->t_l[_L2]]++;
		if (listHead->t_l[_L3] != 0)	listHead->reuse_count[_L3][listHead->t_l[_L3]]++;

		listHead->t_l[_L1] = 0;
		listHead->t_l[_L2] = 0;
		listHead->t_l[_L3] = 0;

		int maxCount[3][3], maxReuse[3][3];

		bzero(maxCount, sizeof(maxCount));
		bzero(maxReuse, sizeof(maxReuse));

		// Iterate over each reuse_count
		for (int i=0; i<3; i++)
		{
			for (google::sparse_hash_map<long, long, hash<long>, eqlng>::iterator it = listHead->reuse_count[i].begin(); it != listHead->reuse_count[i].end(); it++)
			{
				if (it->second > maxCount[i][0])
				{
					maxReuse[i][2] = maxReuse[i][1], maxCount[i][2] = maxCount[i][1];
					maxReuse[i][1] = maxReuse[i][0], maxCount[i][1] = maxCount[i][0];
					maxReuse[i][0] = it->first, maxCount[i][0] = it->second;
				}
				else if (it->second > maxCount[i][1])
				{
					maxReuse[i][2] = maxReuse[i][1], maxCount[i][2] = maxCount[i][1];
					maxReuse[i][1] = it->first, maxCount[i][1] = it->second;
				}
				else if (it->second > maxCount[i][2])
					maxReuse[i][2] = it->first, maxCount[i][2] = it->second;
			}
		}

		localMetrics.l1_reuse[0] = maxReuse[_L1][0], localMetrics.l1_reuse_count[0] = maxCount[_L1][0];
		localMetrics.l1_reuse[1] = maxReuse[_L1][1], localMetrics.l1_reuse_count[1] = maxCount[_L1][1];
		localMetrics.l1_reuse[2] = maxReuse[_L1][2], localMetrics.l1_reuse_count[2] = maxCount[_L1][2];

		localMetrics.l2_reuse[0] = maxReuse[_L2][0], localMetrics.l2_reuse_count[0] = maxCount[_L2][0];
		localMetrics.l2_reuse[1] = maxReuse[_L2][1], localMetrics.l2_reuse_count[1] = maxCount[_L2][1];
		localMetrics.l2_reuse[2] = maxReuse[_L2][2], localMetrics.l2_reuse_count[2] = maxCount[_L2][2];

		localMetrics.l3_reuse[0] = maxReuse[_L3][0], localMetrics.l3_reuse_count[0] = maxCount[_L3][0];
		localMetrics.l3_reuse[1] = maxReuse[_L3][1], localMetrics.l3_reuse_count[1] = maxCount[_L3][1];
		localMetrics.l3_reuse[2] = maxReuse[_L3][2], localMetrics.l3_reuse_count[2] = maxCount[_L3][2];

		if (max_values[0].count > 0)
		{
			omp_set_lock(&var_lock);
			// We have the lock now, so don't worry about another thread accessing the same data

			varMetrics* ptr;
			// Lookup in the variable metrics list
			ptr = varMetricsHead;
			while (ptr && ptr->vIndex != listHead->vIndex)
				ptr = ptr->next;

			// Deal with NULL pointer
			if (!ptr)
			{
				ptr = (varMetrics*) malloc (sizeof (varMetrics));
				if (!ptr)	assert(0 && "Memory allocation failed\n");

				memset(ptr, 0, sizeof(varMetrics));

				ptr->vIndex = listHead->vIndex;
				ptr->next = varMetricsHead;
				varMetricsHead = ptr;

				// Take only the highest values
				ptr->reuse[0] = localMetrics.l1_reuse[0], ptr->reuse_count[0] = localMetrics.l1_reuse_count[0];
				ptr->reuse[1] = localMetrics.l2_reuse[0], ptr->reuse_count[1] = localMetrics.l2_reuse_count[0];
				ptr->reuse[2] = localMetrics.l3_reuse[0], ptr->reuse_count[2] = localMetrics.l3_reuse_count[0];

				ptr->stride_map = google::sparse_hash_map<long, long, hash<long>, eqlng>();
				ptr->stride_map.set_deleted_key(-1);
			}
			else
			{
				// Check if the existing value dominates all others
				if (ptr->reuse[0] == localMetrics.l1_reuse[0] && ptr->reuse_count[0]+localMetrics.l1_reuse_count[0] > localMetrics.l1_reuse_count[1] && ptr->reuse_count[0]+localMetrics.l1_reuse_count[0] > localMetrics.l1_reuse_count[2])
					ptr->reuse_count[0] += localMetrics.l1_reuse_count[0];
				else if (ptr->reuse[0] == localMetrics.l1_reuse[1] && ptr->reuse_count[0]+localMetrics.l1_reuse_count[1] > localMetrics.l1_reuse_count[0] && ptr->reuse_count[0]+localMetrics.l1_reuse_count[1] > localMetrics.l1_reuse_count[2])
					ptr->reuse_count[0] += localMetrics.l1_reuse_count[1];
				else if (ptr->reuse[0] == localMetrics.l1_reuse[2] && ptr->reuse_count[0]+localMetrics.l1_reuse_count[2] > localMetrics.l1_reuse_count[0] && ptr->reuse_count[0]+localMetrics.l1_reuse_count[2] > localMetrics.l1_reuse_count[1])
					ptr->reuse_count[0] += localMetrics.l1_reuse_count[2];
				else	// just take the maximum
					ptr->reuse_count[0] = localMetrics.l1_reuse_count[0], ptr->reuse[0] = localMetrics.l1_reuse[0];

				if (ptr->reuse[1] == localMetrics.l2_reuse[0] && ptr->reuse_count[1]+localMetrics.l2_reuse_count[0] > localMetrics.l2_reuse_count[1] && ptr->reuse_count[1]+localMetrics.l2_reuse_count[0] > localMetrics.l2_reuse_count[2])
					ptr->reuse_count[1] += localMetrics.l2_reuse_count[0];
				else if (ptr->reuse[1] == localMetrics.l2_reuse[1] && ptr->reuse_count[1]+localMetrics.l2_reuse_count[1] > localMetrics.l2_reuse_count[0] && ptr->reuse_count[1]+localMetrics.l2_reuse_count[1] > localMetrics.l2_reuse_count[2])
					ptr->reuse_count[1] += localMetrics.l2_reuse_count[1];
				else if (ptr->reuse[1] == localMetrics.l2_reuse[2] && ptr->reuse_count[1]+localMetrics.l2_reuse_count[2] > localMetrics.l2_reuse_count[0] && ptr->reuse_count[1]+localMetrics.l2_reuse_count[2] > localMetrics.l2_reuse_count[1])
					ptr->reuse_count[1] += localMetrics.l2_reuse_count[2];
				else	// just take the maximum
					ptr->reuse_count[1] = localMetrics.l2_reuse_count[0], ptr->reuse[1] = localMetrics.l2_reuse[0];

				if (ptr->reuse[2] == localMetrics.l3_reuse[0] && ptr->reuse_count[2]+localMetrics.l3_reuse_count[0] > localMetrics.l3_reuse_count[1] && ptr->reuse_count[2]+localMetrics.l3_reuse_count[0] > localMetrics.l3_reuse_count[2])
					ptr->reuse_count[2] += localMetrics.l3_reuse_count[0];
				else if (ptr->reuse[2] == localMetrics.l3_reuse[1] && ptr->reuse_count[2]+localMetrics.l3_reuse_count[1] > localMetrics.l3_reuse_count[0] && ptr->reuse_count[2]+localMetrics.l3_reuse_count[1] > localMetrics.l3_reuse_count[2])
					ptr->reuse_count[2] += localMetrics.l3_reuse_count[1];
				else if (ptr->reuse[2] == localMetrics.l3_reuse[2] && ptr->reuse_count[0]+localMetrics.l3_reuse_count[2] > localMetrics.l3_reuse_count[0] && ptr->reuse_count[2]+localMetrics.l3_reuse_count[2] > localMetrics.l3_reuse_count[1])
					ptr->reuse_count[2] += localMetrics.l3_reuse_count[2];
				else	// just take the maximum
					ptr->reuse_count[2] = localMetrics.l3_reuse_count[0], ptr->reuse[2] = localMetrics.l3_reuse[0];
			}

			google::sparse_hash_map<long, long, hash<long>, eqlng>::iterator it_stride;
			for (it_stride = listHead->stride_hash.begin(); it_stride != listHead->stride_hash.end(); it_stride++)
				ptr->stride_map[it_stride->first] += it_stride->second;

			ptr->l1_hit += localMetrics.l1_hit;
			ptr->l1_miss += localMetrics.l1_miss;

			ptr->l2_hit += localMetrics.l2_hit;
			ptr->l2_miss += localMetrics.l2_miss;

			ptr->numa_hit += localMetrics.numa_hit;
			ptr->numa_miss += localMetrics.numa_miss;

			ptr->tot_dist += localMetrics.program_distance;
			ptr->tot_lat += localMetrics.program_latency;
			ptr->tot_count += localMetrics.program_count;

			omp_unset_lock(&var_lock);
		}

		free(listHead);
		listHead = temp;
	}
}

chunk** readRecords(FILE* fp, short* setCounter, bool debugFlag /*, std::tr1::unordered_set<std::string>& functions */)
{
	int dummy;
	node_t dataNode;
	long counter=0;

#define	CHUNK_COUNT	32
	chunk** recordChunks = (chunk**) malloc (sizeof(chunk*) * CHUNK_COUNT);
	if (recordChunks == NULL)	assert(0);

	recordChunks[0] = (chunk*) malloc (sizeof (chunk));
	if (recordChunks[0] == NULL)
		assert(0);

	recordChunks[0]->fillCount = 0;
	recordChunks[0]->next = NULL;

	*setCounter = 0;

	if (debugFlag)
		fprintf (stderr, "Reading records now...\n");

	short dst_len;
	unsigned stream_count=0;
	while (!feof(fp))
	{
		int fillCount=0;
		dummy = fread(&dataNode, sizeof(node_t), 1, fp);

		switch(dataNode.type_message)
		{
			case MSG_METADATA:
				metadata_info = dataNode.metadata_info;
				break;

			case MSG_STREAM_INFO:
				char stream_name[STREAM_LENGTH];
				dst_len = MIN(STREAM_LENGTH-1, strlen(dataNode.stream_info.stream_name));
				strncpy(stream_name, dataNode.stream_info.stream_name, dst_len);
				stream_name[dst_len] = '\0';
				if (stream_count > 0)
				{
					if (var_idx[stream_count-1] != stream_name)
						var_idx[stream_count++] = stream_name;
				}
				else
					var_idx[stream_count++] = stream_name;

				break;

			case MSG_MEM_INFO:
				if (dataNode.coreID >= NUM_L1)	continue;

				fillCount = recordChunks[*setCounter]->fillCount;
				if (fillCount == CHUNK_SIZE)
				{
					chunk* newChunk = (chunk*) malloc (sizeof (chunk));
					if (newChunk == NULL)
						assert(0);

					newChunk->fillCount = 0;
					newChunk->next = recordChunks[*setCounter];
					recordChunks[*setCounter] = newChunk;
					fillCount = 0;
				}

				recordChunks[*setCounter]->nodes[fillCount].core_home = -1;
				recordChunks[*setCounter]->nodes[fillCount].coreID = dataNode.coreID;
				recordChunks[*setCounter]->nodes[fillCount].address = dataNode.mem_info.address;
				recordChunks[*setCounter]->nodes[fillCount].lineNumber = dataNode.mem_info.line_number;
				recordChunks[*setCounter]->nodes[fillCount].type_access = dataNode.mem_info.read_write;
				recordChunks[*setCounter]->nodes[fillCount].vIndex = dataNode.mem_info.var_idx;

				(recordChunks[*setCounter]->fillCount)++;
				break;

			case MSG_TERMINAL:
				(*setCounter)++;

				if (*setCounter == CHUNK_COUNT)
					assert(0);

				recordChunks[*setCounter] = (chunk*) malloc (sizeof(chunk));
				if (recordChunks[*setCounter] == NULL)
					assert(0);

				recordChunks[*setCounter]->fillCount = 0;
				recordChunks[*setCounter]->next = NULL;
				if (debugFlag)	fprintf (stderr, "Finished reading one set (%ld) of records...\n", counter);
				counter = 0;
				break;

			default:
				printf ("Unknown msg!\n");
		}

		counter++;
	}

	return recordChunks;
}

bool compare_metrics(print_metrics* metrics_01, print_metrics* metrics_02)
{
	if (metrics_01 && metrics_02)
		return metrics_01->observed_count >= metrics_02->observed_count;

	// Whatevah!
	return false;
}

void print_streams(bool bot, bool stream_names)
{
	if (!bot)
	{
		printf ("Total number of streams: %ld", var_idx.size());
		if ((var_idx.size() > 0 && var_idx.size() < STREAM_PRINT_THRESHOLD) || stream_names)
		{
			printf (" [ ");
			std::map<long, std::string>::iterator it;
			for (it=var_idx.begin(); it!=var_idx.end(); it++)
				printf ("%s ", it->second.c_str());
			printf ("]");
		}

		printf ("\n");
	}
	else
	{
		printf ("macpo.stream_count=%ld\n", var_idx.size());
		printf ("macpo.stream_list=");

		long ctr=0, len=var_idx.size();
		std::map<long, std::string>::iterator it;
		for (it=var_idx.begin(); it!=var_idx.end(); it++,ctr++)
		{
			if (ctr!=len-1)	printf ("%s,", it->second.c_str());
			else			printf ("%s", it->second.c_str());
		}

		printf ("\n");
	}
}

int main(int argc, char* argv[])
{
	struct arg_info info;
	memset (&info, 0, sizeof(struct arg_info));
	argp_parse (&argp, argc, argv, 0, 0, &info);

	if (info.arg2 == NULL)
	{
		info.threshold = 0.1;	// Default threshold of 10%
		info.location = info.arg1;
	}
	else
	{
		sscanf(info.arg1, "%f", &info.threshold);
		info.location = info.arg2;
	}

	FILE* fp = fopen(info.location, "r");
	if (fp == NULL)
	{
		printf ("Failed opening file: %s\n", info.location);
		return 1;
	}

	srand((unsigned) time(NULL));
	short setCounter;
	chunk** recordChunks = readRecords (fp, &setCounter, info.showDebug /*, functions */);

	if (info.showDebug)	fprintf (stderr, "Finished reading all records...\n");
	fclose(fp);

	// Printing the header
	if (metadata_info.binary_name[0] != (char) 0)
	{
		if (!info.bot)
			printf ("MACPO :: Analyzing logs created from the binary %s at %02d/%02d/%04d %02d:%02d:%02d\n\n", metadata_info.binary_name, metadata_info.month, metadata_info.day, metadata_info.year, metadata_info.hour, metadata_info.min, metadata_info.sec);
		else
		{
			printf ("macpo.binary_name=%s\n", metadata_info.binary_name);
			printf ("macpo.month=%d\n", metadata_info.month);
			printf ("macpo.day=%d\n", metadata_info.day);
			printf ("macpo.year=%d\n", metadata_info.year);
			printf ("macpo.hour=%d\n", metadata_info.hour);
			printf ("macpo.min=%d\n", metadata_info.min);
			printf ("macpo.sec=%d\n", metadata_info.sec);
		}
	}

	// Print stream count (and if forced or count less than the threshold, then print stream names as well).
	print_streams(info.bot, info.stream_names);

	omp_init_lock(&var_lock);

#pragma omp parallel for
	for (int i=0; i<=setCounter; i++)
	{
		int coreID;
		long loopID[NUM_L1];
		long counter=0, age=0;

		google::sparse_hash_map<size_t, mynode*, hash<size_t>, eqsize_t> avlTrack;
		avlTrack.set_deleted_key(0);

		set* l1_sets[NUM_L1]={0};
		set* l2_sets[NUM_L2]={0};
		set* l3_sets[NUM_L3]={0};

		if (L1_SETS != -1)	for(int j=0; j<NUM_L1; j++)	l1_sets[j] = (set*) calloc (L1_SETS, sizeof(set));
		if (L2_SETS != -1)	for(int j=0; j<NUM_L2; j++)	l2_sets[j] = (set*) calloc (L2_SETS, sizeof(set));
		if (L3_SETS != -1)	for(int j=0; j<NUM_L3; j++)	l3_sets[j] = (set*) calloc (L3_SETS, sizeof(set));

		for (int k=0; k<NUM_L1; k++)
			for (int j=0; j<L1_SETS; j++)
				l1_sets[k][j].residue = rand();

		for (int k=0; k<NUM_L2; k++)
			for (int j=0; j<L2_SETS; j++)
				l2_sets[k][j].residue = rand();

		for (int k=0; k<NUM_L3; k++)
			for (int j=0; j<L3_SETS; j++)
				l3_sets[k][j].residue = rand();

		// Reverse the list
		chunk *old, *curr, *future;
		old = NULL;
		curr = recordChunks[i];

		while (curr != NULL)
		{
			future = curr->next;
			curr->next = old;

			old = curr;
			curr = future;
		}

		recordChunks[i] = old;

		// Process each of these sets
		AVL_TREE mytree = avlinit(valcmp, nodesize);

		curr = recordChunks[i];
		var_list* listHead = NULL;

		if (info.showDebug)	fprintf (stderr, "Set %d starting processing...\n", i);

		size_t prev_addr=0;
		while (curr != NULL)
		{
			if (curr->fillCount == 0 && info.showDebug)	fprintf (stderr, "Fill count is zero!\n");
			for (int j=0; j<curr->fillCount; j++)
			{
				coreID = curr->nodes[j].coreID;
				mynode* old_access = avlTrack [curr->nodes[j].address];
				if (old_access != NULL)	old_access = (mynode*) avldel ((void*) old_access, mytree, valcmp);

				curr->nodes[j].ts = age;

				age++;
				if (old_access != NULL)
				{
					if (prev_addr != curr->nodes[j].address)
						record(info.showDebug, &listHead, &curr->nodes[j], old_access, l1_sets, l2_sets, l3_sets);
					else
						record(info.showDebug, &listHead, &curr->nodes[j], old_access, l1_sets, l2_sets, l3_sets, true);
				}
				else
				{
					recordHome(info.showDebug, &listHead, &curr->nodes[j]);
					record(info.showDebug, &listHead, &curr->nodes[j], NULL, l1_sets, l2_sets, l3_sets);
				}

				prev_addr = curr->nodes[j].address;

				mynode* inserted = avlins((void*) &curr->nodes[j], mytree);
				avlTrack [curr->nodes[j].address] = inserted;
				inserted->vIndex = curr->nodes[j].vIndex;
				// assert (strncmp(inserted->var_name, curr->nodes[j].var_name, EXT_VAR_LEN-1) == 0);

				if (info.showDebug)
				{
					counter++;
					if (counter % 10000 == 0)
						fprintf (stderr, "%ld items finished from set %d\n", counter, i);
				}
			}

			curr = curr->next;
		}

		if (info.showDebug)	fprintf (stderr, "Set %d finished processing, ready to update %ld global records...\n", i, counter);

		avldispose(&mytree, NULL, LEFT_TO_RIGHT);
		freeRecords(info.showDebug, listHead);

		if (L1_SETS != -1)	for(int j=0; j<NUM_L1; j++)	free(l1_sets[j]);
		if (L2_SETS != -1)	for(int j=0; j<NUM_L2; j++)	free(l2_sets[j]);
		if (L3_SETS != -1)	for(int j=0; j<NUM_L3; j++)	free(l3_sets[j]);

		if (info.showDebug)	fprintf (stderr, "Set %d finished updating global records...\n", i);
	}

	omp_destroy_lock(&var_lock);

	varMetrics* ptr = varMetricsHead;
	std::list<print_metrics*> final_metrics_list;

	long total_observed_count = 0;
	while (ptr)
	{
		print_metrics* metrics = new print_metrics();

		metrics->var_name = var_idx[ptr->vIndex];
		double reuse = ptr->reuse[0] + ptr->reuse[1] + ptr->reuse[2];

		// Place stride_map values into a set
		stride_list_t stride_list;

		stride_info_t stride_info;
		google::sparse_hash_map<long, long, hash<long>, eqlng>::iterator it;
		for (it = ptr->stride_map.begin(); it != ptr->stride_map.end(); it++)
		{
			stride_info.first = it->first;	// Stride value
			stride_info.second = it->second;	// Stride count

			stride_list.push_back(stride_info);
		}

		std::sort(stride_list.begin(), stride_list.end(), sort_function);

		int i;
		for (i=0; i<3 && i<stride_list.size(); i++)
		{
			metrics->stride_value[i] = stride_list[i].first;
			metrics->stride_count[i] = stride_list[i].second;
		}

		for (; i<3; i++)
		{
			metrics->stride_value[i] = 0;
			metrics->stride_count[i] = 0;
		}

		total_observed_count += ptr->tot_count;
		metrics->observed_count = ptr->tot_count;
		metrics->avg_cpa = ptr->tot_lat /((double) ptr->tot_count);
		metrics->l1_conflict_ratio = 100.0 * (1-ptr->l1_hit / ((double) (ptr->l1_hit + ptr->l1_miss)));
		metrics->l2_conflict_ratio = 100.0 * (1-ptr->l2_hit / ((double) (ptr->l2_hit + ptr->l2_miss)));
		metrics->numa_conflict_ratio = 100.0 * (1-ptr->numa_hit / ((double) (ptr->numa_hit + ptr->numa_miss)));

		metrics->l1_reuse = 100.0 * ptr->reuse[0] / reuse;
		metrics->l2_reuse = 100.0 * ptr->reuse[1] / reuse;
		metrics->l3_reuse = 100.0 * ptr->reuse[2] / reuse;

		final_metrics_list.push_back(metrics);

		ptr = ptr->next;
	}

	final_metrics_list.sort(compare_metrics);

	for (std::list<print_metrics*>::iterator i = final_metrics_list.begin(); i != final_metrics_list.end(); i++)
	{
		print_metrics* metrics = *i;

		float observed_percentage = float(metrics->observed_count) / total_observed_count;
		if (observed_percentage >= info.threshold)
		{
			if (!info.bot)
			{
				int i, max;
				printf ("\n================================================================================\n");
				printf ("Var \"%s\", seen %ld times [%.2f%%], estimated to cost %.2f cycles on every access\n", metrics->var_name.c_str(), metrics->observed_count, 100.0f * metrics->observed_count / total_observed_count, metrics->avg_cpa);

				long count_sum = metrics->stride_count[0] + metrics->stride_count[1] + metrics->stride_count[2];
				if (count_sum > 0)
				{
					for (int i=0; i<3 && metrics->stride_count[i] > 0; i++)
						printf ("Stride of %d cache lines was observed %ld times (%.2f%%).\n", metrics->stride_value[i], metrics->stride_count[i], ((float) metrics->stride_count[i]) / count_sum * 100.0f);
				}
				else
				{
					for (int i=0; i<3 && metrics->stride_count[i] > 0; i++)
						printf ("Stride of %d cache lines was observed %ld times.\n", metrics->stride_value[i], metrics->stride_count[i]);
				}

				printf ("\n");

				printf ("Level 1 data cache conflicts = %.2f%% [", metrics->l1_conflict_ratio);
				max = ceil(40.0f*((float) metrics->l1_conflict_ratio)/100.0f);
				for (i=0; i<max; i++)
					printf ("#");

				while (i++ < 40)
					printf (" ");

				printf ("]\n");

				printf ("Level 2 data cache conflicts = %.2f%% [", metrics->l2_conflict_ratio);
				max = ceil(40.0f*((float) metrics->l2_conflict_ratio)/100.0f);
				for (i=0; i<max; i++)
					printf ("#");

				while (i++ < 40)
					printf (" ");

				printf ("]\n");

				printf ("NUMA data conflicts = %.2f%%					[", metrics->numa_conflict_ratio);
				max = ceil(40.0f*((float) metrics->numa_conflict_ratio)/100.0f);
				for (i=0; i<max; i++)
					printf ("#");

				while (i++ < 40)
					printf (" ");

				printf ("]\n\n");

				printf ("Level 1 data cache reuse factor = %03.1f%% [", metrics->l1_reuse);
				max = ceil(40.0f*((float) metrics->l1_reuse)/100.0f);
				for (i=0; i<max; i++)
					printf ("#");

				while (i++ < 40)
					printf (" ");

				printf ("]\n");

				printf ("Level 2 data cache reuse factor = %03.1f%% [", metrics->l2_reuse);
				max = ceil(40.0f*((float) metrics->l2_reuse)/100.0f);
				for (i=0; i<max; i++)
					printf ("#");

				while (i++ < 40)
					printf (" ");

				printf ("]\n");

				printf ("Level 3 data cache reuse factor = %03.1f%% [", metrics->l3_reuse);
				max = ceil(40.0f*((float) metrics->l3_reuse)/100.0f);
				for (i=0; i<max; i++)
					printf ("#");

				while (i++ < 40)
					printf (" ");

				printf ("]\n");
				printf ("================================================================================\n");
			}
			else
			{
				for (int i=0; i<3 && metrics->stride_count[i] > 0; i++)
				{
					printf ("%s.stride_%d.value=%d\n", metrics->var_name.c_str(), i, metrics->stride_value[i]);
					printf ("%s.stride_%d.count=%ld\n", metrics->var_name.c_str(), i, metrics->stride_count[i]);
				}

				printf ("%s.count=%ld\n", metrics->var_name.c_str(), metrics->observed_count);
				printf ("%s.percentage=%.2f\n", metrics->var_name.c_str(), 100.0f * metrics->observed_count / total_observed_count);
				printf ("%s.avg_cpa=%.2f\n", metrics->var_name.c_str(), metrics->avg_cpa);
				printf ("%s.l1_conflict_ratio=%.0f\n", metrics->var_name.c_str(), metrics->l1_conflict_ratio);
				printf ("%s.l2_conflict_ratio=%.0f\n", metrics->var_name.c_str(), metrics->l2_conflict_ratio);
				// printf ("%s.l3_conflict_ratio=%.0f\n", var_name.c_str(), 100.0 * (1-ptr->l3_hit / ((double) (ptr->l3_hit + ptr->l3_miss))));
				printf ("%s.numa_conflict_ratio=%.0f\n", metrics->var_name.c_str(), metrics->numa_conflict_ratio);
				printf ("%s.l1_reuse=%.0f\n", metrics->var_name.c_str(), metrics->l1_reuse);
				printf ("%s.l2_reuse=%.0f\n", metrics->var_name.c_str(), metrics->l2_reuse);
				printf ("%s.l3_reuse=%.0f\n", metrics->var_name.c_str(), metrics->l3_reuse);
			}
		}
	}

	ptr = varMetricsHead;
	while (ptr)
	{
		print_metrics* metrics = final_metrics_list.back();
		final_metrics_list.pop_back();
		free(metrics);

		ptr = ptr->next;
		free(varMetricsHead);
		varMetricsHead = ptr;
	}

	return 0;
}
