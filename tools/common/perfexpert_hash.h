/*
 * Copyright (c) 2011-2013  University of Texas at Austin. All rights reserved.
 * Copyright (c) 2003-2013  Troy D. Hanson. All rights reserved.
 *                          http://troydhanson.github.com/uthash/
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
 * Author's License (BSD) follow
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Troy D. Hanson
 * Adapted by: Leonardo Fialho
 *
 * $HEADER$
 */

#ifndef PERFEXPERT_HASH_H_
#define PERFEXPERT_HASH_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STDLIB_H
#include <stdlib.h> /* exit() */
#endif

#ifndef _STRING_H
#include <string.h> /* memcmp,strlen */
#endif

#ifndef _STDDEF_H
#include <stddef.h> /* ptrdiff_t */
#endif

#ifndef _INTTYPES_H
#include <inttypes.h> /* uint32_t */
#endif

#define UTHASH_VERSION 1.9.8

#define DECLTYPE(x) (__typeof(x))
#define DECLTYPE_ASSIGN(dst, src)   \
    do {                            \
        (dst) = DECLTYPE(dst)(src); \
    } while(0)

#ifndef uthash_fatal
#define uthash_fatal(msg) exit(-1)        /* fatal error (out of memory,etc) */
#endif
#ifndef uthash_malloc
#define uthash_malloc(sz) malloc(sz)      /* malloc fcn                      */
#endif
#ifndef uthash_free
#define uthash_free(ptr, sz) free(ptr)    /* free fcn                        */
#endif

#ifndef uthash_noexpand_fyi
#define uthash_noexpand_fyi(tbl)          /* can be defined to log noexpand  */
#endif
#ifndef uthash_expand_fyi
#define uthash_expand_fyi(tbl)            /* can be defined to log expands   */
#endif

/* initial number of buckets */
#define HASH_INITIAL_NUM_BUCKETS      32 /* initial number of buckets        */
#define HASH_INITIAL_NUM_BUCKETS_LOG2 5  /* lg2 of initial number of buckets */
#define HASH_BKT_CAPACITY_THRESH      10 /* expand when bucket count reaches */

/* ELMT_FROM_HH */
#define ELMT_FROM_HH(tbl, hhp) ((void*)(((char*)(hhp)) - ((tbl)->hho)))

/* HASH_FIND */
#define HASH_FIND(hh, head, keyptr, keylen, out)                             \
    do {                                                                     \
        unsigned _hf_bkt,_hf_hashv;                                          \
        out = NULL;                                                          \
        if (head) {                                                          \
            HASH_FCN(keyptr,keylen, (head)->hh.tbl->num_buckets, _hf_hashv,  \
                _hf_bkt);                                                    \
            if (HASH_BLOOM_TEST((head)->hh.tbl, _hf_hashv)) {                \
                HASH_FIND_IN_BKT((head)->hh.tbl, hh,                         \
                    (head)->hh.tbl->buckets[_hf_bkt], keyptr,keylen,out);    \
            }                                                                \
        }                                                                    \
    } while (0)

#ifdef HASH_BLOOM
/* HASH_BLOOM_BITLEN */
#define HASH_BLOOM_BITLEN (1ULL << HASH_BLOOM)

/* HASH_BLOOM_BYTELEN */
#define HASH_BLOOM_BYTELEN (HASH_BLOOM_BITLEN / 8) + ((HASH_BLOOM_BITLEN % 8) \
    ? 1 : 0)

/* HASH_BLOOM_MAKE */
#define HASH_BLOOM_MAKE(tbl)                                           \
    do {                                                               \
        (tbl)->bloom_nbits = HASH_BLOOM;                               \
        (tbl)->bloom_bv = (uint8_t*)uthash_malloc(HASH_BLOOM_BYTELEN); \
        if (!((tbl)->bloom_bv)) {                                      \
            uthash_fatal( "out of memory");                            \
        }                                                              \
        memset((tbl)->bloom_bv, 0, HASH_BLOOM_BYTELEN);                \
        (tbl)->bloom_sig = HASH_BLOOM_SIGNATURE;                       \
    } while (0)

/* HASH_BLOOM_FREE */
#define HASH_BLOOM_FREE(tbl)                              \
    do {                                                  \
        uthash_free((tbl)->bloom_bv, HASH_BLOOM_BYTELEN); \
    } while (0)

/* HASH_BLOOM_BITSET */ 
#define HASH_BLOOM_BITSET(bv, idx) (bv[(idx) / 8] |= (1U << ((idx) % 8)))

/* HASH_BLOOM_BITTEST */
#define HASH_BLOOM_BITTEST(bv,idx) (bv[(idx) / 8] & (1U << ((idx) % 8)))

