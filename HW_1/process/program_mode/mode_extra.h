//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_EXTRA_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_EXTRA_H

#include "../../message/input_message.h"

struct mode_extra_status;

struct mode_extra_status *mode_extra_construct (int output_pipe_fd);
void mode_extra_destroy (struct mode_extra_status *status);

int mode_extra_switch (struct mode_extra_status *status, union switch_data data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_EXTRA_H
