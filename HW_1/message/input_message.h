//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_INPUT_MESSAGE_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_INPUT_MESSAGE_H

#include <stdint.h>
#include <stddef.h>

enum input_message_type
{
  INPUT_MESSAGE_TYPE_BACK,
  INPUT_MESSAGE_TYPE_PROG,
  INPUT_MESSAGE_TYPE_VOL_UP,
  INPUT_MESSAGE_TYPE_VOL_DOWN,

  INPUT_MESSAGE_TYPE_SWITCH
};

typedef size_t input_message_size_t;

struct input_message_header
{
  enum input_message_type type;
  input_message_size_t body_size;
};

#define SWITCH_BUTTON_NUM 9
union switch_data
{
  struct {
    uint16_t s1 : 1;
    uint16_t s2 : 1;
    uint16_t s3 : 1;
    uint16_t s4 : 1;
    uint16_t s5 : 1;
    uint16_t s6 : 1;
    uint16_t s7 : 1;
    uint16_t s8 : 1;
    uint16_t s9 : 1;
  } bit_fields;
  uint16_t val;
};

/* functions for sending input message */
int input_message_back_send (int fd);
int input_message_prog_send (int fd);
int input_message_vol_up_send (int fd);
int input_message_vol_down_send (int fd);
int input_message_switch_send (int fd, union switch_data data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_INPUT_MESSAGE_H
