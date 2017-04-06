//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_TEXT_EDITOR_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_TEXT_EDITOR_H

#include "../../message/input_message.h"

struct mode_text_editor_status;

struct mode_text_editor_status *mode_text_editor_construct (int output_pipe_fd);
void mode_text_editor_destroy (struct mode_text_editor_status *status);

int mode_text_editor_switch (struct mode_text_editor_status *status, union switch_data data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_TEXT_EDITOR_H
