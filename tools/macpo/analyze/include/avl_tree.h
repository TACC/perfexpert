
/* Adapted from: http://www.zentut.com/c-tutorial/c-avl-tree/ */

#ifndef AVLTREE_H_
#define AVLTREE_H_

#include <map>

#include "macpo_record.h"

class avl_tree {
    public:
        avl_tree();
        void print();
        void destroy();
        bool contains(size_t cache_line);
        long long get_distance(size_t cache_line);
        void set_distance(size_t cache_line, long long distance);
        bool insert(const mem_info_t *data_ptr, long long delta = 0);

    private:
        typedef struct tag_avl_node {
            int height;
            mem_info_t data;
            size_t key;
            struct tag_avl_node *left, *right;
        } avl_node_t;

        void _print(avl_node_t* n);
        int max(avl_node_t* n);
        int height(avl_node_t* n);
        void _destroy(avl_node_t* t);
        avl_node_t* _insert(avl_node_t** t, avl_node_t* node_to_insert);

        avl_node_t* double_rotate_with_left(avl_node_t* k3);
        avl_node_t* double_rotate_with_right(avl_node_t* k1);
        avl_node_t* single_rotate_with_left(avl_node_t* k2);
        avl_node_t* single_rotate_with_right(avl_node_t* k1);

        avl_node_t* root;
        size_t last_key;

        std::map<size_t, avl_node_t*> addr_to_node;
};

#endif  /* AVLTREE_H_ */
