#ifndef AVL_H
#define AVL_H

#include <z_types.h>
#include <z_array.h>
#include <z_compare.h>
#include <z_heap.h>
#include <stdbool.h>

typedef struct Z_Avl_Node {
    size_t right;
    size_t left;
    void *key;
    void *value;
    char height;
} Z_Avl_Node;

Z_DEFINE_ARRAY(Z_Avl_Node_Array, Z_Avl_Node);
Z_DEFINE_ARRAY(Z_Avl_Id_Array, size_t);

typedef struct {
    size_t root;
    Z_Avl_Node_Array nodes;
    Z_Avl_Id_Array free_list;
    Z_Compare_Fn compare_keys;
} Z_Avl_Tree;

typedef struct {
    const Z_Avl_Tree *tree;
    Z_Avl_Id_Array stack;
    bool did_visit_left;
    bool did_visit_curr;
    bool did_visit_right;
} Z_Avl_tree_Iter;

typedef struct {
    char operator;
    void *value;
} Z_Avl_Where_Condition;

typedef enum {
    Z_Avl_Tree_Order_By_ASC,
    Z_Avl_Tree_Order_By_DESC,
} Z_Avl_Tree_Order_By;

Z_Avl_Tree z_avl_tree_new(Z_Heap *heap, Z_Compare_Fn compare_keys);
const void *z_avl_tree_try_get(const Z_Avl_Tree *tree, const void *key, const void *fallback);
const void *z_avl_tree_get(const Z_Avl_Tree *tree, const void *key);
bool z_avl_tree_contains(const Z_Avl_Tree *tree, const void *key);
Z_Maybe_Pair z_avl_tree_put(Z_Avl_Tree *tree, void *key, void *value);
Z_Maybe_Pair z_avl_tree_delete(Z_Avl_Tree *tree, void *key);
size_t z_avl_tree_size(const Z_Avl_Tree *tree);
Z_Pair_Array z_avl_tree_to_array(Z_Heap *heap, const Z_Avl_Tree *tree);
Z_Maybe_Pair z_avl_tree_get_max(const Z_Avl_Tree *tree);
Z_Maybe_Pair z_avl_tree_get_min(const Z_Avl_Tree *tree);

bool z_avl_tree_is_healthy(const Z_Avl_Tree *tree);

Z_Avl_tree_Iter z_avl_tree_iter(Z_Heap *heap, const Z_Avl_Tree *tree, Z_Avl_Where_Condition where, Z_Avl_Tree_Order_By order_by);
bool z_avl_tree_iter_next(Z_Avl_tree_Iter *iter, Z_Pair *pair);

// Z_Avl_tree_Iter z_avl_tree_iter_where(Z_Heap *heap, const Z_Avl_Tree *tree, );


#endif
