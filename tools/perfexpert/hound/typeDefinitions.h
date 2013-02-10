/*
    Copyright (C) 2013 The University of Texas at Austin

    This file is part of PerfExpert.

    PerfExpert is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PerfExpert is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PerfExpert.  If not, see <http://www.gnu.org/licenses/>.

    Author: Ashay Rane
*/

#ifndef TYPEDEFINITIONS_H_
#define TYPEDEFINITIONS_H_

#define DONT_CARE			0

enum { INSTRUCTION=0, DATA, UNIFIED, TYPE_UNKNOWN };
enum { CACHE=0, TLB };
enum { UNKNOWN=-1, FULLY_ASSOCIATIVE=0, DIRECT_MAPPED=1 };

enum { PAGESIZE_4K=1, PAGESIZE_2M=2, PAGESIZE_4M=4 };

typedef struct {
	unsigned char cacheOrTLB:1;
	unsigned char type:2;
	short level:4;

	int lineSize;	// Also used as bitmap for page size for TLBs
	int lineCount;
	short wayness;
} cacheInfo;

typedef struct tagCacheList {
	cacheInfo info;
	struct tagCacheList* lpNext;
} cacheList;

typedef struct {
	cacheList* lpL1Caches;
	cacheList* lpL2Caches;
	cacheList* lpL3Caches;
} cacheCollection;

typedef struct {
	short code;
	cacheInfo info;
} intelCacheTableEntry;

const char* getCacheType(unsigned char type) {
	if (type == DATA)
		return "data";

	if (type == INSTRUCTION)
		return "instruction";

	if (type == UNIFIED)
		return "unified";
}

#endif /* TYPEDEFINITIONS_H_ */
