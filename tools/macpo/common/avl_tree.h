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

#if 0
    Adapted from: www.zentut.com/c-tutorial/c-avl-tree/
#endif

#ifndef TOOLS_MACPO_COMMON_AVL_TREE_H_
#define TOOLS_MACPO_COMMON_AVL_TREE_H_

#include <stdint.h>

#include <cassert>
#include <iostream>
#include <map>

#include "generic_defs.h"
#include "macpo_record.h"

#define MAX(a, b)    ((a) > (b) ? (a) : (b))

class avl_tree {
 public:
    avl_tree() {
        root = NULL;
        last_key = 0;
    }

    void print() {
        _print(root);
    }

    void destroy() {
        _destroy(root);
    }

    bool contains(size_t cache_line) {
        std::map<size_t, avl_node_t*>::iterator it =
            addr_to_node.find(cache_line);
        return (it != addr_to_node.end());
    }

    int64_t get_distance(size_t cache_line) {
        std::map<size_t, avl_node_t*>::iterator it =
            addr_to_node.find(cache_line);
        if (it != addr_to_node.end()) {
            avl_node_t* node = addr_to_node[cache_line];
            if (node) {
                assert(ADDR_TO_CACHE_LINE(node->data.address) == cache_line);
                return last_key - node->key - 1;
            }
        }

        // Error
        return -1;
    }

    void set_distance(size_t cache_line, int64_t distance) {
        std::map<size_t, avl_node_t*>::iterator it =
            addr_to_node.find(cache_line);
        if (it != addr_to_node.end()) {
            avl_node_t* node = addr_to_node[cache_line];
            if (node) {
                node->key = last_key - distance - 1;
            }
        }
    }

    bool insert(const mem_info_t *data_ptr, int64_t delta = 0) {
        if (data_ptr) {
            avl_node_t* node_to_insert = NULL;
            node_to_insert = (avl_node_t*) malloc(sizeof(avl_node_t));
            if (node_to_insert) {
                node_to_insert->data = *data_ptr;
                node_to_insert->height = 0;
                node_to_insert->key = last_key - delta;
                node_to_insert->left = node_to_insert->right = NULL;

                avl_node_t* node = _insert(&root, node_to_insert);
                if (node) {
                    addr_to_node[ADDR_TO_CACHE_LINE(data_ptr->address)] =
                        node_to_insert;
                    last_key += 1;
                    return true;
                }
            }
        }

        return false;
    }

 private:
    typedef struct tag_avl_node {
        int height;
        mem_info_t data;
        size_t key;
        struct tag_avl_node *left, *right;
    } avl_node_t;

    void _print(avl_node_t* n) {
        if (n == NULL) {
            return;
        }

        std::cout << &(n->data);

        if (n->left != NULL) {
            std::cout << "(L:" << &(n->left->data) << ")";
        }

        if (n->right != NULL) {
            std::cout << "(R:" << &(n->right->data) << ")";
        }

        std::cout << std::endl;

        _print(n->left);
        _print(n->right);
    }

    int height(avl_node_t* n) {
        if (n == NULL) {
            return -1;
        } else {
            return n->height;
        }
    }

    void _destroy(avl_node_t* t) {
        if (t != NULL) {
            _destroy(t->left);
            _destroy(t->right);
            free(t);
        }
    }

    avl_node_t* _insert(avl_node_t** t, avl_node_t* node_to_insert) {
        if (*t == NULL) {
            *t = node_to_insert;
        } else if (node_to_insert->key < (*t)->key) {
            (*t)->left = _insert(&((*t)->left), node_to_insert);
            if (height((*t)->left) - height((*t)->right) == 2)
                if (node_to_insert->key < (*t)->left->key) {
                    *t = single_rotate_with_left(*t);
                } else {
                    *t = double_rotate_with_left(*t);
                }
        } else if (node_to_insert->key > (*t)->key) {
            (*t)->right = _insert(&((*t)->right), node_to_insert);
            if (height((*t)->right) - height((*t)->left) == 2)
                if (node_to_insert->key > (*t)->right->key) {
                    *t = single_rotate_with_right(*t);
                } else {
                    *t = double_rotate_with_right(*t);
                }
        }

        (*t)->height = MAX(height((*t)->left), height((*t)->right)) + 1;
        return *t;
    }


    avl_node_t* double_rotate_with_left(avl_node_t* k3) {
        /* Rotate between k1 and k2 */
        k3->left = single_rotate_with_right(k3->left);

        // if (k3->left)   addr_to_node[k3->left->data.address] = k3->left;

        /* Rotate between K3 and k2 */
        return single_rotate_with_left(k3);
    }

    avl_node_t* double_rotate_with_right(avl_node_t* k1) {
        /* rotate between K3 and k2 */
        k1->right = single_rotate_with_left(k1->right);

        // if (k1->right)  addr_to_node[k1->right->data.address] = k1->right;

        /* rotate between k1 and k2 */
        return single_rotate_with_right(k1);
    }

    avl_node_t* single_rotate_with_left(avl_node_t* k2) {
        avl_node_t* k1 = NULL;

        k1 = k2->left;
        k2->left = k1->right;
        k1->right = k2;

        k2->height = MAX(height(k2->left), height(k2->right)) + 1;
        k1->height = MAX(height(k1->left), k2->height) + 1;

        if (k1)         addr_to_node[ADDR_TO_CACHE_LINE(k1->data.address)] = k1;
        if (k2)         addr_to_node[ADDR_TO_CACHE_LINE(k2->data.address)] = k2;

        avl_node_t* k2l = k2->left;
        avl_node_t* k1r = k1->right;

        if (k1r) {
            addr_to_node[ADDR_TO_CACHE_LINE(k1r->data.address)] = k1r;
        }

        if (k2l) {
            addr_to_node[ADDR_TO_CACHE_LINE(k2l->data.address)] = k2l;
        }

        return k1; /* new root */
    }

    avl_node_t* single_rotate_with_right(avl_node_t* k1) {
        avl_node_t* k2;

        k2 = k1->right;
        k1->right = k2->left;
        k2->left = k1;

        k1->height = MAX(height(k1->left), height(k1->right)) + 1;
        k2->height = MAX(height(k2->right), k1->height) + 1;

        if (k1)         addr_to_node[ADDR_TO_CACHE_LINE(k1->data.address)] = k1;
        if (k2)         addr_to_node[ADDR_TO_CACHE_LINE(k2->data.address)] = k2;

        avl_node_t* k2l = k2->left;
        avl_node_t* k1r = k1->right;

        if (k1r) {
            addr_to_node[ADDR_TO_CACHE_LINE(k1r->data.address)] = k1r;
        }

        if (k2l) {
            addr_to_node[ADDR_TO_CACHE_LINE(k2l->data.address)] = k2l;
        }

        return k2;  /* New root */
    }

    avl_node_t* root;
    size_t last_key;

    std::map<size_t, avl_node_t*> addr_to_node;
};

#endif  // TOOLS_MACPO_COMMON_AVL_TREE_H_
