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
* avl.c -- C source file for avl trees. Contains the auxillary routines
*          and defines for the avl tree functions and user interface and
*          includes all the necessary public and private routines
*
* Created 03/01/89 by Brad Appleton
*
* ^{Mods:* }
*
* Fri Jul 14 13:53:42 1989, Rev 1.0, brad(0165)
*
**/

     /* some common #defines used throughout most of my files */
#define  PUBLIC   /* default */
#define  PRIVATE  static
#define	FALSE	0
#define	TRUE	!FALSE

     /* some defines for debugging purposes */
#ifdef NDEBUG
#define DBG(x)          /* x */
#else
#define DBG(x)          x
#endif

#define  NEXTERN   /* dont include "extern" declarations from header files */

#include  <stdio.h>
#include <stdlib.h>
#include <string.h>

#include  "avl.h"         /* public types for avl trees */
#include  "avl_typs.h"    /* private types for avl trees */
#include  "structures.h"

/************************************************************************
*       Auxillary functions
*
*       routines to allocate/de-allocate an AVL node,
*       and determine the type of an AVL node.
************************************************************************/

/* ckalloc(size) -- allocate space; check for success */
PRIVATE  char *
ckalloc(int size)
{
   char *ptr;

   if ((ptr = (char*) malloc((unsigned) size)) == NULL) {
      fprintf(stderr, "Unable to allocate storage.");
      exit(1);
   }/* if */

   return  ptr;
}/* ckalloc */


/*
* new_node() -- get space for a new node and its data;
*               return the address of the new node
*/
PRIVATE  AVLtree
new_node(void* data, unsigned size, short coreID, short type_access)
{
   AVLtree  root;

   root = (AVLtree) ckalloc(sizeof (AVLnode));
   root->data = (void *) ckalloc(size);
   memcpy(root->data, data, size);
   root->bal  = BALANCED;
   root->subtree[LEFT]  = root->subtree[RIGHT] = NULL_TREE;

   root->coreID = coreID;
   root->type_access = type_access;
   memset(root->l1_child_count, 0, NUM_L1 * sizeof(long));
   memset(root->l2_child_count, 0, NUM_L2 * sizeof(long));
   memset(root->l3_child_count, 0, NUM_L3 * sizeof(long));

   root->l1_child_count[coreID] = 1;
   root->l2_child_count[coreID >> 1] = 1;
   root->l3_child_count[coreID >> 2] = 1;

   return  root;
}/* new_node */


/*
* free_node()  --  free space for a node and its data!
*                  reset the node pointer to NULL
*/
PRIVATE  void
free_node(AVLtree* rootp)
{
   free((void *) *rootp);
   *rootp = NULL_TREE;
}/* free_node */


/*
* node_type() -- determine the number of null pointers for a given
*                node in an AVL tree, Returns a value of type NODE
*                which is an enumeration type with the following values:
*
*                  IS_TREE     --  both subtrees are non-empty
*                  IS_LBRANCH  --  left subtree is non-empty; right is empty
*                  IS_RBRANCH  --  right subtree is non-empty; left is empty
*                  IS_LEAF     --  both subtrees are empty
*                  IS_NULL     --  given tree is empty
*/
PRIVATE  NODE
node_type(AVLtree tree)
{
   if (tree == NULL_TREE) {
       return  IS_NULL;
   } else if ((tree->subtree[LEFT] != NULL_TREE)  &&
              (tree->subtree[RIGHT] != NULL_TREE)) {
       return  IS_TREE;
   } else if (tree->subtree[LEFT] != NULL_TREE) {
       return  IS_LBRANCH;
   } else if (tree->subtree[RIGHT] != NULL_TREE) {
       return  IS_RBRANCH;
   } else {
       return  IS_LEAF;
   }
}/* node_type */



/************************************************************************
*       PRIVATE functions for manipulating AVL trees
*
*  This following defines a set of routines for creating, maintaining, and
*  manipulating AVL Trees as an Abtract Data Type. The routines in this
*  file that are accessible (through the avl tree user-interface) to other
*  files to allow other programmers to:
*
*       Insert, Delete, and Find a given data item from a Tree.
*
*       Delete and Find the minimal and Maximal items in a Tree.
*
*       Walk through every node in a tree performing a giving operation.
*
*       Walk through and free up space for every node in a tree while performing
*       a given operation on the data items as they are encountered.
************************************************************************/