/* HASH_BLOOM_ADD */
#define HASH_BLOOM_ADD(tbl, hashv)     \
    HASH_BLOOM_BITSET((tbl)->bloom_bv, \
        (hashv & (uint32_t)((1ULL << (tbl)->bloom_nbits) - 1)))

/* HASH_BLOOM_TEST */
#define HASH_BLOOM_TEST(tbl, hashv)     \
    HASH_BLOOM_BITTEST((tbl)->bloom_bv, \
        (hashv & (uint32_t)((1ULL << (tbl)->bloom_nbits) - 1)))

#else /* HASH_BLOOM */
/* HASH_BLOOM_BYTELEN */
#define HASH_BLOOM_BYTELEN 0

/* HASH_BLOOM_MAKE */
#define HASH_BLOOM_MAKE(tbl) 

/* HASH_BLOOM_FREE */
#define HASH_BLOOM_FREE(tbl) 

/* HASH_BLOOM_ADD */
#define HASH_BLOOM_ADD(tbl, hashv) 

/* HASH_BLOOM_TEST */
#define HASH_BLOOM_TEST(tbl, hashv) (1)
#endif /* HASH_BLOOM */

/* HASH_MAKE_TABLE */
#define HASH_MAKE_TABLE(hh, head)                                         \
    do {                                                                  \
        (head)->hh.tbl = (UT_hash_table *)uthash_malloc(sizeof(           \
            UT_hash_table));                                              \
        if (!((head)->hh.tbl)) {                                          \
            uthash_fatal("out of memory");                                \
        }                                                                 \
        memset((head)->hh.tbl, 0, sizeof(UT_hash_table));                 \
        (head)->hh.tbl->tail = &((head)->hh);                             \
        (head)->hh.tbl->num_buckets = HASH_INITIAL_NUM_BUCKETS;           \
        (head)->hh.tbl->log2_num_buckets = HASH_INITIAL_NUM_BUCKETS_LOG2; \
        (head)->hh.tbl->hho = (char*)(&(head)->hh) - (char*)(head);       \
        (head)->hh.tbl->buckets = (UT_hash_bucket*)uthash_malloc(         \
            HASH_INITIAL_NUM_BUCKETS * sizeof(struct UT_hash_bucket));    \
        if (!(head)->hh.tbl->buckets) {                                   \
            uthash_fatal("out of memory");                                \
        }                                                                 \
        memset((head)->hh.tbl->buckets, 0,                                \
            HASH_INITIAL_NUM_BUCKETS * sizeof(struct UT_hash_bucket));    \
        HASH_BLOOM_MAKE((head)->hh.tbl);                                  \
        (head)->hh.tbl->signature = HASH_SIGNATURE;                       \
    } while (0)

/* HASH_ADD */
#define HASH_ADD(hh, head, fieldname, keylen_in, add) \
    HASH_ADD_KEYPTR(hh, head, &((add)->fieldname), keylen_in, add)

/* HASH_REPLACE */
#define HASH_REPLACE(hh, head, fieldname, keylen_in, add, replaced)     \
    do {                                                                \
        replaced = NULL;                                                \
        HASH_FIND(hh, head, &((add)->fieldname), keylen_in, replaced);  \
        if (replaced! = NULL) {                                         \
            HASH_DELETE(hh, head, replaced);                            \
        };                                                              \
        HASH_ADD(hh, head, fieldname, keylen_in, add);                  \
    } while (0)

/* HASH_ADD_KEYPTR */
#define HASH_ADD_KEYPTR(hh, head, keyptr, keylen_in, add)              \
    do {                                                               \
        unsigned _ha_bkt;                                              \
        (add)->hh.next = NULL;                                         \
        (add)->hh.key = (char*)keyptr;                                 \
        (add)->hh.keylen = (unsigned)keylen_in;                        \
        if (!(head)) {                                                 \
            head = (add);                                              \
            (head)->hh.prev = NULL;                                    \
            HASH_MAKE_TABLE(hh, head);                                 \
        } else {                                                       \
            (head)->hh.tbl->tail->next = (add);                        \
            (add)->hh.prev = ELMT_FROM_HH((head)->hh.tbl,              \
                (head)->hh.tbl->tail);                                 \
            (head)->hh.tbl->tail = &((add)->hh);                       \
        }                                                              \
        (head)->hh.tbl->num_items++;                                   \
        (add)->hh.tbl = (head)->hh.tbl;                                \
        HASH_FCN(keyptr,keylen_in, (head)->hh.tbl->num_buckets,        \
            (add)->hh.hashv, _ha_bkt);                                 \
        HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], &(add)->hh); \
        HASH_BLOOM_ADD((head)->hh.tbl, (add)->hh.hashv);               \
        HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                    \
        HASH_FSCK(hh, head);                                           \
    } while (0)

