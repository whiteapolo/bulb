#ifndef Z_CLAMP_H
#define Z_CLAMP_H

#define Z_CLAMP(min, value, max)  \
({                                \
    typeof(min) _min = min;       \
    typeof(max) _max = max;       \
    typeof(value) _value = value; \
    _value < _min                 \
        ? _min                    \
        : _value > _max           \
            ? _max                \
            : _value;             \
})

#endif
