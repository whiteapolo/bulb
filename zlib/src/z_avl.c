#include <internal/z_avl.h>
#include <math.h>

#define Z_AVL_TREE_NULL_ID 0

char z_avl_tree_get_height(const Z_Avl_Tree *tree, size_t node_id);
void z_avl_tree_update_node_height(const Z_Avl_Tree *tree, size_t node_id);
int z_avl_tree_get_node_balance_factor(const Z_Avl_Tree *tree, size_t node_id);
size_t z_avl_tree_right_rotate(Z_Avl_Tree *tree, size_t root_id);
size_t z_avl_tree_left_rotate(Z_Avl_Tree *tree, size_t root_id);
size_t z_avl_tree_left_right_rotate(Z_Avl_Tree *tree, size_t root_id);
size_t z_avl_tree_right_left_rotate(Z_Avl_Tree *tree, size_t root_id);
size_t z_avl_tree_rebalance_node(Z_Avl_Tree *tree, size_t node_id);
size_t z_avl_tree_generate_next_id(Z_Avl_Tree *tree);
size_t z_avl_tree_new_node(Z_Avl_Tree *tree, void *key, void *value);
void z_avl_tree_free_node(Z_Avl_Tree *tree, size_t node_id);
size_t z_avl_tree_get_min_node(const Z_Avl_Tree *tree, size_t node_id);
size_t z_avl_tree_get_node(const Z_Avl_Tree *tree, const void *key);
size_t z_avl_tree_put_impl(Z_Avl_Tree *tree, size_t node_id, void *key, void *value, Z_Maybe_Pair *pair);
size_t z_avl_tree_delete_impl(Z_Avl_Tree *tree, size_t node_id, void *key, Z_Maybe_Pair *pair);
bool z_avl_tree_is_healthy(const Z_Avl_Tree *tree);
static inline Z_Avl_Node *z_avl_tree_node_by_id(const Z_Avl_Tree *tree, size_t node_id);
bool z_avl_tree_is_healthy_impl(const Z_Avl_Tree *tree, size_t node_id);

size_t z_avl_tree_size(const Z_Avl_Tree *tree)
{
    return tree->nodes.length - tree->free_list.length;
}

static inline Z_Avl_Node *z_avl_tree_node_by_id(const Z_Avl_Tree *tree, size_t node_id)
{
    return &tree->nodes.ptr[node_id - 1];
}

char z_avl_tree_get_height(const Z_Avl_Tree *tree, size_t node_id)
{
    if (node_id == Z_AVL_TREE_NULL_ID) {
        return 0;
    }

    return z_avl_tree_node_by_id(tree, node_id)->height;
}

void z_avl_tree_update_node_height(const Z_Avl_Tree *tree, size_t node_id)
{
    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, node_id);
    node->height = 1 + (char)Z_MAX(z_avl_tree_get_height(tree, node->right), z_avl_tree_get_height(tree, node->left));
}

int z_avl_tree_get_node_balance_factor(const Z_Avl_Tree *tree, size_t node_id)
{
    if (node_id == Z_AVL_TREE_NULL_ID) {
        return 0;
    }

    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, node_id);
    return z_avl_tree_get_height(tree, node->left) - z_avl_tree_get_height(tree, node->right);
}

size_t z_avl_tree_left_rotate(Z_Avl_Tree *tree, size_t root_id)
{
    Z_Avl_Node *root = z_avl_tree_node_by_id(tree, root_id);
    size_t pivot_id = root->right;
    Z_Avl_Node *pivot = z_avl_tree_node_by_id(tree, pivot_id);

    size_t pivot_left_id = pivot->left;
    pivot->left = root_id;
    root->right = pivot_left_id;

    z_avl_tree_update_node_height(tree, root_id);
    z_avl_tree_update_node_height(tree, pivot_id);

    return pivot_id;
}

