#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>

#include "../common/puzzle_data.h"
#include "../common/puzzle_dev.h"

int daemon_main()
{
  unsigned char pushsw[9];
  int push_switch_fd = open("/dev/fpga_push_switch", O_RDWR);
  int puzzle_fd = open("/dev/puzzle_dev", O_RDWR);
 
  int puzzle_enable = 0;

  if (push_switch_fd < 0)
    {
      syslog (LOG_NOTICE, "device open error: push_switch_fd < 0\n");
      exit(1);
    }
    
  if (puzzle_fd < 0)
    {
      syslog (LOG_NOTICE, "device open error: puzzle_fd < 0\n");
      exit(1);
    }

  while (1)
    {
      read(push_switch_fd, &pushsw, 9);

      if (pushsw[0])
        {
          pushsw[0] = 0;
          puzzle_enable ^= 1;

          ioctl(puzzle_fd, PUZZLE_DATA_ENABLE, &puzzle_enable/* data */);
          memset(pushsw, 0x00, 9);
        }
      usleep(200000);
    }
}

static void start_skeleton_daemon()
{
  pid_t pid;

  /* Fork off the parent process */
  pid = fork();

  /* An error occurred */
  if (pid < 0)
    exit(EXIT_FAILURE);

  /* Success: Let the parent terminate */
  if (pid > 0)
    exit(EXIT_SUCCESS);

  /* On success: The child process becomes session leader */
  if (setsid() < 0)
    exit(EXIT_FAILURE);

  /* Catch, ignore and handle signals */
  //TODO: Implement a working signal handler */
  signal(SIGCHLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  /* Fork off for the second time*/
  pid = fork();

  /* An error occurred */
  if (pid < 0)
    exit(EXIT_FAILURE);

  /* Success: Let the parent terminate */
  if (pid > 0)
    exit(EXIT_SUCCESS);

  /* Set new file permissions */
  umask(0);

  /* Change the working directory to the root directory */
  /* or another appropriated directory */
  chdir("/");

  /* Close all open file descriptors */
  int x;
  for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
      close (x);
    }

  /* Open the log file */
  openlog ("puzzle_daemon", LOG_PID, LOG_DAEMON);
}

int main()
{
  int ret;

  start_skeleton_daemon();

  syslog (LOG_NOTICE, "Puzzle daemon started.");

  ret = daemon_main();

  syslog (LOG_NOTICE, "Puzzle daemon terminated.");
  closelog();

  return ret;
}