/* HASH_TO_BKT */
#define HASH_TO_BKT(hashv, num_bkts, bkt)   \
    do {                                    \
        bkt = ((hashv) & ((num_bkts) - 1)); \
    } while (0)

/* HASH_DELETE */
#define HASH_DELETE(hh, head, delptr)                                          \
    do {                                                                       \
        unsigned _hd_bkt;                                                      \
        struct UT_hash_handle *_hd_hh_del;                                     \
        if ( ((delptr)->hh.prev == NULL) && ((delptr)->hh.next == NULL) )  {   \
            uthash_free((head)->hh.tbl->buckets, (head)->hh.tbl->num_buckets * \
                sizeof(struct UT_hash_bucket));                                \
            HASH_BLOOM_FREE((head)->hh.tbl);                                   \
            uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                \
            head = NULL;                                                       \
        } else {                                                               \
            _hd_hh_del = &((delptr)->hh);                                      \
            if ((delptr) == ELMT_FROM_HH((head)->hh.tbl,                       \
                (head)->hh.tbl->tail)) {                                       \
                (head)->hh.tbl->tail =                                         \
                    (UT_hash_handle *)((ptrdiff_t)((delptr)->hh.prev) +        \
                    (head)->hh.tbl->hho);                                      \
            }                                                                  \
            if ((delptr)->hh.prev) {                                           \
                ((UT_hash_handle *)((ptrdiff_t)((delptr)->hh.prev) +           \
                        (head)->hh.tbl->hho))->next = (delptr)->hh.next;       \
            } else {                                                           \
                DECLTYPE_ASSIGN(head,(delptr)->hh.next);                       \
            }                                                                  \
            if (_hd_hh_del->next) {                                            \
                ((UT_hash_handle *)((ptrdiff_t)_hd_hh_del->next +              \
                    (head)->hh.tbl->hho))->prev = _hd_hh_del->prev;            \
            }                                                                  \
            HASH_TO_BKT(_hd_hh_del->hashv, (head)->hh.tbl->num_buckets,        \
                _hd_bkt);                                                      \
            HASH_DEL_IN_BKT(hh, (head)->hh.tbl->buckets[_hd_bkt], _hd_hh_del); \
            (head)->hh.tbl->num_items--;                                       \
        }                                                                      \
        HASH_FSCK(hh, head);                                                   \
    } while (0)

#ifdef HASH_DEBUG
/* HASH_OOPS */
#define HASH_OOPS(...)                \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        exit(-1);                     \
    } while (0)

/* HASH_FSCK */
#define HASH_FSCK(hh, head)                                                    \
    do {                                                                       \
        unsigned _bkt_i;                                                       \
        unsigned _count, _bkt_count;                                           \
        char *_prev;                                                           \
        struct perfexpert_hash_handle *_thh;                                   \
        if (head) {                                                            \
            _count = 0;                                                        \
            for (_bkt_i = 0; _bkt_i < (head)->hh.tbl->num_buckets; _bkt_i++) { \
                _bkt_count = 0;                                                \
                _thh = (head)->hh.tbl->buckets[_bkt_i].hh_head;                \
                _prev = NULL;                                                  \
                while (_thh) {                                                 \
                    if (_prev != (char*)(_thh->hh_prev)) {                     \
                        HASH_OOPS("invalid hh_prev %p, actual %p\n",           \
                            _thh->hh_prev, _prev);                             \
                    }                                                          \
                    _bkt_count++;                                              \
                    _prev = (char*)(_thh);                                     \
                    _thh = _thh->hh_next;                                      \
                }                                                              \
                _count += _bkt_count;                                          \
                if ((head)->hh.tbl->buckets[_bkt_i].count != _bkt_count) {     \
                    HASH_OOPS("invalid bucket count %d, actual %d\n",          \
                    (head)->hh.tbl->buckets[_bkt_i].count, _bkt_count);        \
                }                                                              \
            }                                                                  \
            if (_count != (head)->hh.tbl->num_items) {                         \
                HASH_OOPS("invalid hh item count %d, actual %d\n",             \
                    (head)->hh.tbl->num_items, _count);                        \
            }                                                                  \
            /* traverse hh in app order; check next/prev integrity, count */   \
            _count = 0;                                                        \
            _prev = NULL;                                                      \
            _thh =  &(head)->hh;                                               \
            while (_thh) {                                                     \
                _count++;                                                      \
                if (_prev !=(char*)(_thh->prev)) {                             \
                    HASH_OOPS("invalid prev %p, actual %p\n", _thh->prev,      \
                        _prev);                                                \
                }                                                              \
                _prev = (char*)ELMT_FROM_HH((head)->hh.tbl, _thh);             \
                _thh = (_thh->next ?                                           \
                    (perfexpert_hash_handle*)((char*)(_thh->next) +            \
                    (head)->hh.tbl->hho) : NULL);                              \
            }                                                                  \
            if (_count != (head)->hh.tbl->num_items) {                         \
                HASH_OOPS("invalid app item count %d, actual %d\n",            \
                    (head)->hh.tbl->num_items, _count);                        \
            }                                                                  \
        }                                                                      \
    } while (0)

