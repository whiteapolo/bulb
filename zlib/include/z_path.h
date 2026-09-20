#ifndef PATH_H
#define PATH_H

#include <z_string.h>
#include <z_heap.h>

bool z_expand_tilde(Z_String_View pathname, Z_String *out);
bool z_compress_tilde(Z_String_View pathname, Z_String *out);
bool z_is_directory(const char *pathname);
bool z_is_regular_file(const char *pathname);

#endif
