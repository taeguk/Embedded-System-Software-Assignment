//
// Created by taeguk on 2017-04-06.
//

#include <time.h>
#include "mytime.h"

long get_nano_realtime ()
{
  struct timespec tspec;
  clock_gettime (CLOCK_REALTIME, &tspec);
  return tspec.tv_nsec;
}