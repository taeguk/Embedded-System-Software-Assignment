//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include "input_process.h"
#include "../common/logging.h"
#include "../message/input_message.h"

#define DEVICE_READKEY  "/def/input/event0"
#define DEVICE_SWITCH "/dev/fpga_push_switch"
#define DELAY (10*1000)  // background worker delay (microseconds)
#define BUTTON_NUM 9

#define KEY_PRESS   1
#define KEY_RELEASE 0

#define KEY_CODE_BACK       158
#define KEY_CODE_VOL_UP     115
#define KEY_CODE_VOL_DOWN   114

bool terminated = false;

static int process_readkey (int readkey_fd, int pipe_fd)
{
  int read_sz;
  struct input_event ev;

  read_sz = read (readkey_fd, &ev, sizeof (ev));
  if (read_sz < sizeof (ev))
    {
      if (read_sz == -1 && errno == EAGAIN)
        {
          // There is no event.
          // So, SKIP.
        }
      else
        {
          LOG (LOGGING_LEVEL_HIGH, "[Input Process] read from readkey error (read_sz = %d)", read_sz);
          return -1;
        }
    }

  // Only read the key press event.
  if (ev.value == KEY_PRESS)
    {
      switch (ev.code)
        {
          case KEY_CODE_BACK:
            input_message_back_send (pipe_fd);
            terminated = true;
          break;
          case KEY_CODE_VOL_UP:
            input_message_vol_up_send (pipe_fd);
          break;
          case KEY_CODE_VOL_DOWN:
            input_message_vol_down_send (pipe_fd);
          break;
          default:
            ;/* skip when other codes */
        }
    }

  return 0;
}

static int process_switch (int switch_fd, int pipe_fd, bool first)
{
  static unsigned char prev_push[BUTTON_NUM];

  if (first)
    {
      read (switch_fd, prev_push, sizeof (prev_push));
    }
  else
    {
      unsigned char cur_push[BUTTON_NUM];
      read (switch_fd, cur_push, sizeof (cur_push));
      for (int i = 0; i < BUTTON_NUM; ++i)
        {
          if (prev_push[i] != cur_push[i])
            {
              input_message_switch_send (pipe_fd, i + 1);
              prev_push[i] = cur_push[i];
            }
        }
    }

  return 0;
}

int input_process_main (int pipe_fd)
{
  int readkey_fd, switch_fd;

  if ((readkey_fd = open (DEVICE_READKEY, O_RDONLY | O_NONBLOCK)) == -1)
    perror ("[Input Process] open readkey device error : ");

  if ((switch_fd = open (DEVICE_SWITCH, O_RDWR)) == -1)
    perror ("[Input Process] open switch device error : ");

  process_switch (switch_fd, pipe_fd, true);
  while (!terminated)
    {
      process_readkey (readkey_fd, pipe_fd);
      process_switch (switch_fd, pipe_fd, false);
    }

  return 0;
}