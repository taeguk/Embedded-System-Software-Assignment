//
// Created by taeguk on 2017-04-05.
//

#include <stdlib.h>
#include "mode_text_editor.h"
#include "../../message/output_message.h"
#include "../../common/logging.h"

enum typing_mode
{
  TYPING_MODE_ALPHABET = 0, /* must be started from 0. */
  TYPING_MODE_NUMBER,

  _TYPING_MODE_COUNT  /* The number of modes */
};

struct mode_text_editor_status
{
  int output_pipe_fd;
  enum typing_mode typing_mode;
  struct text_lcd_data *lcd_data;
  int prev_switch_no;
  int alphabet_idx;
};

static struct dot_matrix_data dot_matrix_data[_TYPING_MODE_COUNT] =
  {
    {
      {
        { 0, 0, 1, 1, 1, 0, 0 },
        { 0, 1, 1, 0, 1, 1, 0 },
        { 1, 1, 0, 0, 0, 1, 1 },
        { 1, 1, 0, 0, 0, 1, 1 },
        { 1, 1, 0, 0, 0, 1, 1 },

        { 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 1, 1, 1, 1, 1 },
        { 1, 1, 0, 0, 0, 1, 1 },
        { 1, 1, 0, 0, 0, 1, 1 },
        { 1, 1, 0, 0, 0, 1, 1 }
      }
    },
    {
      {
        { 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 1, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 1, 0, 0 },

        { 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 1, 0, 0 },
        { 0, 0, 0, 1, 1, 0, 0 },
        { 0, 1, 1, 1, 1, 1, 1 },
        { 0, 1, 1, 1, 1, 1, 1 }
      }
    }
  };

#define ALPHABETS_PER_SWITCH 3

static const char switch_to_alphabet [SWITCH_BUTTON_NUM + 1][ALPHABETS_PER_SWITCH] =
  {
    [1] = { '.', 'Q', 'Z' },
    [2] = { 'A', 'B', 'C' },
    [3] = { 'D', 'E', 'F' },
    [4] = { 'G', 'H', 'I' },
    [5] = { 'J', 'K', 'L' },
    [6] = { 'M', 'N', 'O' },
    [7] = { 'P', 'R', 'S' },
    [8] = { 'T', 'U', 'V' },
    [9] = { 'W', 'X', 'Y' }
  };

inline static void update_text_lcd (const struct mode_text_editor_status *status)
{
  if (status->lcd_data->len <= 0)
    {
      status->lcd_data->len = MAX_LCD_STRING_LEN;
      for (int i = 0; i < MAX_LCD_STRING_LEN; ++i)
        status->lcd_data->str[i] = ' ';
      status->lcd_data->str[MAX_LCD_STRING_LEN] = '\0';
      
      output_message_text_lcd_send (status->output_pipe_fd, status->lcd_data);

      status->lcd_data->len = 0;
      status->lcd_data->str[0] = '\0';
    }
  else
    {
      LOG (LOGGING_LEVEL_NORMAL, "[Output Process] update text lcd : %s\n", status->lcd_data->str);
      output_message_text_lcd_send (status->output_pipe_fd, status->lcd_data);
    }
}

inline static void update_dot_matrix (const struct mode_text_editor_status *status)
{
  output_message_dot_matrix_send (status->output_pipe_fd, &dot_matrix_data[status->typing_mode]);
}

struct mode_text_editor_status *mode_text_editor_construct (int output_pipe_fd)
{
  struct mode_text_editor_status *status;
  status = malloc (sizeof (*status));

  status->output_pipe_fd = output_pipe_fd;
  status->prev_switch_no = -1;
  status->lcd_data = malloc (sizeof (*status->lcd_data) + (MAX_LCD_STRING_LEN + 1) * sizeof (char));
  status->lcd_data->len = 0;
  status->lcd_data->str[0] = '\0';
  status->typing_mode = TYPING_MODE_ALPHABET;

  update_text_lcd (status);
  update_dot_matrix (status);  

  return status;
}

void mode_text_editor_destroy (struct mode_text_editor_status *status)
{
  free (status->lcd_data);
  free (status);
}

static void add_character (struct mode_text_editor_status *status, char ch)
{
  if (status->lcd_data->len >= MAX_LCD_STRING_LEN)
    {
      for (int i = 0; i < MAX_LCD_STRING_LEN - 1; ++i)
        status->lcd_data->str[i] = status->lcd_data->str[i+1];
      --status->lcd_data->len;
    }
  status->lcd_data->str[status->lcd_data->len++] = ch;
  status->lcd_data->str[status->lcd_data->len] = '\0';
}

inline static void replace_character (struct mode_text_editor_status *status, char rep_ch)
{
  if (status->lcd_data->len <= 0)
    add_character (status, rep_ch);
  else
    status->lcd_data->str[status->lcd_data->len - 1] = rep_ch;
}

static void process_key_combination (struct mode_text_editor_status *status, union switch_data *data)
{
  if (data->bit_fields.s2 && data->bit_fields.s3)
    {
      status->lcd_data->len = 0;
      status->lcd_data->str[0] = '\0';
      data->bit_fields.s2 = data->bit_fields.s3 = 0;
    }

  if (data->bit_fields.s5 && data->bit_fields.s6)
    {
      status->typing_mode = (status->typing_mode + 1) % _TYPING_MODE_COUNT;
      data->bit_fields.s5 = data->bit_fields.s6 = 0;
    }

  if (data->bit_fields.s8 && data->bit_fields.s9)
    {
      add_character (status, ' ');
      data->bit_fields.s8 = data->bit_fields.s9 = 0;
    }
}

typedef void (*handler_character) (struct mode_text_editor_status *status, int switch_no);

static void handle_alphabet (struct mode_text_editor_status *status, int switch_no)
{
  if (status->prev_switch_no == switch_no)
    {
      status->alphabet_idx = (status->alphabet_idx + 1) % ALPHABETS_PER_SWITCH;
      replace_character (status, switch_to_alphabet[switch_no][status->alphabet_idx]);
    }
  else
    {
      status->prev_switch_no = switch_no;
      status->alphabet_idx = 0;
      add_character (status, switch_to_alphabet[switch_no][status->alphabet_idx]);
    }
  update_dot_matrix (status);
}

inline static void handle_number (struct mode_text_editor_status *status, int switch_no)
{
  add_character (status, switch_no + '0');
}

static void process_character_input (struct mode_text_editor_status *status, union switch_data data)
{
  handler_character handler;
  if (status->typing_mode == TYPING_MODE_ALPHABET)
    handler = &handle_alphabet;
  else if (status->typing_mode == TYPING_MODE_NUMBER)
    handler = &handle_number;
  else
    LOG (LOGGING_LEVEL_HIGH, "[Main Process] invalid typing mode.");

#define HANDLE_SWITCH_DATA(no) if (data.bit_fields.s##no) handler (status, no);

  HANDLE_SWITCH_DATA (1);
  HANDLE_SWITCH_DATA (2);
  HANDLE_SWITCH_DATA (3);
  HANDLE_SWITCH_DATA (4);
  HANDLE_SWITCH_DATA (5);
  HANDLE_SWITCH_DATA (6);
  HANDLE_SWITCH_DATA (7);
  HANDLE_SWITCH_DATA (8);
  HANDLE_SWITCH_DATA (9);

#undef HANDLE_SWITCH_DATA
}

int mode_text_editor_switch (struct mode_text_editor_status *status, union switch_data data)
{
  process_key_combination (status, &data);
  process_character_input (status, data);

  update_text_lcd (status);
  update_dot_matrix (status);

  return 0;
}
