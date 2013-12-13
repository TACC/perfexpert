
/* Adapted from: http://www.zentut.com/c-tutorial/c-avl-tree/ */

#include <cassert>
#include <cstdlib>
#include <iostream>

#include "avl_tree.h"

#define MAX(a,b)    ((a) > (b) ? (a) : (b))

avl_tree::avl_tree() {
    root = NULL;
    last_key = 0;
}

void avl_tree::destroy() {
    _destroy(root);
}

void avl_tree::_destroy(avl_node_t* t) {
    if(t != NULL) {
        _destroy(t->left);
        _destroy(t->right);
        free(t);
    }
}

bool avl_tree::contains(size_t address) {
    std::map<size_t, avl_node_t*>::iterator it = addr_to_node.find(address);
    return (it != addr_to_node.end());
}

void avl_tree::set_distance(size_t address, long long distance) {
    std::map<size_t, avl_node_t*>::iterator it = addr_to_node.find(address);
    if (it != addr_to_node.end()) {
        avl_node_t* node = addr_to_node[address];
        if (node) {
            node->key = last_key - distance - 1;
        }
    }
}

long long avl_tree::get_distance(size_t address) {
    std::map<size_t, avl_node_t*>::iterator it = addr_to_node.find(address);
    if (it != addr_to_node.end()) {
        avl_node_t* node = addr_to_node[address];
        if (node) {
            assert(node->data.address == address);
            return last_key - node->key - 1;
        }
    }

    // Error
    return -1;
}

int avl_tree::height(avl_node_t* n) {
    if(n == NULL) {
        return -1;
    } else {
        return n->height;
    }
}

avl_tree::avl_node_t* avl_tree::single_rotate_with_left(avl_node_t* k2) {
    avl_node_t* k1 = NULL;

    k1 = k2->left;
    k2->left = k1->right;
    k1->right = k2;

    k2->height = MAX(height(k2->left), height(k2->right)) + 1;
    k1->height = MAX(height(k1->left), k2->height) + 1;

    if (k1)         addr_to_node[k1->data.address] = k1;
    if (k2)         addr_to_node[k2->data.address] = k2;
    if (k1->right)  addr_to_node[k1->right->data.address] = k1->right;
    if (k2->left)   addr_to_node[k2->left->data.address] = k2->left;

    return k1; /* new root */
}

avl_tree::avl_node_t* avl_tree::single_rotate_with_right(avl_node_t* k1) {
    avl_node_t* k2;

    k2 = k1->right;
    k1->right = k2->left;
    k2->left = k1;

    k1->height = MAX(height(k1->left), height(k1->right)) + 1;
    k2->height = MAX(height(k2->right), k1->height) + 1;

    if (k1)         addr_to_node[k1->data.address] = k1;
    if (k2)         addr_to_node[k2->data.address] = k2;
    if (k1->right)  addr_to_node[k1->right->data.address] = k1->right;
    if (k2->left)   addr_to_node[k2->left->data.address] = k2->left;

    return k2;  /* New root */
}

avl_tree::avl_node_t* avl_tree::double_rotate_with_left(avl_node_t* k3) {
    /* Rotate between k1 and k2 */
    k3->left = single_rotate_with_right(k3->left);

    // if (k3->left)   addr_to_node[k3->left->data.address] = k3->left;

    /* Rotate between K3 and k2 */
    return single_rotate_with_left(k3);
}

avl_tree::avl_node_t* avl_tree::double_rotate_with_right(avl_node_t* k1) {
    /* rotate between K3 and k2 */
    k1->right = single_rotate_with_left(k1->right);

    // if (k1->right)  addr_to_node[k1->right->data.address] = k1->right;

    /* rotate between k1 and k2 */
    return single_rotate_with_right(k1);
}

bool avl_tree::insert(const mem_info_t *data_ptr, long long delta) {
    if (data_ptr) {
        avl_node_t* node_to_insert = (avl_node_t*) malloc(sizeof(avl_node_t));
        if (node_to_insert) {
            node_to_insert->data = *data_ptr;
            node_to_insert->height = 0;
            node_to_insert->key = last_key - delta;
            node_to_insert->left = node_to_insert->right = NULL;

            avl_node_t* node = _insert(&root, node_to_insert);
            if (node) {
                addr_to_node[data_ptr->address] = node;
                last_key += 1;
                return true;
            }
        }
    }

    return false;
}

avl_tree::avl_node_t* avl_tree::_insert(avl_node_t** t,
        avl_node_t* node_to_insert) {
    if(*t == NULL) {
        *t = node_to_insert;
    } else if(node_to_insert->key < (*t)->key) {
        (*t)->left = _insert(&((*t)->left), node_to_insert);
        if(height((*t)->left) - height((*t)->right) == 2)
            if(node_to_insert->key < (*t)->left->key) {
                *t = single_rotate_with_left(*t);
            } else {
                *t = double_rotate_with_left(*t);
            }
    } else if(node_to_insert->key > (*t)->key) {
        (*t)->right = _insert(&((*t)->right), node_to_insert);
        if(height((*t)->right) - height((*t)->left) == 2)
            if(node_to_insert->key > (*t)->right->key) {
                *t = single_rotate_with_right(*t);
            } else {
                *t = double_rotate_with_right(*t);
            }
    }

    (*t)->height = MAX(height((*t)->left), height((*t)->right)) + 1;
    return *t;
}

void avl_tree::print() {
    _print(root);
}

void avl_tree::_print(avl_node_t* t)
{
    if (t == NULL) {
        return;
    }

    std::cout << &(t->data);

    if(t->left != NULL) {
        std::cout << "(L:" << &(t->left->data) << ")";
    }

    if(t->right != NULL) {
        std::cout << "(R:" << &(t->right->data) << ")";
    }

    std::cout << std::endl;

    _print(t->left);
    _print(t->right);
}
