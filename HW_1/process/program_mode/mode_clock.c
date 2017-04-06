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
#include "mode_clock.h"
#include "../../common/atomic.h"
#include "../../common/mytime.h"

#define BACKGROUND_WORKER_DELAY (10*1000)  // background worker delay (microseconds)
#define LED_BLINK_DELAY (1000*1000*1000)  // nanoseconds.

struct mode_clock_status
{
  int output_pipe_fd;
  int saved_hour;
  int saved_min;
  volatile int hour;
  volatile int min;

  volatile bool changing;
  union led_data normal_led_data;
  union led_data changing_led_data[2];

  pthread_t background_worker;
  volatile bool terminated;  // flag for terminating background worker.

  volatile bool invalidated;
};

static void *background_worker_main (void *arg);

struct mode_clock_status *mode_clock_construct (int output_pipe_fd)
{
  struct mode_clock_status *status;
  status = malloc (sizeof (*status));

  // Initializing
  {
    status->output_pipe_fd = output_pipe_fd;

    time_t cur_time;
    struct tm *time_info;
    time (&cur_time);
    time_info = localtime (&cur_time);
    status->saved_hour = status->hour = time_info->tm_hour;
    status->saved_min = status->min = time_info->tm_min;

    status->normal_led_data.val = 0;
    status->normal_led_data.bit_fields.e1 = 1;
    status->changing_led_data[0].val = 0;
    status->changing_led_data[0].bit_fields.e1 = status->changing_led_data[0].bit_fields.e3 = 1;
    status->changing_led_data[1].val = 0;
    status->changing_led_data[1].bit_fields.e1 = status->changing_led_data[1].bit_fields.e4 = 1;

    status->changing = false;
    status->terminated = false;
    status->invalidated = true;
    pthread_create (&status->background_worker, NULL, &background_worker_main, status);
  }

  return status;
}

void mode_clock_destroy (struct mode_clock_status *status)
{
  atomic_store_bool (&status->terminated, true);
  pthread_join (status->background_worker, NULL);
  free (status);
}

int mode_clock_switch (struct mode_clock_status *status, union switch_data data)
{
  if (data.bit_fields.s1)
    {
      if (status->changing)
        {
          status->saved_hour = status->hour;
          status->saved_min = status->min;
          atomic_store_bool (&status->changing, false);
          atomic_store_bool (&status->invalidated, true);
          // led 깜박의 종료.
        }
      else
        {
          atomic_store_bool (&status->changing, true);
          // led 깜박의 시작.
        }
    }

  if (data.bit_fields.s2)
    {
      status->hour = status->saved_hour;
      status->min = status->saved_min;
      atomic_store_bool (&status->invalidated, true);
    }

  if (data.bit_fields.s3)
    {
      if (status->changing)
        {
          status->hour = (status->hour + 1) % 24;
          atomic_store_bool (&status->invalidated, true);
        }
    }

  if (data.bit_fields.s4)
    {
      // 의문: hour를 1증가 안해도 되나?
      if (status->changing)
        {
          status->min = (status->min + 1) % 60;
          atomic_store_bool (&status->invalidated, true);
        }
    }

  return 0;
}

static void *background_worker_main (void *arg)
{
  struct mode_clock_status *status = arg;
  int led_idx = 0;
  long long prev_blink_time = get_nano_time () - 2 * LED_BLINK_DELAY;

  while (!atomic_load_bool (&status->terminated))
    {
      long long cur_time = get_nano_time ();

      if (cur_time >= prev_blink_time + LED_BLINK_DELAY && atomic_load_bool (&status->changing))
        {
          LOG (LOGGING_LEVEL_DEBUG, "[Main Process - Background Worker] blink!");
          output_message_led_send (status->output_pipe_fd, status->changing_led_data[led_idx]);
          led_idx = (led_idx + 1) % 2;
          prev_blink_time = cur_time;
        }

      if (atomic_exchange_bool (&status->invalidated, false))
        {
          if (!atomic_load_bool (&status->changing))
            output_message_led_send (status->output_pipe_fd, status->normal_led_data);
          output_message_fnd_send (status->output_pipe_fd, status->hour * 100 + status->min);
        }

      usleep (BACKGROUND_WORKER_DELAY);
    }

  return NULL;
}
