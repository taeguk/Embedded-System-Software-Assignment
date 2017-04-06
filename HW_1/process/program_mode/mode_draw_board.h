//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_DRAW_BOARD_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_DRAW_BOARD_H

#include "../../message/input_message.h"

struct mode_draw_board_status;

struct mode_draw_board_status *mode_draw_board_construct (int output_pipe_fd);
void mode_draw_board_destroy (struct mode_draw_board_status *status);

int mode_draw_board_switch (struct mode_draw_board_status *status, union switch_data data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_MODE_DRAW_BOARD_H
