//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include <stdio.h>
#include "output_message.h"

// An Usage of it is not good. "return -1" is better software design.
// But, for now, program is simple, so it is okay.
#define WRITE_OR_DIE(...) \
  if (write (__VA_ARGS__) == -1) \
    perror ("[Main Process] Fail to write : ");

int output_message_fnd_send (int fd, fnd_data_t data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_FND;
  msg_header.body_size = sizeof (data);

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // Send body.
  WRITE_OR_DIE (fd, &data, msg_header.body_size);

  return 0;
}

int output_message_led_send (int fd, union led_data data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_LED;
  msg_header.body_size = sizeof (data);

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // Send body.
  WRITE_OR_DIE (fd, &data, msg_header.body_size);

  return 0;
}

int output_message_text_lcd_send (int fd, const struct text_lcd_data *data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_TEXT_LCD;
  msg_header.body_size = sizeof (*data) + data->len * sizeof (char);

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // Send body.
  WRITE_OR_DIE (fd, data, msg_header.body_size);

  return 0;
}

int output_message_dot_matrix_send (int fd, const struct dot_matrix_data *data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_DOT_MATRIX;
  msg_header.body_size = sizeof (*data);

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // Send body.
  WRITE_OR_DIE (fd, data, msg_header.body_size);

  return 0;
}

int output_message_terminate_send (int fd)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_TERMINATE;
  msg_header.body_size = 0;

  // Send header.
  WRITE_OR_DIE (fd, &msg_header, sizeof (msg_header));
  // No body.
  
  return 0;
}