size_t z_avl_tree_right_rotate(Z_Avl_Tree *tree, size_t root_id)
{
    Z_Avl_Node *root = z_avl_tree_node_by_id(tree, root_id);
    size_t pivot_id = root->left;
    Z_Avl_Node *pivot = z_avl_tree_node_by_id(tree, pivot_id);

    size_t pivot_right_id = pivot->right;
    pivot->right = root_id;
    root->left = pivot_right_id;

    z_avl_tree_update_node_height(tree, root_id);
    z_avl_tree_update_node_height(tree, pivot_id);

    return pivot_id;
}

size_t z_avl_tree_left_right_rotate(Z_Avl_Tree *tree, size_t root_id)
{
    size_t new_id = z_avl_tree_left_rotate(tree, z_avl_tree_node_by_id(tree, root_id)->left);
    return z_avl_tree_right_rotate(tree, new_id);
}

size_t z_avl_tree_right_left_rotate(Z_Avl_Tree *tree, size_t root_id)
{
    size_t new_id = z_avl_tree_right_rotate(tree, z_avl_tree_node_by_id(tree, root_id)->right);
    return z_avl_tree_left_rotate(tree, new_id);
}

size_t z_avl_tree_rebalance_node(Z_Avl_Tree *tree, size_t node_id)
{
    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, node_id);
    z_avl_tree_update_node_height(tree, node_id);
    int balance_factor = z_avl_tree_get_node_balance_factor(tree, node_id);

    if (balance_factor > 1 && z_avl_tree_get_node_balance_factor(tree, node->left) >= 0) {
        return z_avl_tree_right_rotate(tree, node_id);
    } else if (balance_factor < -1 && z_avl_tree_get_node_balance_factor(tree, node->right) <= 0) {
        return z_avl_tree_left_rotate(tree, node_id);
    } else if (balance_factor > 1 && z_avl_tree_get_node_balance_factor(tree, node->left) < 0) {
        return z_avl_tree_left_right_rotate(tree, node_id);
    } else if (balance_factor < -1 && z_avl_tree_get_node_balance_factor(tree, node->right) > 0) {
        return z_avl_tree_right_left_rotate(tree, node_id);
    }

    return node_id;
}

size_t z_avl_tree_generate_next_id(Z_Avl_Tree *tree)
{
    if (tree->free_list.length > 0) {
        return z_array_pop(&tree->free_list);
    }

    tree->nodes.length++;
    z_array_ensure_capacity(&tree->nodes, tree->nodes.length);
    return tree->nodes.length;
}

size_t z_avl_tree_new_node(Z_Avl_Tree *tree, void *key, void *value)
{
    size_t id = z_avl_tree_generate_next_id(tree);
    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, id);

    node->key = key;
    node->value = value;
    node->height = 0;
    node->right = Z_AVL_TREE_NULL_ID;
    node->left = Z_AVL_TREE_NULL_ID;
    
    return id;
}

void z_avl_tree_free_node(Z_Avl_Tree *tree, size_t node_id)
{
    z_array_push(&tree->free_list, node_id);
}

Z_Avl_Tree z_avl_tree_new(Z_Heap *heap, Z_Compare_Fn compare_keys)
{
    Z_Avl_Tree tree = {
        .root = Z_AVL_TREE_NULL_ID,
        .nodes = z_array_new(heap, Z_Avl_Node_Array),
        .free_list = z_array_new(heap, Z_Avl_Id_Array),
        .compare_keys = compare_keys,
    };

    return tree;
}

size_t z_avl_tree_get_min_node(const Z_Avl_Tree *tree, size_t node_id)
{
    size_t last = node_id;
    size_t lookahead = z_avl_tree_node_by_id(tree, node_id)->left;

    while (lookahead != Z_AVL_TREE_NULL_ID) {
        last = lookahead;
        lookahead = z_avl_tree_node_by_id(tree, lookahead)->left;
    }

    return last;
}

