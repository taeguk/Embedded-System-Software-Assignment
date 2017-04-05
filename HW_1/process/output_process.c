//
// Created by taeguk on 2017-04-05.
//

#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include "output_process.h"
#include "../message/output_message.h"
#include "../common/logging.h"

static bool terminated = false;

static int process_message (const struct output_message_header *msg_header, void *msg_body);
static int message_h_fnd (fnd_data_t data);
static int message_h_text_lcd (struct text_lcd_data data);
static int message_h_led (union led_data data);
static int message_h_dot_matrix (struct dot_matrix_data data);

int output_process_main (int pipe_fd)
{
  if (fcntl (pipe_fd, F_SETFL, fcntl(pipe_fd, F_GETFL) | O_NONBLOCK) == -1)
    perror ("[Output Process] fcntl error : ");

  while (!terminated)
    {
      int read_sz;
      struct output_message_header msg_header;

      read_sz = read (pipe_fd, &msg_header, sizeof (msg_header));
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
              perror ("[Output Process] input pipe read error : ");
            }
        }
      else if (read_sz != sizeof (msg_header))
        {
          // ERROR.
          LOG (LOGGING_LEVEL_HIGH,
               "[Output Process] input pipe read_sz = %d (expected %u).",
               read_sz, sizeof (msg_header));
          return -1;
        }

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

      process_message (&msg_header, msg_body);

      free (msg_body);
    }

  return 0;
}

static int process_message (const struct output_message_header *msg_header, void *msg_body)
{
  switch (msg_header->type)
    {
      case OUTPUT_MESSAGE_TYPE_FND:
        return message_h_fnd ( *((fnd_data_t *) msg_body) );
      case OUTPUT_MESSAGE_TYPE_LED:
        return message_h_led ( *((union led_data *) msg_body) );
      case OUTPUT_MESSAGE_TYPE_TEXT_LCD:
        return message_h_text_lcd ( *((struct text_lcd_data *) msg_body) );
      case OUTPUT_MESSAGE_TYPE_DOT_MATRIX:
        return message_h_dot_matrix ( *((struct dot_matrix_data *) msg_body) );
      default:
        LOG (LOGGING_LEVEL_HIGH, "[Output Process] strange message type : %d.", msg_header->type);
        return -1;
    }
}

static int message_h_fnd (fnd_data_t data)
{
  // TODO:
  (void) data;
  return 0;
}

static int message_h_text_lcd (struct text_lcd_data data)
{
  // TODO:
  (void) data;
  return 0;
}

static int message_h_led (union led_data data)
{
  // TODO:
  (void) data;
  return 0;
}

static int message_h_dot_matrix (struct dot_matrix_data data)
{
  // TODO:
  (void) data;
  return 0;
}