/************************************************************************
*       routines used to find the minimal and maximal elements
*       (nodes) of an AVL tree.
************************************************************************/

/*
* avl_min() -- compare function used to find the minimal element in a tree
*/
PRIVATE  int
avl_min(void* elt1, void* elt2, NODE nd_typ)
{
   return (nd_typ == IS_RBRANCH  ||  nd_typ == IS_LEAF)
             ?  0   /* left subtree is empty -- this is the minimum */
             : -1;  /* keep going left */
}/* avl_min */


/*
* avl_max() -- compare function used to find the maximal element in a tree
*/
PRIVATE  int
avl_max(void* elt1, void* elt2, NODE nd_typ)
{
   return (nd_typ == IS_LBRANCH  ||  nd_typ == IS_LEAF)
             ?  0   /* right subtree is empty -- this is the maximum */
             :  1;  /* keep going right */
}/* avl_max */


/*
avl_children() -- returns the number of children for a given node
*
*   PARAMETERS:
*                root       -- pointer to node
*/

PRIVATE  long
avl_children(AVLtree root, short level, short coreID)
{
/*
*                  IS_TREE     --  both subtrees are non-empty
*                  IS_LBRANCH  --  left subtree is non-empty; right is empty
*                  IS_RBRANCH  --  right subtree is non-empty; left is empty
*                  IS_LEAF     --  both subtrees are empty
*                  IS_NULL     --  given tree is empty
*/

	NODE nd_type = node_type(root);

	if (nd_type == IS_NULL)	return 0;

	if (level == 1)	return root->l1_child_count[coreID];
	if (level == 2)	return root->l2_child_count[coreID >> 1];
	if (level == 3)	return root->l3_child_count[coreID >> 2];

	assert(0);
//	return root->child_count;
}


/************************************************************************
*       Routines to perform rotations on AVL trees
************************************************************************/

/*
* rotate_once()  --  rotate a given node in the given direction
*                    to restore the balance of a tree
*/
PRIVATE  short
rotate_once(AVLtree* rootp, DIRECTION dir, short coreID, short type_access)
{
   DIRECTION   other_dir = OPPOSITE(dir);  /* opposite direction to "dir" */
   AVLtree     old_root  = *rootp;         /* copy of original root of tree */
   short       ht_unchanged;               /* true if height unchanged */

   ht_unchanged = ((*rootp)->subtree[other_dir]->bal) ? FALSE : TRUE;

      /* assign new root */
   *rootp = old_root->subtree[other_dir];

      /* new-root exchanges it's "dir" subtree for it's parent */
   old_root->subtree[other_dir] =  (*rootp)->subtree[dir];
   (*rootp)->subtree[dir]       =  old_root;

      /* update balances */
   old_root->bal = -(dir == LEFT ? --((*rootp)->bal) : ++((*rootp)->bal));

   short rightCoreID, rightTypeAccess, cacheLevel;
   AVLtree temp = old_root;
   if (temp->subtree[RIGHT])
   {
      rightCoreID = temp->subtree[RIGHT]->coreID;
      rightTypeAccess = temp->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	temp->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	temp->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	temp->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 3, coreID) + 1;

   return  ht_unchanged;
}/* rotate_once */


