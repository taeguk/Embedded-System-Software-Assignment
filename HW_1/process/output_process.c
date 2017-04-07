//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include "output_process.h"
#include "../message/output_message.h"
#include "../common/logging.h"

#define DEVICE_FND "/dev/fpga_fnd"
#define DEVICE_TEXT_LCD "/dev/fpga_text_lcd"
#define DEVICE_LED "/dev/fpga_led"
#define DEVICE_DOT_MATRIX "/dev/fpga_dot"

static bool terminated = false;

static int process_message (const struct output_message_header *msg_header, void *msg_body);
static int message_h_fnd (fnd_data_t data);
static int message_h_text_lcd (const struct text_lcd_data *data);
static int message_h_led (union led_data data);
static int message_h_dot_matrix (const struct dot_matrix_data *data);
static int message_h_terminate ();

int output_process_main (int pipe_fd)
{
  while (!terminated)
    {
      int read_sz;
      struct output_message_header msg_header;

      /* Read message header */
      read_sz = read (pipe_fd, &msg_header, sizeof (msg_header));
      if (read_sz == -1)
        {
          // ERROR.
          perror ("[Output Process] input pipe read error : ");
        }
      else if (read_sz != sizeof (msg_header))
        {
          // ERROR.
          LOG (LOGGING_LEVEL_HIGH,
               "[Output Process] input pipe read_sz = %d (expected %u).",
               read_sz, sizeof (msg_header));
          return -1;
        }

      /* Read message body */
      void *msg_body= malloc (msg_header.body_size);
      read_sz = 0;
      while (read_sz < msg_header.body_size)
        {
          int tmp_read_sz;
          tmp_read_sz = read (pipe_fd, msg_body + read_sz, msg_header.body_size - read_sz);
          if (tmp_read_sz == -1)
            {
              if (errno == EAGAIN)
                continue;
              else
                perror ("[Output Process] input pipe read error : ");
            }
          read_sz += tmp_read_sz;
        }

      /* Process message */
      if (process_message (&msg_header, msg_body) == -1)
        {
          LOG (LOGGING_LEVEL_HIGH, 
               "[Output Process] Process message error (msg type = %d).", msg_header.type);
        }

      free (msg_body);
    }
  
  close (pipe_fd);

  LOG (LOGGING_LEVEL_NORMAL, "[Output Process] I'm gracefully dead.");

  return 0;
}

static int process_message (const struct output_message_header *msg_header, void *msg_body)
{
  switch (msg_header->type)
    {
      case OUTPUT_MESSAGE_TYPE_FND:
        LOG (LOGGING_LEVEL_LOW, "[Output Process] Recv output_message (FND - %d).", *((fnd_data_t *) msg_body));
        return message_h_fnd ( *((fnd_data_t *) msg_body) );

      case OUTPUT_MESSAGE_TYPE_LED:
        LOG (LOGGING_LEVEL_LOW, "[Output Process] Recv output_message (LED - %d).", ((union led_data *) msg_body)->val);
        return message_h_led ( *((union led_data *) msg_body) );

      case OUTPUT_MESSAGE_TYPE_TEXT_LCD:
        LOG (LOGGING_LEVEL_LOW, "[Output Process] Recv output_message (TEXT_LCD - %d).", 
             ((struct text_lcd_data *) msg_body)->len);
        return message_h_text_lcd ( (struct text_lcd_data *) msg_body);

      case OUTPUT_MESSAGE_TYPE_DOT_MATRIX:
        LOG (LOGGING_LEVEL_LOW, "[Output Process] Recv output_message (DOT_MATRIX).");
        return message_h_dot_matrix ( (struct dot_matrix_data *) msg_body);

      case OUTPUT_MESSAGE_TYPE_TERMINATE:
        LOG (LOGGING_LEVEL_LOW, "[Output Process] Recv output_message (TERMINATE).");
        return message_h_terminate ();
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Output Process] strange message type : %d.", msg_header->type);
        return -1;
    }
}

static int message_h_fnd (fnd_data_t data)
{
  unsigned char dev_data[4] = { 0, };
  for (int i = 3; i >= 0; --i)
    {
      dev_data[i] = data % 10;
      data /= 10;
    }

  int fd = open (DEVICE_FND, O_WRONLY);
  if (fd == -1)
    return -1;

  write (fd, dev_data, sizeof (dev_data));
  close (fd);
  return 0;
}

static int message_h_text_lcd (const struct text_lcd_data *data)
{
  char dev_data[MAX_LCD_STRING_LEN] = { [0 ... MAX_LCD_STRING_LEN-1] = ' ' };
  int fd = open (DEVICE_TEXT_LCD, O_WRONLY);
  if (fd == -1)
    return -1;

  int over_len = data->len > MAX_LCD_STRING_LEN ? data->len - MAX_LCD_STRING_LEN : 0;

  strncpy (dev_data, data->str + over_len, data->len - over_len);

  write (fd, dev_data, sizeof (dev_data));
  close (fd);
  return 0;
}

static int message_h_led (union led_data data)
{
  int fd = open (DEVICE_LED, O_WRONLY);
  if (fd == -1)
    return -1;

  write (fd, &data.val, sizeof (data.val));
  close (fd);
  return 0;
}

static int message_h_dot_matrix (const struct dot_matrix_data *data)
{
  unsigned char dev_data[DOT_MATRIX_HEIGHT] = { 0, };
  for (int i = 0; i < DOT_MATRIX_HEIGHT; ++i)
    {
      for (int j = 0; j < DOT_MATRIX_WIDTH; ++j)
        {
          if (data->data[i][j] == 1)
            dev_data[i] |= (1 << (DOT_MATRIX_WIDTH-1-j));
        }
      LOG (LOGGING_LEVEL_DEBUG, "[Output Process] %d - %02X", i, dev_data[i]);
    }

  int fd = open (DEVICE_DOT_MATRIX, O_WRONLY);
  if (fd == -1)
    return -1;

  write (fd, dev_data, sizeof (dev_data));
  close (fd);
  return 0;
}

static int message_h_terminate ()
{
  terminated = true;
  return true;
}
