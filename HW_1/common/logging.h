//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_LOGGING_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_LOGGING_H

#include <stdio.h>

#define _LOGGING_LEVEL 4

#define LOGGING_LEVEL_DEBUG 1
#define LOGGING_LEVEL_LOW 3
#define LOGGING_LEVEL_NORMAL 5
#define LOGGING_LEVEL_HIGH 7

#define LOG_STDOUT(level_, ...) if ((level_) >= _LOGGING_LEVEL) { fprintf (stdout, __VA_ARGS__); fprintf (stdout, "\n"); fflush (stdout); }
#define LOG_STDERR(level_, ...) if ((level_) >= _LOGGING_LEVEL) { fprintf (stderr, __VA_ARGS__); fprintf (stderr, "\n"); fflush (stderr); }

#define LOG(...) LOG_STDERR(__VA_ARGS__)

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_LOGGING_H