/*
* rotate_twice()  --  rotate a given node in the given direction
*                     and then in the opposite direction
*                     to restore the balance of a tree
*/
PRIVATE  void
rotate_twice(AVLtree* rootp, DIRECTION dir, short coreID, short type_access)
{
   DIRECTION  other_dir             = OPPOSITE(dir);
   AVLtree    old_root              = *rootp;
   AVLtree    old_other_dir_subtree = (*rootp)->subtree[other_dir];

      /* assign new root */
   *rootp = old_root->subtree[other_dir]->subtree[dir];

      /* new-root exchanges it's "dir" subtree for it's grandparent */
   old_root->subtree[other_dir] = (*rootp)->subtree[dir];
   (*rootp)->subtree[dir]       = old_root;

      /* new-root exchanges it's "other-dir" subtree for it's parent */
   old_other_dir_subtree->subtree[dir] = (*rootp)->subtree[other_dir];
   (*rootp)->subtree[other_dir]        = old_other_dir_subtree;

      /* update balances */
   (*rootp)->subtree[LEFT]->bal  = -MAX((*rootp)->bal, 0);
   (*rootp)->subtree[RIGHT]->bal = -MIN((*rootp)->bal, 0);
   (*rootp)->bal                 = 0;

   short rightCoreID, rightTypeAccess, cacheLevel;
   AVLtree temp;

   temp = (*rootp)->subtree[LEFT];
   if (temp->subtree[RIGHT])
   {
      rightCoreID = temp->subtree[RIGHT]->coreID;
      rightTypeAccess = temp->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	temp->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	temp->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	temp->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 3, coreID) + 1;

   temp = (*rootp)->subtree[RIGHT];
   if (temp->subtree[RIGHT])
   {
      rightCoreID = temp->subtree[RIGHT]->coreID;
      rightTypeAccess = temp->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	temp->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	temp->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	temp->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 3, coreID) + 1;

   temp = (*rootp);
   if (temp->subtree[RIGHT])
   {
      rightCoreID = temp->subtree[RIGHT]->coreID;
      rightTypeAccess = temp->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	temp->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	temp->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	temp->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 3, coreID) + 1;

#if 0
   (*rootp)->subtree[LEFT]->child_count = /* avl_children((*rootp)->subtree[LEFT]->subtree[LEFT]) + */ avl_children((*rootp)->subtree[LEFT]->subtree[RIGHT]) + 1;
   (*rootp)->subtree[RIGHT]->child_count = /* avl_children((*rootp)->subtree[RIGHT]->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT]->subtree[RIGHT]) + 1;
   (*rootp)->child_count = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT]) + 1;
#endif

}/* rotate_twice */


/************************************************************************
*                       Rebalance an AVL tree
************************************************************************/

/*
* balance()  --  determines and performs the  sequence of rotations needed
*                   (if any) to restore the balance of a given tree.
*
*     Returns 1 if tree height changed due to rotation; 0 otherwise
*/
PRIVATE short
balance(AVLtree* rootp, short coreID, short type_access)
{
   short  special_case = FALSE;

   if (LEFT_IMBALANCE(*rootp)) {   /* need a right rotation */
      if ((*rootp)->subtree[LEFT]->bal  ==  RIGHT_HEAVY) {
         rotate_twice(rootp, RIGHT, coreID, type_access);    /* double RL rotation needed */
      } else {   /* single RR rotation needed */
         special_case = rotate_once(rootp, RIGHT, coreID, type_access);
      }
   } else if (RIGHT_IMBALANCE(*rootp)) {   /* need a left rotation */
      if ((*rootp)->subtree[RIGHT]->bal  ==  LEFT_HEAVY) {
         rotate_twice(rootp, LEFT, coreID, type_access);     /* double LR rotation needed */
      } else {   /* single LL rotation needed */
         special_case = rotate_once(rootp, LEFT, coreID, type_access);
      }
   } else {
      return  HEIGHT_UNCHANGED;     /* no rotation occurred */
   }

	if (!special_case)
	{
   short rightCoreID, rightTypeAccess, cacheLevel;
   AVLtree temp;

   temp = (*rootp);
   if (temp->subtree[RIGHT])
   {
      rightCoreID = temp->subtree[RIGHT]->coreID;
      rightTypeAccess = temp->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	temp->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	temp->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	temp->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children(temp->subtree[RIGHT], 3, coreID) + 1;


//		(*rootp)->child_count = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT]) + 1;
	}

   return  (special_case) ? HEIGHT_UNCHANGED : HEIGHT_CHANGED;
}/* balance */

/************************************************************************
*       Routines to:    Find an item in an AVL tree
*                       Insert an item into an AVL tree
*                       Delete an item from an AVL tree
************************************************************************/

/*
* avl_find() -- find an item in the given tree
*
*   PARAMETERS:
*                data       --  a pointer to the key to find
*                rootp      --  a pointer to an AVL tree
*                compar     --  name of a function to compare 2 data items
*/
PRIVATE  void *
avl_find(void* data, AVLtree tree, int (*compar)(void*, void*, NODE))
{
   NODE  nd_typ = node_type(tree);
   int   cmp;

   while ((tree != NULL_TREE)  &&
          (cmp = (*compar)(data, tree->data, nd_typ))) {
      tree = tree->subtree[(cmp < 0) ? LEFT : RIGHT];
   }

   return  (tree == NULL_TREE) ? NULL : tree->data;
}/* avl_find */


/*
* avl_insert() -- insert an item into the given tree
*
*   PARAMETERS:
*                data       --  a pointer to a pointer to the data to add;
*                               On exit, *data is NULL if insertion succeeded,
*                               otherwise address of the duplicate key
*                rootp      --  a pointer to an AVL tree
*                compar     --  name of the function to compare 2 data items
*/
PRIVATE  short
avl_insert(void** data, unsigned size, AVLtree* rootp, int (*compar)(void*, void*, NODE), mynode** inserted, short coreID, short type_access)
{
   short increase;
   int   cmp;

   if (*rootp == NULL_TREE) {  /* insert new node here */
      *rootp = new_node(*data, size, coreID, type_access);
      *inserted = ((mynode*) (*rootp)->data);
      *data  = NULL;     /* set return value in data */
      return  HEIGHT_CHANGED;
   }/* if */

   cmp = (*compar)(*data, (*rootp)->data, IS_NULL);   /* compare data items */

   if (cmp < 0) {  /* insert into the left subtree */
      increase =  -avl_insert(data, size, &((*rootp)->subtree[LEFT]), compar, inserted, coreID, type_access);
      if (*data != NULL)  return  HEIGHT_UNCHANGED;
   } else if (cmp > 0) {  /* insert into the right subtree */
      increase =  avl_insert(data, size, &((*rootp)->subtree[RIGHT]), compar, inserted, coreID, type_access);
      if (*data != NULL)  return  HEIGHT_UNCHANGED;
   } else  {   /* data already exists */
      *data = (*rootp)->data;   /* set return value in data */
      return  HEIGHT_UNCHANGED;
   }

   short rightCoreID, rightTypeAccess, cacheLevel;
   if ((*rootp)->subtree[RIGHT])
   {
      rightCoreID = (*rootp)->subtree[RIGHT]->coreID;
      rightTypeAccess = (*rootp)->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	(*rootp)->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	(*rootp)->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	(*rootp)->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT], 3, coreID) + 1;

   (*rootp)->bal += increase;    /* update balance factor */

  /************************************************************************
  * re-balance if needed -- height of current tree increases only if its
  * subtree height increases and the current tree needs no rotation.
  ************************************************************************/
   return  (increase  &&  (*rootp)->bal)
      ? (1 - balance(rootp, coreID, type_access))
      : HEIGHT_UNCHANGED;
}/* avl_insert */

/*
* avl_delete() -- delete an item from the given tree
*
*   PARAMETERS:
*                data       --  a pointer to a pointer to the key to delete
*                               On exit, *data points to the deleted data item
*                               (or NULL if deletion failed).
*                rootp      --  a pointer to an AVL tree
*                compar     --  name of function to compare 2 data items
*/
PRIVATE  short
avl_delete(void** data, AVLtree* rootp, int (*compar)(void*, void*, NODE), short coreID, short type_access)
{
   short      decrease;
   int        cmp;
   AVLtree    old_root = *rootp;
   NODE       nd_typ   = node_type(*rootp);
   DIRECTION  dir      = (nd_typ == IS_LBRANCH) ? LEFT : RIGHT;

   if (*rootp == NULL_TREE) {  /* data not found */
      *data = NULL;    /* set return value in data */
      return  HEIGHT_UNCHANGED;
   }/* if */

   cmp = compar(*data, (*rootp)->data, nd_typ);   /* compare data items */

   if (cmp < 0) {  /* delete from left subtree */
      decrease =  -avl_delete(data, &((*rootp)->subtree[LEFT]), compar, coreID, type_access);

      ((mynode*) *data)->l1_child_count[coreID] += (*rootp)->l1_child_count[coreID];
      ((mynode*) *data)->l1_child_count[coreID >> 1] += (*rootp)->l1_child_count[coreID >> 1];
      ((mynode*) *data)->l1_child_count[coreID >> 2] += (*rootp)->l1_child_count[coreID >> 2];

      if (*data == NULL)  return  HEIGHT_UNCHANGED;
   } else if (cmp > 0) {  /* delete from right subtree */
      decrease =  avl_delete(data, &((*rootp)->subtree[RIGHT]), compar, coreID, type_access);
      if (*data == NULL)  return  HEIGHT_UNCHANGED;
   } else {  /* cmp == 0 */
      *data = (*rootp)->data;  /* set return value in data */

      ((mynode*) *data)->l1_child_count[coreID] = (*rootp)->l1_child_count[coreID];
      ((mynode*) *data)->l2_child_count[coreID >> 1] = (*rootp)->l2_child_count[coreID >> 1];
      ((mynode*) *data)->l3_child_count[coreID >> 2] = (*rootp)->l3_child_count[coreID >> 2];

      /***********************************************************************
      *  At this point we know "cmp" is zero and "*rootp" points to
      *  the node that we need to delete.  There are three cases:
      *
      *     1) The node is a leaf.  Remove it and return.
      *
      *     2) The node is a branch (has only 1 child). Make "*rootp"
      *        (the pointer to this node) point to the child.
      *
      *     3) The node has two children. We swap data with the successor of
      *        "*rootp" (the smallest item in its right subtree) and delete
      *        the successor from the right subtree of "*rootp".  The
      *        identifier "decrease" should be reset if the subtree height
      *        decreased due to the deletion of the successor of "rootp".
      ***********************************************************************/

      switch (nd_typ) {  /* what kind of node are we removing? */
      case  IS_LEAF :
         free_node(rootp);          /* free the leaf, its height     */
         return  HEIGHT_CHANGED;    /* changes from 1 to 0, return 1 */

      case  IS_RBRANCH :       /* only child becomes new root */
      case  IS_LBRANCH :
         *rootp = (*rootp)->subtree[dir];
         free_node(&old_root);      /* free the deleted node */
         return  HEIGHT_CHANGED;    /* we just shortened the "dir" subtree */

      case  IS_TREE  :
         decrease = avl_delete(&((*rootp)->data),
                               &((*rootp)->subtree[RIGHT]),
                               avl_min, coreID, type_access);
      case  IS_NULL :
         break;
     } /* switch */
   } /* else */

   (*rootp)->bal -= decrease;       /* update balance factor */

   short rightCoreID, rightTypeAccess, cacheLevel;
   if ((*rootp)->subtree[RIGHT])
   {
      rightCoreID = (*rootp)->subtree[RIGHT]->coreID;
      rightTypeAccess = (*rootp)->subtree[RIGHT]->type_access;
      cacheLevel = getCommonCacheLevel (coreID, rightCoreID, type_access, rightTypeAccess);
   }
   else
   {
      rightCoreID = coreID;
      rightTypeAccess = type_access;
      cacheLevel = 1;
   }

   if (cacheLevel == 1)	(*rootp)->l1_child_count[coreID] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT], 1, coreID) + 1;
   if (cacheLevel == 2)	(*rootp)->l2_child_count[coreID >> 1] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT], 2, coreID) + 1;
   if (cacheLevel == 3)	(*rootp)->l3_child_count[coreID >> 2] = /* avl_children((*rootp)->subtree[LEFT]) + */ avl_children((*rootp)->subtree[RIGHT], 3, coreID) + 1;

  /**********************************************************************
  * Rebalance if necessary -- the height of current tree changes if one
  * of two things happens: (1) a rotation was performed which changed
  * the height of the subtree (2) the subtree height decreased and now
  * matches the height of its other subtree (so the current tree now
  * has a zero balance when it previously did not).
  **********************************************************************/
   if (decrease  &&  (*rootp)->bal) {
      return  balance(rootp, coreID, type_access);  /* rebalance and see if height changed */
   } else if (decrease  && !(*rootp)->bal) {
      return  HEIGHT_CHANGED;  /* balanced because subtree decreased */
   } else {
      return  HEIGHT_UNCHANGED;
   }
}/* avl_delete */



