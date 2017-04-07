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

#define DEVICE_READKEY  "/dev/input/event0"
#define DEVICE_SWITCH "/dev/fpga_push_switch"
#define POLLING_DELAY (100*1000)  // polling delay (microseconds)

#define KEY_PRESS   1
#define KEY_RELEASE 0

#define KEY_CODE_BACK       158
#define KEY_CODE_VOL_UP     115
#define KEY_CODE_VOL_DOWN   114

bool terminated = false;

/* poll and process readkey device */
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
            LOG (LOGGING_LEVEL_NORMAL, "[Input Process] Event occurs. (Read Key - Back).");
            input_message_back_send (pipe_fd);
            terminated = true;
          break;
          case KEY_CODE_VOL_UP:
            LOG (LOGGING_LEVEL_NORMAL, "[Input Process] Event occurs. (Read Key - VOL_UP).");
            input_message_vol_up_send (pipe_fd);
          break;
          case KEY_CODE_VOL_DOWN:
            LOG (LOGGING_LEVEL_NORMAL, "[Input Process] Event occurs. (Read Key - VOL_DOWN).");
            input_message_vol_down_send (pipe_fd);
          break;
          default:
            if (ev.code != 0)
              LOG (LOGGING_LEVEL_NORMAL, "[Input Process] Event occurs. (Read Key - OTHERS, code : %d).", ev.code);
            /* skip when other codes */
        }
    }

  return 0;
}

/* poll and process switch device */
static int process_switch (int switch_fd, int pipe_fd)
{
  static unsigned char prev_push[SWITCH_BUTTON_NUM + 1] = { 0, };
  unsigned char cur_push[SWITCH_BUTTON_NUM + 1];
  bool pushed = false;

  read (switch_fd, cur_push + 1, SWITCH_BUTTON_NUM * sizeof (unsigned char));

#define CHECK_AND_SET(no) \
  if (cur_push[no] == 1 && prev_push[no] != 1) \
    { \
      data.bit_fields.s##no = 1; \
      pushed = true; \
    } \
  prev_push[no] = cur_push[no];

  union switch_data data;
  data.val = 0;

  CHECK_AND_SET(1);
  CHECK_AND_SET(2);
  CHECK_AND_SET(3);
  CHECK_AND_SET(4);
  CHECK_AND_SET(5);
  CHECK_AND_SET(6);
  CHECK_AND_SET(7);
  CHECK_AND_SET(8);
  CHECK_AND_SET(9);

#undef CHECK_AND_SET

  if (pushed)
    {
      LOG (LOGGING_LEVEL_NORMAL, "[Input Process] Event occurs. (Switch Push - no : %03X).", data.val);
      input_message_switch_send(pipe_fd, data);
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

  while (!terminated)
    {
      process_readkey (readkey_fd, pipe_fd);
      process_switch (switch_fd, pipe_fd);
      usleep (POLLING_DELAY);
    }

  close (readkey_fd);
  close (switch_fd);
  close (pipe_fd);
  
  LOG (LOGGING_LEVEL_NORMAL, "[Input Process] I'm gracefully dead.");

  return 0;
}
