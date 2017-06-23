#include <linux/string.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#include <linux/ioctl.h>

#include "../common/puzzle_dev.h"
#include "../common/puzzle_data.h"

#include <asm/uaccess.h>

int __init puzzle_init(void);
void __exit puzzle_exit(void);

int puzzle_open(struct inode *, struct file *);
int puzzle_release(struct inode *, struct file *);
long puzzle_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);


struct __puzzle_data puzzle_data = 
{
  .row = 2,
  .col = 2,
  .mat = { { 2, 3 }, { 0, 1 } }
};
static int puzzle_enable;


/* variables about this module */
static struct file_operations puzzle_fops =
{
  .open = puzzle_open,
  .release = puzzle_release,
  .unlocked_ioctl = puzzle_ioctl,
};


int
puzzle_open(struct inode *minode, struct file *mfile)
{
  printk("puzzle_open\n");
  return 0;
}

int
puzzle_release(struct inode *minode, struct file *mfile)
{
  printk("puzzle_release\n");
  return 0;
}


/* ioctl_num : The number of the ioctl */
/* ioctl_param : The parameter to it */
long
puzzle_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  /* Switch according to the ioctl called */
  switch (ioctl_num)
    {
    case PUZZLE_PUT:
      (void) copy_from_user(&puzzle_data, (struct __puzzle_data*)ioctl_param, sizeof(struct __puzzle_data));

      printk("PUZZLE_PUT: %d %d\n", puzzle_data.row, puzzle_data.col);
      break;

    case PUZZLE_DATA_ENABLE:
        {
          int data;
          (void) copy_from_user(&data, (int*) ioctl_param, sizeof(int));

          if (data) // enable
            puzzle_enable = 1; // static global
          else // disable
            puzzle_enable = 0;
          printk("PUZZLE_DATA_ENABLE: %d\n", puzzle_enable);
        }
      break;

    case PUZZLE_GET:
        {
          (void) copy_to_user((struct __puzzle_data*) ioctl_param,
                       &puzzle_data,
                       sizeof(struct __puzzle_data));
          printk("PUZZLE_GET: %d %d\n", puzzle_data.row, puzzle_data.col);
        }
      break;
    case PUZZLE_GET_ENABLE:
        {
          (void) copy_to_user((int*) ioctl_param, &puzzle_enable, sizeof(int));
          printk("PUZZLE_DATA_ENABLE: %d\n", puzzle_enable);
        }
      break;
    }

  return 0;
}

int __init 
puzzle_init(void)
{
  int result;

  result = register_chrdev(PUZZLE_MAJOR, PUZZLE_NAME, &puzzle_fops);

  if(result <0)
    {
      printk( "puzzle_init error %d\n",result);
      return result;
    }
  
  printk("init module\n");
  return 0;
}

void __exit
puzzle_exit(void)
{
  printk("puzzle_exit\n");
  unregister_chrdev(PUZZLE_MAJOR, PUZZLE_NAME);
}

module_init( puzzle_init);
module_exit( puzzle_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Taeseung Lee");