/**
*       Routines which Recursively Traverse an AVL TREE
*
* These routines may perform a particular action function upon each node
* encountered. In these cases, "action" has the following definition:
*
*   void action(data, order, node, level, bal)
*       void   *data
*       VISIT   order;
*       NODE    node;
*       short   bal;
*       int     level;
*
*         "data"    is a pointer to the data field of an AVL node
*         "order"   corresponds to whether this is the 1st, 2nd or 3rd time
*                   that this node has been visited.
*         "node"    indicates which children (if any) of the current node
*                   are null.
*         "level"   is the current level (or depth) in the tree of the
*                   curent node.
*         "bal"     is the balance factor of the current node.
**/


/************************************************************************
*       Walk an AVL tree, performing a given function at each node
************************************************************************/


/*
* avl_walk -- traverse the given tree performing "action"
*            upon each data item encountered.
*
*/
PRIVATE  void
avl_walk(AVLtree tree, void (*action)(void*, VISIT, NODE, int, short, long), SIBLING_ORDER sibling_order, int level)
{
   DIRECTION  dir1 = (sibling_order == LEFT_TO_RIGHT) ? LEFT : RIGHT;
   DIRECTION  dir2 = OPPOSITE(dir1);
   NODE       node = node_type(tree);

   if ((tree != NULL_TREE)  &&  ((void(*)()) action != NULL_ACTION)) {
      (*action)(tree->data, PREORDER, node, level, tree->bal, tree->l1_child_count[0]);

      if (tree->subtree[dir1]  !=  NULL_TREE) {
         avl_walk(tree->subtree[dir1], action, sibling_order, level+1);
      }

      (*action)(tree->data, INORDER, node, level, tree->bal, tree->l1_child_count[0]);

      if (tree->subtree[dir2]  !=  NULL_TREE) {
         avl_walk(tree->subtree[dir2], action, sibling_order, level+1);
      }

      (*action)(tree->data, POSTORDER, node, level, tree->bal, tree->l1_child_count[0]);
   }/* if non-empty tree */

}/* avl_walk */