#else /* HASH_DEBUG */
/* HASH_FSCK */
#define HASH_FSCK(hh, head) 
#endif /* HASH_DEBUG */

#ifdef HASH_EMIT_KEYS
/* HASH_EMIT_KEY */
#define HASH_EMIT_KEY(hh, head, keyptr, fieldlen)     \
    do {                                              \
        unsigned _klen = fieldlen;                    \
        write(HASH_EMIT_KEYS, &_klen, sizeof(_klen)); \
        write(HASH_EMIT_KEYS, keyptr, fieldlen);      \
    } while (0)

#else /* HASH_EMIT_KEYS */
/* HASH_EMIT_KEY */
#define HASH_EMIT_KEY(hh,head,keyptr,fieldlen)                    
#endif /* HASH_EMIT_KEYS */

/* HASH_FCN */
#define HASH_FCN HASH_JEN

/* HASH_JEN_MIX */
#define HASH_JEN_MIX(a, b, c)           \
    do {                                \
        a -= b; a -= c; a ^= (c >> 13); \
        b -= c; b -= a; b ^= (a << 8);  \
        c -= a; c -= b; c ^= (b >> 13); \
        a -= b; a -= c; a ^= (c >> 12); \
        b -= c; b -= a; b ^= (a << 16); \
        c -= a; c -= b; c ^= (b >> 5);  \
        a -= b; a -= c; a ^= (c >> 3);  \
        b -= c; b -= a; b ^= (a << 10); \
        c -= a; c -= b; c ^= (b >> 15); \
    } while (0)

/* HASH_JEN */
#define HASH_JEN(key, keylen, num_bkts, hashv, bkt)            \
    do {                                                       \
        unsigned _hj_i, _hj_j, _hj_k;                          \
        unsigned char *_hj_key = (unsigned char*)(key);        \
        hashv = 0xfeedbeef;                                    \
        _hj_i = _hj_j = 0x9e3779b9;                            \
        _hj_k = (unsigned)keylen;                              \
        while (_hj_k >= 12) {                                  \
            _hj_i += (_hj_key[0] + ((unsigned)_hj_key[1] << 8) \
                + ((unsigned)_hj_key[2] << 16)                 \
                + ((unsigned)_hj_key[3] << 24));               \
            _hj_j += (_hj_key[4] + ((unsigned)_hj_key[5] << 8) \
                + ((unsigned)_hj_key[6] << 16)                 \
                + ((unsigned)_hj_key[7] << 24));               \
            hashv += (_hj_key[8] + ((unsigned)_hj_key[9] << 8) \
                + ((unsigned)_hj_key[10] << 16)                \
                + ((unsigned)_hj_key[11] << 24));              \
                                                               \
            HASH_JEN_MIX(_hj_i, _hj_j, hashv);                 \
                                                               \
            _hj_key += 12;                                     \
            _hj_k -= 12;                                       \
        }                                                      \
        hashv += keylen;                                       \
        switch ( _hj_k ) {                                     \
            case 11: hashv += ((unsigned)_hj_key[10] << 24);   \
            case 10: hashv += ((unsigned)_hj_key[9] << 16);    \
            case 9:  hashv += ((unsigned)_hj_key[8] << 8);     \
            case 8:  _hj_j += ((unsigned)_hj_key[7] << 24);    \
            case 7:  _hj_j += ((unsigned)_hj_key[6] << 16);    \
            case 6:  _hj_j += ((unsigned)_hj_key[5] << 8);     \
            case 5:  _hj_j += _hj_key[4];                      \
            case 4:  _hj_i += ((unsigned)_hj_key[3] << 24);    \
            case 3:  _hj_i += ((unsigned)_hj_key[2] << 16);    \
            case 2:  _hj_i += ((unsigned)_hj_key[1] << 8);     \
            case 1:  _hj_i += _hj_key[0];                      \
        }                                                      \
        HASH_JEN_MIX(_hj_i, _hj_j, hashv);                     \
        bkt = hashv & (num_bkts - 1);                          \
    } while (0)

/* HASH_KEYCMP */
#define HASH_KEYCMP(a, b, len) memcmp(a, b, len) 

