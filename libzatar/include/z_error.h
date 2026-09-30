#ifndef Z_ERROR_H
#define Z_ERROR_H

#include <stdarg.h>
#include <stdbool.h>

void z_die(const char *format, ...);
void z_die_va(const char *format, va_list args);
void z_perror(const char *format, ...);
void z_perror_va(const char *format, va_list args);
void z_enforce(bool predicate, const char *format, ...);

#endif
