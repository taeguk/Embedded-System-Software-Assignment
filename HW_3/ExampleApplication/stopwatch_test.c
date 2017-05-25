#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE_NAME "/dev/stopwatch"

int main(int argc, char **argv)
{
  int dev;
  unsigned char data;
  int ret;

  dev = open(DEVICE_NAME, O_WRONLY);
  if (dev < 0) 
    {
      printf("Device open error : %s\n", DEVICE_NAME);
      return -1;
    }

  write(dev, &data, sizeof(data));

  close(dev);

  return(0);
}