/* HASH_FIND_IN_BKT */
#define HASH_FIND_IN_BKT(tbl, hh, head, keyptr, keylen_in, out)             \
    do {                                                                    \
        if (head.hh_head) {                                                 \
            DECLTYPE_ASSIGN(out, ELMT_FROM_HH(tbl, head.hh_head));          \
        }  else {                                                           \
            out = NULL;                                                     \
        }                                                                   \
        while (out) {                                                       \
            if ((out)->hh.keylen == keylen_in) {                            \
                if ((HASH_KEYCMP((out)->hh.key, keyptr, keylen_in)) == 0) { \
                    break;                                                  \
                }                                                           \
            }                                                               \
            if ((out)->hh.hh_next) {                                        \
                DECLTYPE_ASSIGN(out, ELMT_FROM_HH(tbl, (out)->hh.hh_next)); \
            } else {                                                        \
                out = NULL;                                                 \
            }                                                               \
        }                                                                   \
    } while(0)

/* HASH_ADD_TO_BKT */
#define HASH_ADD_TO_BKT(head, addhh)                                          \
    do {                                                                      \
        head.count++;                                                         \
        (addhh)->hh_next = head.hh_head;                                      \
        (addhh)->hh_prev = NULL;                                              \
        if (head.hh_head) {                                                   \
            (head).hh_head->hh_prev = (addhh);                                \
        }                                                                     \
        (head).hh_head=addhh;                                                 \
        if (head.count >= ((head.expand_mult + 1) * HASH_BKT_CAPACITY_THRESH) \
            && (addhh)->tbl->noexpand != 1) {                                 \
            HASH_EXPAND_BUCKETS((addhh)->tbl);                                \
        }                                                                     \
    } while (0)

/* HASH_DEL_IN_BKT */
#define HASH_DEL_IN_BKT(hh, head, hh_del)           \
    (head).count--;                                 \
    if ((head).hh_head == hh_del) {                 \
        (head).hh_head = hh_del->hh_next;           \
    }                                               \
    if (hh_del->hh_prev) {                          \
        hh_del->hh_prev->hh_next = hh_del->hh_next; \
    }                                               \
    if (hh_del->hh_next) {                          \
        hh_del->hh_next->hh_prev = hh_del->hh_prev; \
    }

#define HASH_EXPAND_BUCKETS(tbl)                                            \
    do {                                                                    \
        unsigned _he_bkt;                                                   \
        unsigned _he_bkt_i;                                                 \
        struct UT_hash_handle *_he_thh, *_he_hh_nxt;                        \
        UT_hash_bucket *_he_new_buckets, *_he_newbkt;                       \
        _he_new_buckets = (UT_hash_bucket *)uthash_malloc(2 *               \
            tbl->num_buckets * sizeof(struct UT_hash_bucket));              \
        if (!_he_new_buckets) {                                             \
            uthash_fatal("out of memory");                                  \
        }                                                                   \
        memset(_he_new_buckets, 0, 2 * tbl->num_buckets *                   \
            sizeof(struct UT_hash_bucket));                                 \
        tbl->ideal_chain_maxlen =                                           \
            (tbl->num_items >> (tbl->log2_num_buckets + 1)) +               \
            ((tbl->num_items & ((tbl->num_buckets * 2) - 1)) ? 1 : 0);      \
        tbl->nonideal_items = 0;                                            \
        for(_he_bkt_i = 0; _he_bkt_i < tbl->num_buckets; _he_bkt_i++)  {    \
            _he_thh = tbl->buckets[_he_bkt_i].hh_head;                      \
            while (_he_thh) {                                               \
                _he_hh_nxt = _he_thh->hh_next;                              \
                HASH_TO_BKT(_he_thh->hashv, tbl->num_buckets * 2, _he_bkt); \
                _he_newbkt = &(_he_new_buckets[_he_bkt]);                   \
                if (++(_he_newbkt->count) > tbl->ideal_chain_maxlen) {      \
                    tbl->nonideal_items++;                                  \
                    _he_newbkt->expand_mult = _he_newbkt->count /           \
                        tbl->ideal_chain_maxlen;                            \
                }                                                           \
            _he_thh->hh_prev = NULL;                                        \
            _he_thh->hh_next = _he_newbkt->hh_head;                         \
            if (_he_newbkt->hh_head) {                                      \
                _he_newbkt->hh_head->hh_prev =                              \
                _he_thh;                                                    \
            }                                                               \
            _he_newbkt->hh_head = _he_thh;                                  \
            _he_thh = _he_hh_nxt;                                           \
        }                                                                   \
    }                                                                       \
    uthash_free(tbl->buckets, tbl->num_buckets *                            \
        sizeof(struct UT_hash_bucket));                                     \
    tbl->num_buckets *= 2;                                                  \
    tbl->log2_num_buckets++;                                                \
    tbl->buckets = _he_new_buckets;                                         \
    tbl->ineff_expands = (tbl->nonideal_items > (tbl->num_items >> 1)) ?    \
        (tbl->ineff_expands + 1) : 0;                                       \
    if (tbl->ineff_expands > 1) {                                           \
        tbl->noexpand = 1;                                                  \
        uthash_noexpand_fyi(tbl);                                           \
    }                                                                       \
    uthash_expand_fyi(tbl);                                                 \
} while(0)

