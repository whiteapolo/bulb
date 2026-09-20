#include <z_hash_table.h>
#include <z_min_max.h>

#define Z__HASH_TABLE_EMPTY 0
#define Z__HASH_TABLE_TOMBSTONE 1

void z__hash_table_free(Z_Hash_Table *ht);
Z_Maybe_Pair z__hash_table_put_no_resize(Z_Hash_Table *ht, void *key, void *value, size_t hash);
void z__hash_table_resize(Z_Hash_Table *ht, size_t new_capacity);

Z_Hash_Table z_hash_table_new(Z_Heap *heap, Z_Equal_Fn equal, Z_Hash_Fn hash)
{
    return z_hash_table_new_with_capacity(heap, equal, hash, 0);
}

Z_Hash_Table z_hash_table_new_with_capacity(Z_Heap *heap, Z_Equal_Fn equal, Z_Hash_Fn hash, size_t capacity)
{
    Z_Hash_Table table = {
        .keys = z_heap_calloc(heap, sizeof(void *) * capacity),
        .values = z_heap_calloc(heap, sizeof(void *) * capacity),
        .hashes = z_heap_calloc(heap, sizeof(size_t) * capacity),
        .occupied = 0,
        .size = 0,
        .capacity = capacity,
        .equal = equal,
        .hash = hash,
        .heap = heap,
    };

    return table;
}

void z__hash_table_free(Z_Hash_Table *ht)
{
    if (ht->capacity == 0) {
        return;
    }

    z_heap_free(ht->heap, ht->keys);
    z_heap_free(ht->heap, ht->values);
    z_heap_free(ht->heap, ht->hashes);
}

static inline size_t z__hash_table_hash(const Z_Hash_Table *ht, const void *key)
{
    size_t hash = ht->hash(key);

    if (hash < 2) {
        return hash + 2;
    }

    return hash;
}

static inline float z__hash_table_get_load_factor(const Z_Hash_Table *ht)
{
    if (ht->capacity == 0) {
        return 1;
    }

    return (float)ht->occupied / (float)ht->capacity;
}

void *z_hash_table_try_get(const Z_Hash_Table *ht, const void *key, void *fallback)
{
    if (ht->capacity == 0) {
        return fallback;
    }

    size_t hash = z__hash_table_hash(ht, key);
    size_t i = hash % ht->capacity;

    while (ht->hashes[i] != Z__HASH_TABLE_EMPTY) {

        if (ht->hashes[i] == hash && ht->equal(ht->keys[i], key)) {
            return ht->values[i];
        }

        i = (i + 1) % ht->capacity;
    }

    return fallback;
}

void *z_hash_table_get(const Z_Hash_Table *ht, const void *key)
{
    return z_hash_table_try_get(ht, key, NULL);
}

Z_Maybe_Pair z__hash_table_put_no_resize(Z_Hash_Table *ht, void *key, void *value, size_t hash)
{
    size_t i = hash % ht->capacity;
    ssize_t first_tompstone = -1;

    while (ht->hashes[i] != Z__HASH_TABLE_EMPTY) {

        if (first_tompstone == -1 && ht->hashes[i] == Z__HASH_TABLE_TOMBSTONE) {
            first_tompstone = (ssize_t)i;
        }

        if (ht->hashes[i] == hash && ht->equal(ht->keys[i], key)) {
            Z_Maybe_Pair old_pair = {
                .ok = true,
                .pair = z_make_pair(ht->keys[i], ht->values[i]),
            };

            ht->keys[i] = key;
            ht->values[i] = value;

            return old_pair;
        }

        i = (i + 1) % ht->capacity;
    }

    if (first_tompstone == -1) {
        ht->keys[i] = key;
        ht->values[i] = value;
        ht->hashes[i] = hash;
        ht->occupied++;
    } else {
        ht->keys[first_tompstone] = key;
        ht->values[first_tompstone] = value;
        ht->hashes[first_tompstone] = hash;
    }

    ht->size++;

    Z_Maybe_Pair pair = {
        .ok = true,
    };

    return pair;
}

