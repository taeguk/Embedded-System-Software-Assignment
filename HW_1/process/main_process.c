//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <error.h>
#include <errno.h>
#include "main_process.h"
#include "../message/input_message.h"
#include "../message/output_message.h"
#include "../common/logging.h"
#include "program_mode/mode_clock.h"
#include "program_mode/mode_counter.h"
#include "program_mode/mode_text_editor.h"
#include "program_mode/mode_draw_board.h"
#include "program_mode/mode_extra.h"

enum program_mode
{
  PROGRAM_MODE_CLOCK = 0, /* must be started with 0 */
  PROGRAM_MODE_COUNTER,
  PROGRAM_MODE_TEXT_EDITOR,
  PROGRAM_MODE_DRAW_BOARD,
  PROGRAM_MODE_EXTRA,

  _PROGRAM_MODE_COUNT  /* The number of program modes */
};

static bool terminated = false;

/* program mode and status variables for each modes */
enum program_mode program_mode = PROGRAM_MODE_CLOCK;
struct mode_clock_status *mode_clock_status = NULL;
struct mode_counter_status *mode_counter_status = NULL;
struct mode_text_editor_status *mode_text_editor_status = NULL;
struct mode_draw_board_status *mode_draw_board_status = NULL;
struct mode_extra_status *mode_extra_status = NULL;

static enum program_mode change_mode (enum program_mode new_mode, int output_pipe_fd);
static int process_input_message (const struct input_message_header *msg_header, void *msg_body, int output_pipe_fd);
static int input_message_h_back ();
static int input_message_h_prog ();
static int input_message_h_vol_up (int output_pipe_fd);
static int input_message_h_vol_down (int output_pipe_fd);
static int input_message_h_switch (union switch_data data);

int main_process_main (int input_pipe_fd, int output_pipe_fd)
{
  change_mode (PROGRAM_MODE_CLOCK, output_pipe_fd);

  while (!terminated)
    {
      int read_sz;
      struct input_message_header msg_header;
      
      /* Read message header */
      read_sz = read (input_pipe_fd, &msg_header, sizeof (msg_header));
      if (read_sz == -1)
        {
          // ERROR.
          perror ("[Main Process] input pipe read error : ");
        }
      else if (read_sz != sizeof (msg_header))
        {
          // ERROR.
          LOG (LOGGING_LEVEL_HIGH,
               "[Main Process] input pipe read_sz = %d (expected %u).",
               read_sz, sizeof (msg_header));
          return -1;
        }

      /* Read message body */
      void *msg_body= malloc (msg_header.body_size);
      read_sz = 0;
      while (read_sz < msg_header.body_size)
        {
          int tmp_read_sz;
          tmp_read_sz = read (input_pipe_fd, msg_body + read_sz, msg_header.body_size - read_sz);
          if (tmp_read_sz == -1)
            {
              if (errno == EAGAIN)
                continue;
              else
                perror ("[Main Process] input pipe read error : ");
            }
          read_sz += tmp_read_sz;
        }
      
      /* Process message */
      if (process_input_message (&msg_header, msg_body, output_pipe_fd) == -1)
        {
          LOG (LOGGING_LEVEL_HIGH, 
               "[Main Process] Process message error (msg type = %d).", msg_header.type);
        }
      
      free (msg_body);
    }

  close (input_pipe_fd);
  close (output_pipe_fd);
  
  LOG (LOGGING_LEVEL_NORMAL, "[Main Process] I'm gracefully dead.");

  return 0;
}