/************************************************************************
*       Walk an AVL tree, de-allocating space for each node
*       and performing a given function at each node
*       (such as de-allocating the user-defined data item).
************************************************************************/


/*
* avl_free() -- free up space for all nodes in a given tree
*              performing "action" upon each data item encountered.
*
*       (only perform "action" if it is a non-null function)
*/
PRIVATE  void
avl_free(AVLtree* rootp, void (*action)(void*, short, NODE, int), SIBLING_ORDER sibling_order, int level)
{
   DIRECTION  dir1 = (sibling_order == LEFT_TO_RIGHT) ? LEFT : RIGHT;
   DIRECTION  dir2 = OPPOSITE(dir1);
   NODE       node = node_type(*rootp);

   if (*rootp != NULL_TREE) {
      if ((void(*)()) action != NULL_ACTION) {
         (*action)((*rootp)->data, PREORDER, node, level);
      }
      if ((*rootp)->subtree[dir1]  !=  NULL_TREE) {
         avl_free(&((*rootp)->subtree[dir1]), action, sibling_order, level+1);
      }
      if ((void(*)()) action != NULL_ACTION) {
         (*action)((*rootp)->data, INORDER, node, level);
      }
      if ((*rootp)->subtree[dir2]  !=  NULL_TREE) {
         avl_free(&((*rootp)->subtree[dir2]), action, sibling_order, level+1);
      }
      if ((void(*)()) action != NULL_ACTION) {
         (*action)((*rootp)->data, POSTORDER, node, level);
      }
      free(*rootp);
   }/* if non-empty tree */

}/* avl_free */




