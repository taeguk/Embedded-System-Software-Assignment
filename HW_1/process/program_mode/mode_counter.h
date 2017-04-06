//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_COUNTER_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_COUNTER_H

#include "../../message/input_message.h"

struct mode_counter_status;

struct mode_counter_status *mode_counter_construct (int output_pipe_fd);
void mode_counter_destroy (struct mode_counter_status *status);

int mode_counter_switch (struct mode_counter_status *status, union switch_data data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_COUNTER_H
