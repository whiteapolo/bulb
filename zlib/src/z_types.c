#include <z_types.h>

Z_Pair z_make_pair(void *key, void *value)
{
    Z_Pair pair = {
        .key = key,
        .value = value,
    };

    return pair;
}