void z__hash_table_resize(Z_Hash_Table *ht, size_t new_capacity)
{
    Z_Hash_Table new_ht = z_hash_table_new_with_capacity(ht->heap, ht->equal, ht->hash, new_capacity);

    for (size_t i = 0; i < ht->capacity; i++) {
        if (ht->hashes[i] >= 2) {
            z__hash_table_put_no_resize(&new_ht, ht->keys[i], ht->values[i], ht->hashes[i]);
        }
    }

    z__hash_table_free(ht);
    *ht = new_ht;
}

Z_Maybe_Pair z_hash_table_put(Z_Hash_Table *ht, void *key, void *value)
{
    if (z__hash_table_get_load_factor(ht) >= Z_HASH_TABLE_MAX_LOAD_FACTOR) {
        size_t new_capacity = Z_MAX((size_t)Z_HASH_TABLE_MIN_CAPACITY, ht->capacity * 2);
        z__hash_table_resize(ht, new_capacity);
    }

    size_t hash = z__hash_table_hash(ht, key);
    return z__hash_table_put_no_resize(ht, key, value, hash);
}

Z_Maybe_Pair z_hash_table_delete(Z_Hash_Table *ht, void *key)
{
    if (ht->size == 0) {
        Z_Maybe_Pair result = {
            .ok = false,
        };

        return result;
    }

    size_t hash = z__hash_table_hash(ht, key);
    size_t i = hash % ht->capacity;

    while (ht->hashes[i] != Z__HASH_TABLE_EMPTY && (ht->hashes[i] != hash || !ht->equal(key, ht->keys[i]))) {
        i = (i + 1) % ht->capacity;
    }

    if (ht->hashes[i] == Z__HASH_TABLE_EMPTY) {
        Z_Maybe_Pair result = {
            .ok = false,
        };

        return result;
    }

    ht->hashes[i] = Z__HASH_TABLE_TOMBSTONE;
    ht->size--;

    Z_Maybe_Pair old_pair = {
        .ok = true,
        .pair = z_make_pair(ht->keys[i], ht->values[i]),
    };

    return old_pair;
}

bool z_hash_table_contains(const Z_Hash_Table *ht, void *key)
{
    if (ht->capacity == 0) {
        return false;
    }

    size_t hash = z__hash_table_hash(ht, key);
    size_t i = hash % ht->capacity;

    while (ht->hashes[i] != Z__HASH_TABLE_EMPTY) {
        if (ht->hashes[i] == hash && ht->equal(key, ht->keys[i])) {
            return true;
        }

        i = (i + 1) % ht->capacity;
    }

    return false;
}

size_t z_hash_table_size(const Z_Hash_Table *ht)
{
    return ht->size;
}

Z_Pair_Array z_hash_table_to_array(Z_Heap *heap, const Z_Hash_Table *ht)
{
    Z_Pair_Array array = z_array_new(heap, Z_Pair_Array);

    for (size_t i = 0; i < ht->capacity; i++) {
        if (ht->hashes[i] >= 2) {
            z_array_push(&array, z_make_pair(ht->keys[i], ht->values[i]));
        }
    }

    return array;
}

Z_Hash_Table_Iter z_hash_table_iter(const Z_Hash_Table *ht)
{
    Z_Hash_Table_Iter iter = {
        .ht = ht,
        .i = 0,
    };

    return iter;
}

bool z_hash_table_iter_next(Z_Hash_Table_Iter *iter, Z_Pair *pair)
{
    const Z_Hash_Table *ht = iter->ht;
    size_t *i = &iter->i;

    while (*i < ht->capacity) {
        if (ht->hashes[*i] >= 2) {
            pair->key = ht->keys[*i];
            pair->value = ht->values[*i];
            (*i)++;
            return true;
        }

        (*i)++;
    }

    return false;
}

bool z_str_equal(const void *a, const void *b)
{
    return strcmp(a, b) == 0;
}

size_t z_str_hash(const void *s)
{
    size_t hash = 5381;

    for (const char *curr = s; *curr != '\0'; curr++) {
        hash = ((hash << 5) + hash) + (size_t)*curr;
    }

    return hash;
}
