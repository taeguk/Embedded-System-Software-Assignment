//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include "output_message.h"

int output_message_fnd_send (int fd, fnd_data_t data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_FND;
  msg_header.body_size = sizeof (data);

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // Send body.
  write (fd, &data, msg_header.body_size);

  return 0;
}

int output_message_led_send (int fd, union led_data data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_LED;
  msg_header.body_size = sizeof (data);

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // Send body.
  write (fd, &data, msg_header.body_size);

  return 0;
}

int output_message_text_lcd_send (int fd, const struct text_lcd_data *data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_TEXT_LCD;
  msg_header.body_size = sizeof (*data) + data->len * sizeof (char);

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // Send body.
  write (fd, data, msg_header.body_size);

  return 0;
}

int output_message_dot_matrix_send (int fd, const struct dot_matrix_data *data)
{
  struct output_message_header msg_header;
  msg_header.type = OUTPUT_MESSAGE_TYPE_DOT_MATRIX;
  msg_header.body_size = sizeof (*data);

  // Send header.
  write (fd, &msg_header, sizeof (msg_header));
  // Send body.
  write (fd, data, msg_header.body_size);

  return 0;
}