/**********************************************************************
*
*               C-interface (public functions) for avl trees
*
*       These are the functions that are visible to the user of the
*       AVL Tree Library. Mostly they just return or modify a
*       particular attribute, or Call a private functions with the
*       given parameters.
*
*       Note that public routine names begin with "avl" whereas
*       private routine names that are called by public routines
*       begin with "avl_" (the underscore character is added).
*
*       Each public routine must convert (cast) any argument of the
*       public type "AVL_TREE" to a pointer to on object of the
*       private type "AVLdescriptor" before passing the actual
*       AVL tree to any of the private routines. In this way, the
*       type "AVL_TREE" is implemented as an opaque type.
*
*       An "AVLdescriptor" is merely a container for AVL-tree
*       objects which contains the pointer to the root of the
*       tree and the various attributes of the tree.
*
*       The function types prototypes for the routines which follow
*       are declared in the include file "avl.h"
*
***********************************************************************/



/*
* avlinit() -- get space for an AVL descriptor for the given tree
*              structure and initialize its fields.
*/
PUBLIC  AVL_TREE
avlinit(int (*compar)(void*, void*, NODE), unsigned (*isize)(void*))
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) ckalloc(sizeof (AVLdescriptor));
   avl_desc->root   = NULL_TREE;
   avl_desc->compar = compar;
   avl_desc->isize  = isize;
   avl_desc->count  = 0;

   return  (AVL_TREE) avl_desc;
}/* avlinit */



