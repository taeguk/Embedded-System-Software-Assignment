#include <stdio.h>
#include <unistd.h>
#include "process/input_process.h"
#include "process/main_process.h"
#include "process/output_process.h"

int main ()
{
  int input_pid, output_pid;
  int input_pipe_fd[2], output_pipe_fd[2];

  if (pipe (input_pipe_fd) == -1)
    perror ("input pipe error : ");
  if ((input_pid = fork ()) == -1)
    perror ("input process fork error : ");

  if (input_pid > 0)
    {
      if (pipe (output_pipe_fd) == -1)
        perror ("output pipe error : ");
      if ((output_pid = fork ()) == -1)
        perror ("output process fork error : ");

      if (output_pid > 0)
        {
          // I'm main process.
          close (input_pipe_fd[1]);
          close (output_pipe_fd[0]);
          main_process_main (input_pipe_fd[0], output_pipe_fd[1]);
        }
      else
        {
          // I'm output process.
          close (input_pipe_fd[0]);
          close (input_pipe_fd[1]);
          close (output_pipe_fd[1]);
          output_process_main (output_pipe_fd[0]);
        }
    }
  else
    {
      // I'm input process
      close (input_pipe_fd[0]);
      input_process_main (input_pipe_fd[1]);
    }

  return 0;
}
