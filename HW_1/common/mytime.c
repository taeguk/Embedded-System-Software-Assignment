//
// Created by taeguk on 2017-04-06.
//

#include <time.h>
#include "mytime.h"

long long get_nano_time ()
{
  struct timespec tspec;
  clock_gettime (CLOCK_MONOTONIC, &tspec);
  return tspec.tv_sec * 1000LL * 1000LL * 1000LL + tspec.tv_nsec;
}