/*
* avldispose() -- free up all space associated with the given tree structure.
*/
PUBLIC  void
avldispose(AVL_TREE* treeptr, void (*action)(void*, short, NODE, int), SIBLING_ORDER sibling_order)
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) *treeptr;
   avl_free(&(avl_desc->root), action, sibling_order, 1);
   *treeptr = (AVL_TREE) NULL;
}/* avldispose */



/*
* avlwalk() -- traverse the given tree structure and perform the
*              given action on each data item in the tree.
*/
PUBLIC  void
avlwalk(AVL_TREE tree, void (*action)(void*, VISIT, NODE, int, short, long), SIBLING_ORDER sibling_order)
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   avl_walk(avl_desc->root, action, sibling_order, 1);
}/* avlwalk */



/*
* avlcount() --  return the number of nodes in the given tree
*/
PUBLIC  int
avlcount(AVL_TREE tree)
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   return  avl_desc->count;
}/* avlcount */



/*
* avlins() -- insert the given item into the tree structure
*/
PUBLIC  mynode *
avlins(void* data, AVL_TREE tree)
{
   AVLdescriptor  *avl_desc;
   short coreID = ((mynode*) data)->coreID;
   short type_access = ((mynode*) data)->type_access;

   mynode* inserted = NULL;
   avl_desc = (AVLdescriptor *) tree;
   avl_insert(&data,
              (*(avl_desc->isize))(data),
              &(avl_desc->root),
              avl_desc->compar, &inserted, coreID, type_access);
   if (data == NULL)  ++(avl_desc->count);

   return  inserted;
}/* avlins */



/*
* avldel() -- delete the given item from the given tree structure
*/
PUBLIC  void *
avldel(void* data, AVL_TREE tree, int (*compar)(void*, void*, NODE))
{
   AVLdescriptor  *avl_desc;
   short coreID = ((mynode*) data)->coreID;
   short type_access = ((mynode*) data)->type_access;

   avl_desc = (AVLdescriptor *) tree;
   avl_delete(&data, &(avl_desc->root), compar, coreID, type_access);
   if (data != NULL)  --(avl_desc->count);

   return  data;
}/* avldel */



/*
* avlfind() -- find the given item in the given tree structure
*              and return its address (NULL if not found).
*/
PUBLIC  void *
avlfind(void* data, AVL_TREE tree, int (*compar)(void*, void*, NODE))
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   return  avl_find(data, avl_desc->root, compar);
}/* avlfind */



/*
* avldelmin() -- delete the minimal item from the given tree structure
*/
PUBLIC  void *
avldelmin(AVL_TREE tree)
{
   void *data;
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   avl_delete(&data, &(avl_desc->root), avl_min, 0, 0);
   if (data != NULL)  --(avl_desc->count);

   return  data;
}/* avldelmin */



/*
* avlfindmin() -- find the minimal item in the given tree structure
*              and return its address (NULL if not found).
*/
PUBLIC  void *
avlfindmin(AVL_TREE tree)
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   return  avl_find(NULL, avl_desc->root, avl_min);
}/* avlfindmin */



/*
* avldelmax() -- delete the maximal item from the given tree structure
*/
PUBLIC  void *
avldelmax(AVL_TREE tree)
{
   void *data;
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   avl_delete(&data, &(avl_desc->root), avl_max, 0, 0);
   if (data != NULL)  --(avl_desc->count);

   return  data;
}/* avldelmax */



/*
* avlfindmax() -- find the maximal item in the given tree structure
*              and return its address (NULL if not found).
*/
PUBLIC  void *
avlfindmax (AVL_TREE tree)
{
   AVLdescriptor  *avl_desc;

   avl_desc = (AVLdescriptor *) tree;
   return  avl_find(NULL, avl_desc->root, avl_max);
}/* avlfindmax */