Z_Maybe_Pair z_avl_tree_get_max(const Z_Avl_Tree *tree)
{
    if (tree->root == Z_AVL_TREE_NULL_ID) {
        Z_Maybe_Pair result = { .ok = false };
        return result;
    }

    size_t curr = tree->root;
    size_t lookahead = z_avl_tree_node_by_id(tree, tree->root)->right;

    while (lookahead != Z_AVL_TREE_NULL_ID) {
        curr = lookahead;
        lookahead = z_avl_tree_node_by_id(tree, lookahead)->right;
    }

    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, curr);

    Z_Maybe_Pair result = {
        .ok = true,
        .pair = z_make_pair(node->key, node->value),
    };

    return result;
}

Z_Maybe_Pair z_avl_tree_get_min(const Z_Avl_Tree *tree)
{
    if (tree->root == Z_AVL_TREE_NULL_ID) {
        Z_Maybe_Pair result = { .ok = false };
        return result;
    }

    size_t curr = tree->root;
    size_t lookahead = z_avl_tree_node_by_id(tree, tree->root)->left;

    while (lookahead != Z_AVL_TREE_NULL_ID) {
        curr = lookahead;
        lookahead = z_avl_tree_node_by_id(tree, lookahead)->left;
    }

    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, curr);

    Z_Maybe_Pair result = {
        .ok = true,
        .pair = z_make_pair(node->key, node->value),
    };

    return result;
}

size_t z_avl_tree_get_node(const Z_Avl_Tree *tree, const void *key)
{
    size_t curr = tree->root;

    while (curr != Z_AVL_TREE_NULL_ID) {
        Z_Avl_Node *node = z_avl_tree_node_by_id(tree, curr);
        int compare_result = tree->compare_keys(key, node->key);

        if (compare_result > 0) {
            curr = node->right;
        } else if (compare_result < 0) {
            curr = node->left;
        } else {
            return curr;
        }
    }

    return curr;
}

const void *z_avl_tree_try_get(const Z_Avl_Tree *tree, const void *key, const void *fallback)
{
    size_t node_id = z_avl_tree_get_node(tree, key);

    if (node_id == Z_AVL_TREE_NULL_ID) {
        return fallback;
    }

    return z_avl_tree_node_by_id(tree, node_id)->value;
}

const void *z_avl_tree_get(const Z_Avl_Tree *tree, const void *key)
{
    return z_avl_tree_try_get(tree, key, NULL);
}

bool z_avl_tree_contains(const Z_Avl_Tree *tree, const void *key)
{
    return z_avl_tree_get_node(tree, key) != Z_AVL_TREE_NULL_ID;
}

size_t z_avl_tree_put_impl(Z_Avl_Tree *tree, size_t node_id, void *key, void *value, Z_Maybe_Pair *pair)
{
    if (node_id == Z_AVL_TREE_NULL_ID) {
        pair->ok = false;
        return z_avl_tree_new_node(tree, key, value);
    }

    int compare_result = tree->compare_keys(key, z_avl_tree_node_by_id(tree, node_id)->key);

    if (compare_result > 0) {
        size_t new_right = z_avl_tree_put_impl(tree, z_avl_tree_node_by_id(tree, node_id)->right, key, value, pair);;
        z_avl_tree_node_by_id(tree, node_id)->right = new_right;
    } else if (compare_result < 0) {
        size_t new_left = z_avl_tree_put_impl(tree, z_avl_tree_node_by_id(tree, node_id)->left, key, value, pair);;
        z_avl_tree_node_by_id(tree, node_id)->left = new_left;
    } else {
        pair->ok = true;
        pair->pair.key = z_avl_tree_node_by_id(tree, node_id)->key;
        pair->pair.value = z_avl_tree_node_by_id(tree, node_id)->value;
        z_avl_tree_node_by_id(tree, node_id)->key = key;
        z_avl_tree_node_by_id(tree, node_id)->value = value;
        return node_id;
    }

    return z_avl_tree_rebalance_node(tree, node_id);
}

