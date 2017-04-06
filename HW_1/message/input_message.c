//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include "input_message.h"

int input_message_back_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_BACK;
  msg_header.body_size = 0;

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // No body.

  return 0;
}

int input_message_prog_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_PROG;
  msg_header.body_size = 0;

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // No body.

  return 0;
}

int input_message_vol_up_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_VOL_UP;
  msg_header.body_size = 0;

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // No body.
  
  return 0;
}

int input_message_vol_down_send (int fd)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_VOL_DOWN;
  msg_header.body_size = 0;

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // No body.

  return 0;
}

int input_message_switch_send (int fd, union switch_data data)
{
  struct input_message_header msg_header;
  msg_header.type = INPUT_MESSAGE_TYPE_SWITCH;
  msg_header.body_size = sizeof (data);

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // Send body.
  write (fd, &data, msg_header.body_size);

  return 0;
}
