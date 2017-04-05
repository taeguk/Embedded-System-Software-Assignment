//
// Created by taeguk on 2017-04-05.
//

#ifndef EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_OUTPUT_MESSAGE_H
#define EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_OUTPUT_MESSAGE_H

#include <stdint.h>
#include <stddef.h>

enum output_message_type
{
  OUTPUT_MESSAGE_TYPE_FND,
  OUTPUT_MESSAGE_TYPE_TEXT_LCD,
  OUTPUT_MESSAGE_TYPE_LED,
  OUTPUT_MESSAGE_TYPE_DOT_MATRIX
};

typedef size_t output_message_size_t;

struct output_message_header
{
  enum output_message_type type;
  output_message_size_t body_size;
};

typedef uint16_t fnd_data_t;

union led_data
{
  struct {
    uint8_t e1 : 1;
    uint8_t e2 : 1;
    uint8_t e3 : 1;
    uint8_t e4 : 1;
    uint8_t e5 : 1;
    uint8_t e6 : 1;
    uint8_t e7 : 1;
    uint8_t e8 : 1;
  } bit_fields;
  uint8_t val;
};

struct text_lcd_data
{
  size_t len;  // the length of string.
  char str[0];
};

#define DOT_MATRIX_WIDTH 11
#define DOT_MATRIX_HEIGHT 11

struct dot_matrix_data
{
  char data[DOT_MATRIX_HEIGHT][DOT_MATRIX_WIDTH];  // must be 0 or 1.
};

int output_message_fnd_send (int fd, fnd_data_t data);
int output_message_led_send (int fd, union led_data data);
int output_message_text_lcd_send (int fd, const struct text_lcd_data *data);
int output_message_dot_matrix_send (int fd, const struct dot_matrix_data *data);

#endif //EMBEDDED_SYSTEM_SOFTWARE_ASSIGNMENT_OUTPUT_MESSAGE_H