static enum program_mode change_mode (enum program_mode new_mode, int output_pipe_fd)
{
  LOG (LOGGING_LEVEL_NORMAL, "[Main Process] Change mode (%d -> %d).", program_mode, new_mode);
  switch (program_mode)
    {
      case PROGRAM_MODE_CLOCK:
        if (mode_clock_status)
          {
            mode_clock_destroy (mode_clock_status);
            mode_clock_status = NULL;
          }
        break;
      case PROGRAM_MODE_COUNTER:
        if (mode_counter_status)
          {
            mode_counter_destroy (mode_counter_status);
            mode_counter_status = NULL;
          }
        break;
      case PROGRAM_MODE_TEXT_EDITOR:
        if (mode_text_editor_status)
          {
            mode_text_editor_destroy (mode_text_editor_status);
            mode_text_editor_status = NULL;
          }
        break;
      case PROGRAM_MODE_DRAW_BOARD:
        if (mode_draw_board_status)
          {
            mode_draw_board_destroy (mode_draw_board_status);
            mode_draw_board_status = NULL;
          }
        break;
      case PROGRAM_MODE_EXTRA:
        if (mode_extra_status)
          {
            mode_extra_destroy (mode_extra_status);
            mode_extra_status = NULL;
          }
        break;
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Main Process] strange program mode : %d.", program_mode);
    }

  switch (new_mode)
    {
      case PROGRAM_MODE_CLOCK:
        mode_clock_status = mode_clock_construct (output_pipe_fd);
        break;
      case PROGRAM_MODE_COUNTER:
        mode_counter_status = mode_counter_construct (output_pipe_fd);
        break;
      case PROGRAM_MODE_TEXT_EDITOR:
        mode_text_editor_status = mode_text_editor_construct (output_pipe_fd);
        break;
      case PROGRAM_MODE_DRAW_BOARD:
        mode_draw_board_status = mode_draw_board_construct (output_pipe_fd);
        break;
      case PROGRAM_MODE_EXTRA:
        mode_extra_status = mode_extra_construct (output_pipe_fd);
        break;
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Main Process] strange program mode : %d.", program_mode);
      exit (-1);
    }

  enum program_mode prev_mode = program_mode;
  program_mode = new_mode;

  return prev_mode;
}

static int process_input_message (const struct input_message_header *msg_header, void *msg_body, int output_pipe_fd)
{
  switch (msg_header->type)
    {
      case INPUT_MESSAGE_TYPE_BACK:
        LOG (LOGGING_LEVEL_NORMAL, "[Main Process] Recv input_message (Read Key - Back).");
        return input_message_h_back ();
      case INPUT_MESSAGE_TYPE_PROG:
        LOG (LOGGING_LEVEL_NORMAL, "[Main Process] Recv input_message (Read Key - PROG).");
        return input_message_h_prog ();
      case INPUT_MESSAGE_TYPE_VOL_UP:
        LOG (LOGGING_LEVEL_NORMAL, "[Main Process] Recv input_message (Read Key - VOL_UP).");
        return input_message_h_vol_up (output_pipe_fd);
      case INPUT_MESSAGE_TYPE_VOL_DOWN:
        LOG (LOGGING_LEVEL_NORMAL, "[Main Process] Recv input_message (Read Key - VOL_DOWN).");
        return input_message_h_vol_down (output_pipe_fd);
      case INPUT_MESSAGE_TYPE_SWITCH:
        LOG (LOGGING_LEVEL_NORMAL, "[Main Process] Recv input_message (Switch - val : %03X).", ((union switch_data *) msg_body)->val);
        return input_message_h_switch (*((union switch_data *) msg_body));
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Main Process] strange input message type : %d.", msg_header->type);
        return -1;
    }
}

static int input_message_h_back ()
{
  LOG (LOGGING_LEVEL_HIGH, "[Main Process] Program Terminated.");
  terminated = true;
  return 0;
}

static int input_message_h_prog ()
{
  // nothing to do.
  return 0;
}

static int input_message_h_vol_up (int output_pipe_fd)
{
  change_mode ((program_mode + 1) % _PROGRAM_MODE_COUNT, output_pipe_fd);
  return 0;
}

static int input_message_h_vol_down (int output_pipe_fd)
{
  change_mode ((program_mode + _PROGRAM_MODE_COUNT - 1) % _PROGRAM_MODE_COUNT, output_pipe_fd);
  return 0;
}

static int input_message_h_switch (union switch_data data)
{
  switch (program_mode)
    {
      case PROGRAM_MODE_CLOCK:
        return mode_clock_switch (mode_clock_status, data);
      case PROGRAM_MODE_COUNTER:
        return mode_counter_switch (mode_counter_status, data);
      case PROGRAM_MODE_TEXT_EDITOR:
        return mode_text_editor_switch (mode_text_editor_status, data);
      case PROGRAM_MODE_DRAW_BOARD:
        return mode_draw_board_switch (mode_draw_board_status, data);
      case PROGRAM_MODE_EXTRA:
        return mode_extra_switch (mode_extra_status, data);
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Main Process] strange program mode : %d.", program_mode);
        return -1;
    }
}
