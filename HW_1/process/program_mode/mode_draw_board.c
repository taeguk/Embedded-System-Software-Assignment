//
// Created by taeguk on 2017-04-05.
//
//
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "../../message/output_message.h"
#include "../../common/logging.h"
#include "../../common/atomic.h"
#include "../../common/mytime.h"
#include "mode_draw_board.h"

#define BACKGROUND_WORKER_DELAY (10*1000)  // background worker delay (microseconds)
#define CURSOR_BLINK_DELAY (1000*1000*1000)  // nanoseconds.

static struct dot_matrix_data empty_dot_data;

/*
 * TODO: Thread-Safe.
 */

struct mode_draw_board_status
{
  int output_pipe_fd;

  volatile bool cur_show;
  int cur_pos_x, cur_pos_y;
  char cur_val;
  struct dot_matrix_data dot_data;  // Thread-Unsafe. TODO: It must be syncronized.
  
  pthread_t background_worker;
  volatile bool terminated;  // flag for terminating background worker.

  volatile bool invalidated;
};

static void *background_worker_main (void *arg);

struct mode_draw_board_status *mode_draw_board_construct (int output_pipe_fd)
{
  struct mode_draw_board_status *status;
  status = malloc (sizeof (*status));

  status->output_pipe_fd = output_pipe_fd;
  status->cur_show = true;
  status->cur_pos_x = status->cur_pos_y = 0;
  status->cur_val = 0;
  status->dot_data = empty_dot_data;
  status->terminated = false;
  
  status->invalidated = true;
  pthread_create (&status->background_worker, NULL, &background_worker_main, status);

  return status;
}

void mode_draw_board_destroy (struct mode_draw_board_status *status)
{
  atomic_store_bool (&status->terminated, true);
  pthread_join (status->background_worker, NULL);
  free (status);
}

int mode_draw_board_switch (struct mode_draw_board_status *status, union switch_data data)
{
  int next_x = status->cur_pos_x;
  int next_y = status->cur_pos_y;

  if (data.bit_fields.s1)
    {
      atomic_store_bool (&status->cur_show, true);
      next_x = next_y = 0;
      status->dot_data = empty_dot_data;
    }

  if (data.bit_fields.s2)
    {
      next_y = status->cur_pos_y - 1;
    }

  if (data.bit_fields.s3)
    {
      // TODO: problem. Need "atomic not".
      atomic_store_bool (&status->cur_show, atomic_load_bool (&status->cur_show));
    }

  if (data.bit_fields.s4)
    {
      next_x = status->cur_pos_x - 1;
    }

  if (data.bit_fields.s5)
    {
      status->cur_val = (status->cur_val + 1) % 2;
    }

  if (data.bit_fields.s6)
    {
      next_x = status->cur_pos_x + 1;
    }

  if (data.bit_fields.s7)
    {
      status->dot_data = empty_dot_data;
    }

  if (data.bit_fields.s8)
    {
      next_y = status->cur_pos_y + 1;
    }

  if (data.bit_fields.s9)
    {
      for (int i = 0; i < DOT_MATRIX_HEIGHT; ++i)
        {
          for (int j = 0; j < DOT_MATRIX_WIDTH; ++j)
            {
              if (i == status->cur_pos_y && j == status->cur_pos_x)
                continue;
              status->dot_data.data[i][j] = (status->dot_data.data[i][j] + 1) % 2;
            }
        }
    }

  /* When cursor position is changed. */
  if (0 <= next_x && next_x < DOT_MATRIX_WIDTH &&
      0 <= next_y && next_y < DOT_MATRIX_HEIGHT &&
      (next_x != status->cur_pos_x || next_y != status->cur_pos_y))
    {
      status->dot_data.data[status->cur_pos_y][status->cur_pos_x] = status->cur_val;
      status->cur_pos_x = next_x;
      status->cur_pos_y = next_y;
      status->cur_val = status->dot_data.data[status->cur_pos_y][status->cur_pos_x];
    }

  status->invalidated = true;

  return 0;
}

// TODO: need to be refactored.
static void *background_worker_main (void *arg)
{
  struct mode_draw_board_status *status = arg;
  long long prev_blink_time = get_nano_time () - 2 * CURSOR_BLINK_DELAY;

  while (!atomic_load_bool (&status->terminated))
    {
      long long cur_time = get_nano_time ();

      if (cur_time >= prev_blink_time + CURSOR_BLINK_DELAY && atomic_load_bool (&status->cur_show))
        {
          LOG (LOGGING_LEVEL_DEBUG, "[Main Process - Background Worker] blink!");

          status->dot_data.data[status->cur_pos_y][status->cur_pos_x] = 
            (status->dot_data.data[status->cur_pos_y][status->cur_pos_x] + 1) % 2;
          
          output_message_dot_matrix_send (status->output_pipe_fd, &status->dot_data);
          
          prev_blink_time = cur_time;
        }

      if (atomic_exchange_bool (&status->invalidated, false))
        {
          if (!atomic_load_bool (&status->cur_show))
            {
              status->dot_data.data[status->cur_pos_y][status->cur_pos_x] = status->cur_val;
              output_message_dot_matrix_send (status->output_pipe_fd, &status->dot_data);
            }
        }

      usleep (BACKGROUND_WORKER_DELAY);
    }

  return NULL;
}
