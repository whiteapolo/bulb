#ifndef Z_TYPES_H
#define Z_TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <z_array.h>

typedef void (*Z_Print_Fn)(const void *);
typedef int (*Z_Compare_Fn)(const void *, const void *);
typedef size_t (*Z_Hash_Fn)(const void *);
typedef bool (*Z_Equal_Fn)(const void *, const void *);

typedef struct {
    void *key;
    void *value;
} Z_Pair;

typedef struct {
    bool ok;
    Z_Pair pair;
} Z_Maybe_Pair;

Z_DEFINE_ARRAY(Z_Pair_Array, Z_Pair);

Z_Pair z_make_pair(void *key, void *value);

#endif