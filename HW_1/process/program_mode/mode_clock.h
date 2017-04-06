//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_CLOCK_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_CLOCK_H

#include "../../message/input_message.h"

struct mode_clock_status;

struct mode_clock_status *mode_clock_construct (int output_pipe_fd);
void mode_clock_destroy (struct mode_clock_status *status);

int mode_clock_switch (struct mode_clock_status *status, union switch_data data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_CLOCK_H