Z_Maybe_Pair z_avl_tree_put(Z_Avl_Tree *tree, void *key, void *value)
{
    Z_Maybe_Pair pair;
    tree->root = z_avl_tree_put_impl(tree, tree->root, key, value, &pair);
    return pair;
}

size_t z_avl_tree_delete_impl(Z_Avl_Tree *tree, size_t node_id, void *key, Z_Maybe_Pair *pair)
{
    if (node_id == Z_AVL_TREE_NULL_ID) {
        pair->ok = false;
        return node_id;
    }

    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, node_id);
    int compare_result = tree->compare_keys(key, node->key);

    if (compare_result > 0) {
        node->right = z_avl_tree_delete_impl(tree, node->right, key, pair);
    } else if (compare_result < 0) {
        node->left = z_avl_tree_delete_impl(tree, node->left, key, pair);
    } else {
        if (node->left == Z_AVL_TREE_NULL_ID) {
            pair->ok = true;
            pair->pair = z_make_pair(node->key, node->value);
            z_avl_tree_free_node(tree, node_id);
            return node->right;
        } else if (node->right == Z_AVL_TREE_NULL_ID) {
            pair->ok = true;
            pair->pair = z_make_pair(node->key, node->value);
            z_avl_tree_free_node(tree, node_id);
            return node->left;
        }

        size_t successor_id = z_avl_tree_get_min_node(tree, node->right);
        Z_Avl_Node *successor = z_avl_tree_node_by_id(tree, successor_id);
        node->key = successor->key;
        node->value = successor->value;
        z_avl_tree_delete_impl(tree, node->right, successor->key, pair);
    }

    return z_avl_tree_rebalance_node(tree, node_id);
}

Z_Maybe_Pair z_avl_tree_delete(Z_Avl_Tree *tree, void *key)
{
    Z_Maybe_Pair pair = {0};
    tree->root = z_avl_tree_delete_impl(tree, tree->root, key, &pair);
    return pair;
}

// Z_Avl_tree_Iter z_avl_tree_iter(Z_Heap *heap, const Z_Avl_Tree *tree)
// {
//     Z_Avl_tree_Iter iter = {
//         .tree = tree,
//         .did_visit_left = false,
//         .did_visit_curr = false,
//         .did_visit_right = false,
//         .stack = z_array_new(heap, Z_Avl_Id_Array),
//     };

//     size_t curr = tree->root;

//     while (curr != Z_AVL_TREE_NULL_ID) {
//         z_array_push(&iter.stack, curr);
//         curr = z_avl_tree_node_by_id(tree, curr)->left;
//     }

//     return iter;
// }

// bool z_avl_tree_iter_next(Z_Avl_tree_Iter *iter, Z_Pair *pair)
// {

// }

Z_Pair_Array z_avl_tree_to_array(Z_Heap *heap, const Z_Avl_Tree *tree)
{
    Z_Pair_Array array = z_array_new(heap, Z_Pair_Array);

    for (size_t i = 0; i < tree->nodes.length; i++) {
        z_array_push(&array, z_make_pair(tree->nodes.ptr[i].key, tree->nodes.ptr[i].value));
    }

    return array;
}

bool z_avl_tree_is_healthy_impl(const Z_Avl_Tree *tree, size_t node_id)
{
    if (node_id == Z_AVL_TREE_NULL_ID) {
        return true;
    }

    int bf = z_avl_tree_get_node_balance_factor(tree, node_id);
    Z_Avl_Node *node = z_avl_tree_node_by_id(tree, node_id);

    if (bf < -1 || bf > 1) {
        return false;
    }

    return z_avl_tree_is_healthy_impl(tree, node->left) && z_avl_tree_is_healthy_impl(tree, node->right);
}

bool z_avl_tree_is_healthy(const Z_Avl_Tree *tree)
{
    return z_avl_tree_is_healthy_impl(tree, tree->root);
}