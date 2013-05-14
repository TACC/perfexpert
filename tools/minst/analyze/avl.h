/** <plaintext>
*
*  avl.h -- public types and external declarations for avl trees
*
*  Created 03/01/89 by Brad Appleton
*
* ^{Mods:* }
*
* Fri Jul 14 13:54:12 1989, Rev 1.0, brad(0165)
*
**/

#ifndef AVL_H
#define AVL_H

#ifdef __STDC__
# define _P(x)  x
#else
# define _P(x)  ()
#endif

       /* definition of traversal type */
typedef  enum  { PREORDER, INORDER, POSTORDER }  VISIT;


       /* definition of sibling order type */
typedef  enum  { LEFT_TO_RIGHT, RIGHT_TO_LEFT }  SIBLING_ORDER;


       /* definition of node type */
typedef  enum  { IS_TREE, IS_LBRANCH, IS_RBRANCH, IS_LEAF, IS_NULL }  NODE;


       /* definition of opaque type for AVL trees */
typedef  void  *AVL_TREE;


#include "structures.h"
#ifndef NEXTERN

     /* Constructor and Destructor functions for AVL trees:
     *          avlfree is a macro for avldispose in the fashion
     *          of free(). It assumes certain default values
     *          (shown below) for the deallocation function and
     *          for the order in which children are traversed.
     */
extern AVL_TREE     avlinit    _P((int(*) (void*, void*, NODE), unsigned(*)(void*)));
extern void         avldispose _P((AVL_TREE *, void (*)(void*, short, NODE, int), SIBLING_ORDER));
#define avlfree(x)  avldispose _P(&(x), free, LEFT_TO_RIGHT)


       /* Routine for manipulating/accessing each data item in a tree */
extern void      avlwalk  _P((AVL_TREE, void(*)(void*, VISIT, NODE, int, short, long), SIBLING_ORDER));


       /* Routine for obtaining the size of an AVL tree */
extern int       avlcount  _P((AVL_TREE));


       /* Routines to search for a given item */
extern mynode   *avlins  _P((void *, AVL_TREE));
extern void     *avldel  _P((void *, AVL_TREE, int(*)(void*, void*, NODE)));
extern void     *avlfind _P((void *, AVL_TREE, int(*)(void*, void*, NODE)));


       /* Routines to search for the minimal item of a tree */
extern void     *avldelmin  _P((AVL_TREE));
extern void     *avlfindmin _P((AVL_TREE));


       /* Routines to search for the maximal item of a tree */
extern void     *avldelmax  _P((AVL_TREE));
extern void     *avlfindmax _P((AVL_TREE));

#endif /* NEXTERN */

#undef _P
#endif /* AVL_H */
