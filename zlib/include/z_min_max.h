#ifndef Z_MIN_MAX_H
#define Z_MIN_MAX_H

#define Z_MAX(a, b)     \
({                      \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a > _b ? _a : _b;  \
})

#define Z_MIN(a, b)     \
({                      \
    typeof(a) _a = (a); \
    typeof(b) _b = (b); \
    _a < _b ? _a : _b;  \
})

#endif