/* HASH_SRT */
#define HASH_SRT(hh, head, cmpfcn)                                             \
    do {                                                                       \
        unsigned _hs_i;                                                        \
        unsigned _hs_looping, _hs_nmerges, _hs_insize, _hs_psize, _hs_qsize;   \
        struct perfexpert_hash_handle *_hs_p, *_hs_q, *_hs_e, *_hs_list,       \
            *_hs_tail;                                                         \
        if (head) {                                                            \
            _hs_insize = 1;                                                    \
            _hs_looping = 1;                                                   \
            _hs_list = &((head)->hh);                                          \
            while (_hs_looping) {                                              \
                _hs_p = _hs_list;                                              \
                _hs_list = NULL;                                               \
                _hs_tail = NULL;                                               \
                _hs_nmerges = 0;                                               \
                while (_hs_p) {                                                \
                    _hs_nmerges++;                                             \
                    _hs_q = _hs_p;                                             \
                    _hs_psize = 0;                                             \
                    for (_hs_i = 0; _hs_i < _hs_insize; _hs_i++) {             \
                        _hs_psize++;                                           \
                        _hs_q = (perfexpert_hash_handle*)((_hs_q->next) ?      \
                            ((void*)((char*)(_hs_q->next) +                    \
                            (head)->hh.tbl->hho)) : NULL);                     \
                        if (!(_hs_q)) {                                        \
                            break;                                             \
                        }                                                      \
                    }                                                          \
                    _hs_qsize = _hs_insize;                                    \
                    while ((_hs_psize > 0) || ((_hs_qsize > 0) && _hs_q )) {   \
                        if (_hs_psize == 0) {                                  \
                            _hs_e = _hs_q;                                     \
                            _hs_q = (perfexpert_hash_handle*)((_hs_q->next) ?  \
                                ((void*)((char*)(_hs_q->next) +                \
                                (head)->hh.tbl->hho)) : NULL);                 \
                            _hs_qsize--;                                       \
                        } else if ((_hs_qsize == 0) || !(_hs_q)) {             \
                            _hs_e = _hs_p;                                     \
                            if (_hs_p) {                                       \
                                _hs_p =                                        \
                                    (perfexpert_hash_handle*)((_hs_p->next) ?  \
                                    ((void*)((char*)(_hs_p->next) +            \
                                    (head)->hh.tbl->hho)) : NULL);             \
                            }                                                  \
                            _hs_psize--;                                       \
                        } else if ((cmpfcn(DECLTYPE(head)                      \
                            (ELMT_FROM_HH((head)->hh.tbl, _hs_p)),             \
                            DECLTYPE(head)(ELMT_FROM_HH((head)->hh.tbl,        \
                            _hs_q)))) <= 0) {                                  \
                            _hs_e = _hs_p;                                     \
                            if (_hs_p){                                        \
                                _hs_p =                                        \
                                    (perfexpert_hash_handle*)((_hs_p->next) ?  \
                                    ((void*)((char*)(_hs_p->next) +            \
                                    (head)->hh.tbl->hho)) : NULL);             \
                            }                                                  \
                            _hs_psize--;                                       \
                        } else {                                               \
                            _hs_e = _hs_q;                                     \
                            _hs_q = (perfexpert_hash_handle*)((_hs_q->next) ?  \
                                ((void*)((char*)(_hs_q->next) +                \
                                (head)->hh.tbl->hho)) : NULL);                 \
                            _hs_qsize--;                                       \
                        }                                                      \
                        if (_hs_tail) {                                        \
                            _hs_tail->next = ((_hs_e) ?                        \
                                ELMT_FROM_HH((head)->hh.tbl,_hs_e) : NULL);    \
                        } else {                                               \
                            _hs_list = _hs_e;                                  \
                        }                                                      \
                        if (_hs_e) {                                           \
                            _hs_e->prev = ((_hs_tail) ?                        \
                                ELMT_FROM_HH((head)->hh.tbl,_hs_tail) : NULL); \
                        }                                                      \
                        _hs_tail = _hs_e;                                      \
                    }                                                          \
                    _hs_p = _hs_q;                                             \
                }                                                              \
                if (_hs_tail) {                                                \
                    _hs_tail->next = NULL;                                     \
                }                                                              \
                if (_hs_nmerges <= 1) {                                        \
                    _hs_looping = 0;                                           \
                    (head)->hh.tbl->tail = _hs_tail;                           \
                    DECLTYPE_ASSIGN(head, ELMT_FROM_HH((head)->hh.tbl,         \
                        _hs_list));                                            \
                }                                                              \
                _hs_insize *= 2;                                               \
            }                                                                  \
            HASH_FSCK(hh, head);                                               \
        }                                                                      \
    } while (0)

