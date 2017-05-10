#include <linux/kernel.h>
#include <linux/uaccess.h>

union stream
  {
    struct
      {
        unsigned char pos; // 0 ~ 3
        unsigned char val; // 1 ~ 8
        unsigned char count;
        unsigned char interval;
      } data;
    unsigned int result;
  };

asmlinkage long sys_homework2(int interval, int count, int start_option, unsigned int *result)
{
  union stream stream;
  int i;

  printk(KERN_INFO"sys_homework2 interval = %d, count = %d, start_option = %d\n", 
         interval, count, start_option);

  if (interval < 1 || interval > 100)
    {
      printk(KERN_INFO"interval must be [1 - 100]\n");
      return -1;
    }
  if (count < 1 || count > 100)
    {
      printk(KERN_INFO"count must be [1 - 100]\n");
      return -1;
    }
  if (start_option < 1 || start_option > 8000)
    {
      printk(KERN_INFO"start_option must be [0001 - 8000]\n");
      return -1;
    }
  
  stream.data.val = 0;
  for (i = 3; i >= 0; --i)
    {
      unsigned char digit = start_option % 10;
      if (digit > 8)
        {
          printk(KERN_INFO"start_option's digit must be [1 - 8]\n");
          return -1;
        }
      else if (digit > 0)
        {
          if (stream.data.val == 0)
            {
              stream.data.pos = i;
              stream.data.val = digit;
            }
          else
            {
              printk(KERN_INFO"start_option must have only one significant digit.\n");
              return -1;
            }
        }
      start_option /= 10;
    }

  stream.data.count = (unsigned char) count;
  stream.data.interval = (unsigned char) interval;

  copy_to_user(result, &stream, sizeof(stream));

  return 0;
}
