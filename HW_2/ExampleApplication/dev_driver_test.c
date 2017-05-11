/* FPGA Text LCD Test Application
File : fpga_test_text_lcd.c
Auth : largest@huins.com */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DEVICE_NAME "/dev/dev_driver"

int main(int argc, char **argv)
{
  int i;
  int dev;
  int interval, count, start_option, run_option;
  unsigned int data;
  int ret;

  if(argc != 5) 
    {
      printf("Invalid Value Arguments! %s <interval (1 ~ 100)> <count (1 ~ 100)> <start option (0001 ~ 8000)> <run option (0 : write, otherwise : ioctl)>\n", argv[0]);
      return -1;
    }

  interval = atoi(argv[1]);
  count = atoi(argv[2]);
  start_option = atoi(argv[3]);
  run_option = atoi(argv[4]);

  ret = syscall(376, interval, count, start_option, &data);
  if (ret != 0)
    {
      printf("An error occurs when calling syscall-homework2.\n");
      return -1;
    }

  dev = open(DEVICE_NAME, O_WRONLY);
  if (dev < 0) 
    {
      printf("Device open error : %s\n", DEVICE_NAME);
      return -1;
    }

  if (run_option == 0)
    write(dev, &data, sizeof(data));
  else
    ioctl(dev, 0, &data);

  close(dev);

  return(0);
}
