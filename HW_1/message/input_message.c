//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include <stdio.h>
#include "input_message.h"

// An Usage of it is not good. "return -1" is better software design.
// But, for now, program is simple and restricted, so it is okay.
#define WRITE_OR_DIE(...) \
  if (write (__VA_ARGS__) == -1) \
    perror ("[Input Process] Fail to write : ");

int input_message_back_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_BACK;
  msg_header.body_size = 0;

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // No body.

  return 0;
}

int input_message_prog_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_PROG;
  msg_header.body_size = 0;

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // No body.

  return 0;
}

int input_message_vol_up_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_VOL_UP;
  msg_header.body_size = 0;

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // No body.
  
  return 0;
}

int input_message_vol_down_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_VOL_DOWN;
  msg_header.body_size = 0;

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // No body.

  return 0;
}

int input_message_switch_send (int fd, union switch_data data)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_SWITCH;
  msg_header.body_size = sizeof (data);

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // Send body.
  WRITE_OR_DIE (fd, &data, msg_header.body_size);

  return 0;
}
