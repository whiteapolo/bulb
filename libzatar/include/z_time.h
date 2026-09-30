#ifndef TIME_H
#define TIME_H

#include <time.h>

typedef clock_t Z_Clock;
typedef clock_t Z_Time;

Z_Time z_time(void);

double z_time_elapsed_seconds(Z_Time time);
double z_time_elapsed_mseconds(Z_Time time);

void z_time_print_elapsed_seconds(Z_Time time);
void z_time_print_elapsed_mseconds(Z_Time time);

#endif