/* HASH_SELECT */
#define HASH_SELECT(hh_dst, dst, hh_src, src, cond)                            \
    do {                                                                       \
        unsigned _src_bkt, _dst_bkt;                                           \
        void *_last_elt = NULL, *_elt;                                         \
        perfexpert_hash_handle *_src_hh, *_dst_hh, *_last_elt_hh = NULL;       \
        ptrdiff_t _dst_hho = ((char*)(&(dst)->hh_dst) - (char*)(dst));         \
        if (src) {                                                             \
            for(_src_bkt = 0; _src_bkt < (src)->hh_src.tbl->num_buckets;       \
                _src_bkt++) {                                                  \
                for(_src_hh = (src)->hh_src.tbl->buckets[_src_bkt].hh_head;    \
                    _src_hh; _src_hh = _src_hh->hh_next) {                     \
                    _elt = ELMT_FROM_HH((src)->hh_src.tbl, _src_hh);           \
                    if (cond(_elt)) {                                          \
                        _dst_hh = (perfexpert_hash_handle*)(((char*)_elt) +    \
                            _dst_hho);                                         \
                        _dst_hh->key = _src_hh->key;                           \
                        _dst_hh->keylen = _src_hh->keylen;                     \
                        _dst_hh->hashv = _src_hh->hashv;                       \
                        _dst_hh->prev = _last_elt;                             \
                        _dst_hh->next = NULL;                                  \
                        if (_last_elt_hh) {                                    \
                            _last_elt_hh->next = _elt;                         \
                        }                                                      \
                        if (!dst) {                                            \
                            DECLTYPE_ASSIGN(dst, _elt);                        \
                            HASH_MAKE_TABLE(hh_dst, dst);                      \
                        } else {                                               \
                            _dst_hh->tbl = (dst)->hh_dst.tbl;                  \
                        }                                                      \
                        HASH_TO_BKT(_dst_hh->hashv, _dst_hh->tbl->num_buckets, \
                            _dst_bkt);                                         \
                        HASH_ADD_TO_BKT(_dst_hh->tbl->buckets[_dst_bkt],       \
                            _dst_hh);                                          \
                        (dst)->hh_dst.tbl->num_items++;                        \
                        _last_elt = _elt;                                      \
                        _last_elt_hh = _dst_hh;                                \
                    }                                                          \
                }                                                              \
            }                                                                  \
        }                                                                      \
        HASH_FSCK(hh_dst, dst);                                                \
    } while (0)

/* HASH_CLEAR */
#define HASH_CLEAR(hh, head)                                                   \
    do {                                                                       \
        if (head) {                                                            \
            uthash_free((head)->hh.tbl->buckets, (head)->hh.tbl->num_buckets * \
                sizeof(struct UT_hash_bucket));                                \
            HASH_BLOOM_FREE((head)->hh.tbl);                                   \
            uthash_free((head)->hh.tbl, sizeof(UT_hash_table));                \
            (head) = NULL;                                                     \
        }                                                                      \
    } while (0)

/* HASH_ITER */
#define HASH_ITER(hh, head, el, tmp)                                           \
    for ((el) = (head), (tmp) = DECLTYPE(el)((head) ? (head)->hh.next : NULL); \
        el; (el) = (tmp), (tmp) = DECLTYPE(el)((tmp) ? (tmp)->hh.next : NULL))

/* HASH_CNT */
#define HASH_CNT(hh, head) ((head) ? ((head)->hh.tbl->num_items) : 0)

/* HASH_SIGNATURE */
#define HASH_SIGNATURE 0xa0111fe1

/* HASH_BLOOM_SIGNATURE */
#define HASH_BLOOM_SIGNATURE 0xb12220f2

typedef struct UT_hash_bucket {
    struct UT_hash_handle *hh_head;
    unsigned count;
    unsigned expand_mult;
} UT_hash_bucket;

