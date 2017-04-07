//
// Created by taeguk on 2017-04-05.
//

#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include "../../message/output_message.h"
#include "../../common/logging.h"
#include "../../common/atomic.h"
#include "../../common/mytime.h"
#include "mode_extra.h"

#define BACKGROUND_WORKER_DELAY (10*1000)  // background worker delay (microseconds)

struct mode_extra_status
{
  int output_pipe_fd;
  pthread_mutex_t mutex;
  union switch_data answer;
  long long rest_time[SWITCH_BUTTON_NUM + 1];
  int score; 
  pthread_t background_worker;
  bool terminated;  // flag for terminating background worker.
};

static void *background_worker_main (void *arg);

struct mode_extra_status *mode_extra_construct (int output_pipe_fd)
{
  struct mode_extra_status *status;
  status = malloc (sizeof (*status));

  status->output_pipe_fd = output_pipe_fd;
  pthread_mutex_init (&status->mutex, NULL);
  status->answer.val = 0;
  status->score = 0;
  status->terminated = false;

  pthread_create (&status->background_worker, NULL, &background_worker_main, status);
  
  return status;
}

void mode_extra_destroy (struct mode_extra_status *status)
{
  atomic_store_bool (&status->terminated, true);
  pthread_join (status->background_worker, NULL);
  pthread_mutex_destroy (&status->mutex);
  free (status);
}

int mode_extra_switch (struct mode_extra_status *status, union switch_data data)
{
  pthread_mutex_lock (&status->mutex);

  int16_t correct = status->answer.val & data.val;
  int16_t incorrect = (status->answer.val ^ data.val) & data.val;

  LOG (LOGGING_LEVEL_HIGH, "[Main Process] correct = %03X, incorrect = %03X.", correct, incorrect);

  status->answer.val ^= correct;

  for (int i = 0; i < SWITCH_BUTTON_NUM; ++i)
    {
      status->score += (correct & 1);
      status->score -= (incorrect & 1);
      correct >>= 1;
      incorrect >>= 1;
    }
  if (status->score < 0)
    status->score = 0;
  
  output_message_fnd_send (status->output_pipe_fd, status->score);

  pthread_mutex_unlock (&status->mutex);

  return 0;
}

static void *background_worker_main (void *arg)
{
  struct mode_extra_status *status = arg;
  
  long long prev_nano_time = get_nano_time ();
  long long prev_creation_time = 0;

  while (!atomic_load_bool (&status->terminated))
    {
      pthread_mutex_lock (&status->mutex);

      long long cur_nano_time = get_nano_time ();
      long long time_gap = cur_nano_time - prev_nano_time;
      prev_nano_time = cur_nano_time;

#define CHECK_EXPIRED(no) \
      if (status->answer.bit_fields.s##no) \
        { \
          status->rest_time[ no ] -= time_gap; \
          if (status->rest_time[ no ] < 0) \
            { \
              status->answer.bit_fields.s##no = 0; \
              if (status->score > 0) \
                --status->score; \
            } \
        }

      CHECK_EXPIRED (1);
      CHECK_EXPIRED (2);
      CHECK_EXPIRED (3);
      CHECK_EXPIRED (4);
      CHECK_EXPIRED (5);
      CHECK_EXPIRED (6);
      CHECK_EXPIRED (7);
      CHECK_EXPIRED (8);
      CHECK_EXPIRED (9);

#undef CHECK_EXPIRED
      
      long long delay_base = 300LL * 1000LL * 1000LL;
      long long micro_delay = (rand () % 10 + 1) * delay_base;
      double delay_coef = 5 / ((status->score + 10) / 20.0);

      if (cur_nano_time - prev_creation_time > delay_coef * delay_base)
        {
          int no = 1 + rand () % SWITCH_BUTTON_NUM;
          status->rest_time[no] = micro_delay;
          status->answer.val |= (1 << (no-1));
          prev_creation_time = cur_nano_time;
        }

      output_message_fnd_send (status->output_pipe_fd, status->score);

      struct dot_matrix_data dot_data = { { { 0, }, } };
      int16_t bit_mask = 1;
      const int x_jump = 2, x_size = 3;
      const int y_jump = 3, y_size = 4;
      for (int i = 0; i < 3; ++i)
        {
          for (int j = 0; j < 3; ++j)
            {
              if (status->answer.val & bit_mask)
                {
                  int base_y = i * y_jump;
                  int base_x = j * x_jump;
                  for (int y = base_y; y < base_y + y_size; ++y)
                    for (int x = base_x; x < base_x + x_size; ++x)
                      dot_data.data[y][x] = 1;
                }
              bit_mask <<= 1;
            }
        }
      output_message_dot_matrix_send (status->output_pipe_fd, &dot_data);

      pthread_mutex_unlock (&status->mutex);

      usleep (BACKGROUND_WORKER_DELAY);
    }

  return NULL;
}
