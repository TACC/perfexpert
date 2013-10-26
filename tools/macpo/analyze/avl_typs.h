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

/** <plaintext>
*
* avl_typs.h -- declaration of private types used for avl trees
*
* Created 03/01/89 by Brad Appleton
*
* ^{Mods:* }
*
* Fri Jul 14 13:55:58 1989, Rev 1.0, brad(0165)
*
**/

#ifndef AVL_TYPS_H_
#define AVL_TYPS_H_

  /* definition of a NULL action and a NULL tree */
#define NULL_ACTION    ((void(*)()) NULL)
#define NULL_TREE      ((AVLtree)     NULL)


        /* MIN and MAX macros (used for rebalancing) */
#define  MIN(a,b)    ((a) < (b) ? (a) : (b))
#define  MAX(a,b)    ((a) > (b) ? (a) : (b))


       /* Directional Definitions */
typedef  short  DIRECTION;
#define  LEFT             0
#define  RIGHT            1
#define  OPPOSITE(x)     (1 - (x))


       /* return codes used by avl_insert(), avl_delete(), and balance() */
#define  HEIGHT_UNCHANGED       0
#define  HEIGHT_CHANGED         1


       /* Balance Definitions */
#define  LEFT_HEAVY            -1
#define  BALANCED               0
#define  RIGHT_HEAVY            1
#define  LEFT_IMBALANCE(nd)    ((nd)->bal < LEFT_HEAVY)
#define  RIGHT_IMBALANCE(nd)   ((nd)->bal > RIGHT_HEAVY)


  /* structure for a node in an AVL tree */
typedef struct avl_node {
    void        *data;            /* pointer to data */
    short       bal;             /* balance factor */
    struct avl_node  *subtree[2];      /* LEFT and RIGHT subtrees */

    short coreID, type_access;
    long l1_child_count [NUM_L1];
    long l2_child_count [NUM_L2];
    long l3_child_count [NUM_L3];
} AVLnode, *AVLtree;


  /* structure which holds information about an AVL tree */
typedef struct avl_descriptor {
    AVLtree     root;           /* pointer to the root node of the tree */
    int         (*compar)(void*, void*, NODE);    /* function used to compare keys */
    unsigned    (*isize)(void*);     /* function to return the size of an item */
    long        count;          /* number of nodes in the tree */
} AVLdescriptor;

#endif /* AVL_TYPS_H_ */