typedef struct UT_hash_table {
    UT_hash_bucket *buckets;
    unsigned num_buckets, log2_num_buckets;
    unsigned num_items;
    struct UT_hash_handle *tail;
    ptrdiff_t hho;
    unsigned ideal_chain_maxlen;
    unsigned nonideal_items;
    unsigned ineff_expands, noexpand;
    uint32_t signature;
    #ifdef HASH_BLOOM
    uint32_t bloom_sig;
    uint8_t *bloom_bv;
    char bloom_nbits;
    #endif
} UT_hash_table;

typedef struct UT_hash_handle {
    struct UT_hash_table *tbl;
    void *prev;                       /* prev element in app order      */
    void *next;                       /* next element in app order      */
    struct UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
    struct UT_hash_handle *hh_next;   /* next hh in bucket order        */
    void *key;                        /* ptr to enclosing struct's key  */
    unsigned keylen;                  /* enclosing struct's key len     */
    unsigned hashv;                   /* result of hash-fcn(key)        */
} UT_hash_handle;

/* Convenience forms */
#define perfexpert_hash_find(_1, _2, _3, _4) HASH_FIND(hh, _1, _2, _3, _4)
#define perfexpert_hash_find_str(head, findstr, out) \
    HASH_FIND(hh_str, head, findstr, strlen(findstr), out)
#define perfexpert_hash_find_int(head, findint, out) \
    HASH_FIND(hh_int, head, findint, sizeof(int), out)
#define perfexpert_hash_find_ptr(head, findptr, out) \
    HASH_FIND(hh_ptr, head, findptr, sizeof(void *), out)

#define perfexpert_hash_add(_1, _2, _3, _4) HASH_ADD(hh, _1, _2, _3, _4)
#define perfexpert_hash_add_str(head, strfield, add) \
    HASH_ADD(hh_str, head, strfield, strlen(add->strfield), add)
#define perfexpert_hash_add_int(head, intfield, add) \
    HASH_ADD(hh_int, head, intfield, sizeof(int), add)
#define perfexpert_hash_add_ptr(head, ptrfield, add) \
    HASH_ADD(hh_ptr, head, ptrfield, sizeof(void *), add)

#define perfexpert_hash_replace(_1, _2, _3, _4) HASH_REPLACE(hh, _1, _2, _3, _4)
#define perfexpert_hash_replace_str(head, strfield, add, replaced) \
    HASH_REPLACE(hh_str, head, strfield, strlen(add->strfield), add, replaced)
#define perfexpert_hash_replace_int(head, intfield, add, replaced) \
    HASH_REPLACE(hh_int, head, intfield, sizeof(int), add, replaced)
#define perfexpert_hash_replace_ptr(head, ptrfield, add) \
    HASH_REPLACE(hh_ptr, head, ptrfield, sizeof(void *), add, replaced)

#define perfexpert_hash_del(_1, _2) HASH_DELETE(hh, _1, _2)
#define perfexpert_hash_del_str(head, delptr) HASH_DELETE(hh_str, head, delptr)
#define perfexpert_hash_del_int(head, delptr) HASH_DELETE(hh_int, head, delptr)
#define perfexpert_hash_del_ptr(head, delptr) HASH_DELETE(hh_ptr, head, delptr)

#define perfexpert_hash_sort(_1, _2) HASH_SRT(hh, _1, _2)
#define perfexpert_hash_sort_str(head, cmpfcn) HASH_SRT(hh_str, head, cmpfcn)
#define perfexpert_hash_sort_int(head, cmpfcn) HASH_SRT(hh_int, head, cmpfcn)
#define perfexpert_hash_sort_ptr(head, cmpfcn) HASH_SRT(hh_ptr, head, cmpfcn)

#define perfexpert_hash_count(_1) HASH_SRT(hh, _1)
#define perfexpert_hash_count_str(head) HASH_CNT(hh_str, head)
#define perfexpert_hash_count_int(head) HASH_CNT(hh_int, head)
#define perfexpert_hash_count_ptr(head) HASH_CNT(hh_ptr, head)

#define perfexpert_hash_iter(_1, _2) HASH_ITER(hh, _1, _2)
#define perfexpert_hash_iter_str(head, el, tmp) HASH_ITER(hh_str, head, el, tmp)
#define perfexpert_hash_iter_int(head, el, tmp) HASH_ITER(hh_int, head, el, tmp)
#define perfexpert_hash_iter_ptr(head, el, tmp) HASH_ITER(hh_ptr, head, el, tmp)

/* Just to be PerfExpert compliant :-) */
typedef UT_hash_table perfexpert_hash_table_t;
typedef UT_hash_bucket perfexpert_hash_bucket_t;
typedef UT_hash_handle perfexpert_hash_handle_t;

#ifdef __cplusplus
}
#endif

#endif /* PERFEXPERT_HASH_H */
