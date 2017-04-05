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
  /*
  PROGRAM_MODE_COUNTER,
  PROGRAM_MODE_TEXT_EDITOR,
  PROGRAM_MODE_DRAW_BOARD,
  PROGRAM_MODE_EXTRA,
  */

  _PROGRAM_MODE_COUNT  /* The number of program modes */
};

static bool terminated = false;

enum program_mode program_mode = PROGRAM_MODE_CLOCK;
struct mode_clock_status *mode_clock_status = NULL;

static enum program_mode change_mode (enum program_mode new_mode, int output_pipe_fd);
static int process_input_message (const struct input_message_header *msg_header, void *msg_body, int output_pipe_fd);
static int input_message_h_back ();
static int input_message_h_prog ();
static int input_message_h_vol_up (int output_pipe_fd);
static int input_message_h_vol_down (int output_pipe_fd);
static int input_message_h_switch (switch_data_t data);

int main_process_main (int input_pipe_fd, int output_pipe_fd)
{
  if (fcntl (input_pipe_fd, F_SETFL, fcntl(input_pipe_fd, F_GETFL) | O_NONBLOCK) == -1)
    perror ("[Main Process] fcntl error : ");

  change_mode (PROGRAM_MODE_CLOCK, output_pipe_fd);

  while (!terminated)
    {
      int read_sz;
      struct input_message_header msg_header;

      read_sz = read (input_pipe_fd, &msg_header, sizeof (msg_header));
      if (read_sz == -1)
        {
          if (errno == EAGAIN)
            {
              // There is no message in input pipe.
              continue;
            }
          else
            {
              // ERROR.
              perror ("[Main Process] input pipe read error : ");
            }
        }
      else if (read_sz != sizeof (msg_header))
        {
          // ERROR.
          LOG (LOGGING_LEVEL_HIGH,
               "[Main Process] input pipe read_sz = %d (expected %u).",
               read_sz, sizeof (msg_header));
          return -1;
        }

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

      process_input_message (&msg_header, msg_body, output_pipe_fd);

      free (msg_body);
    }

  return 0;
}


static enum program_mode change_mode (enum program_mode new_mode, int output_pipe_fd)
{
  switch (program_mode)
    {
      case PROGRAM_MODE_CLOCK:
        if (mode_clock_status)
          {
            mode_clock_destroy (mode_clock_status);
            mode_clock_status = NULL;
          }
      break;
      /*
      case PROGRAM_MODE_COUNTER:
        break;
      case PROGRAM_MODE_TEXT_EDITOR:
        break;
      case PROGRAM_MODE_DRAW_BOARD:
        break;
      case PROGRAM_MODE_EXTRA:
        break;
      */
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Main Process] strange program mode : %d.", program_mode);
    }

  switch (new_mode)
    {
      case PROGRAM_MODE_CLOCK:
        mode_clock_status = mode_clock_construct (output_pipe_fd);
      break;
      /*
      case PROGRAM_MODE_COUNTER:
        break;
      case PROGRAM_MODE_TEXT_EDITOR:
        break;
      case PROGRAM_MODE_DRAW_BOARD:
        break;
      case PROGRAM_MODE_EXTRA:
        break;
      */
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
        return input_message_h_back ();
      case INPUT_MESSAGE_TYPE_PROG:
        return input_message_h_prog ();
      case INPUT_MESSAGE_TYPE_VOL_UP:
        return input_message_h_vol_up (output_pipe_fd);
      case INPUT_MESSAGE_TYPE_VOL_DOWN:
        return input_message_h_vol_down (output_pipe_fd);
      case INPUT_MESSAGE_TYPE_SWITCH:
        return input_message_h_switch (*((switch_data_t*) msg_body));
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

static int input_message_h_switch (switch_data_t data)
{
  switch (program_mode)
    {
      case PROGRAM_MODE_CLOCK:
        return mode_clock_switch (mode_clock_status, data);
      /*
      case PROGRAM_MODE_COUNTER:
        break;
      case PROGRAM_MODE_TEXT_EDITOR:
        break;
      case PROGRAM_MODE_DRAW_BOARD:
        break;
      case PROGRAM_MODE_EXTRA:
        break;
      */
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Main Process] strange program mode : %d.", program_mode);
        return -1;
    }
}