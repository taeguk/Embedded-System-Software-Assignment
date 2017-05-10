#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>

#include "fpga_dot_font.h"

#define MY_DEVICE_NAME "dev_driver"

#define IOM_LED_ADDRESS 0x08000016 // pysical address
#define IOM_FND_ADDRESS 0x08000004 // pysical address
#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address
#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090 // pysical address - 32 Byte (16 * 2)

#define LCD_LINE_MAX_LEN 16

static const char lcd_text[2][LCD_LINE_MAX_LEN+1] = 
  {
    "20141500",
    "Taeguk Kwon"
  };
static const int lcd_text_len[2] = { 8, 11 };
static int lcd_text_offset[2] = { 0, 0 };
static int lcd_text_dir[2] = { 1, 1 };

struct my_timer_data
  {
    unsigned char pos; // 0 ~ 3
    unsigned char val; // 1 ~ 8
    unsigned char count;
    unsigned char interval;
    struct timer_list timer;
  };

//Global variable
static unsigned char *iom_fpga_led_addr;
static unsigned char *iom_fpga_fnd_addr;
static unsigned char *iom_fpga_dot_addr;
static unsigned char *iom_fpga_text_lcd_addr;

static int my_dev_usage = 0;
static unsigned int my_dev_major_no;

static int timer_set = 0;
static struct my_timer_data my_timer_data;

static ssize_t run_timer(const char *data);
static void timer_callback(unsigned long arg);
static void do_action(struct my_timer_data *p_data);

ssize_t my_dev_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
int my_dev_open(struct inode *minode, struct file *mfile);
int my_dev_release(struct inode *minode, struct file *mfile);
long my_dev_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param);
// http://opensourceforu.com/2011/08/io-control-in-linux/

// define file_operations structure
struct file_operations my_dev_fops =
    {
        .owner	  	    =	THIS_MODULE,
        .open		    =	my_dev_open,
        .write		    =	my_dev_write,
        .release	    =	my_dev_release,
        .unlocked_ioctl =   my_dev_ioctl,
    };

// when device open, call this function
int my_dev_open(struct inode *minode, struct file *mfile)
{
  if(my_dev_usage != 0) return -EBUSY;

  my_dev_usage = 1;

  return 0;
}

// when device close, call this function
int my_dev_release(struct inode *minode, struct file *mfile)
{
  my_dev_usage = 0;

  return 0;
}

// when write to device, call this function
ssize_t my_dev_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
  printk(KERN_INFO"In device driver, write handler called.\n");
  return run_timer(gdata);
}

long my_dev_ioctl(struct file *f, unsigned int ioctl_num, unsigned long ioctl_param)
{
  printk(KERN_INFO"In device driver, ioctl handler called.\n");
  switch (ioctl_num)
    {
    case 0 :
      return run_timer((char *)(ioctl_param));
    default:
      printk(KERN_INFO"Invalid ioctl_num: %u.\n", ioctl_num);
      return -EINVAL;
    }

  return 0;
}

int __init my_module_init(void)
{
  int result;

  result = register_chrdev(0, MY_DEVICE_NAME, &my_dev_fops);
  if(result < 0) {
      printk(KERN_WARNING"Can't get any major\n");
      return result;
    }
  my_dev_major_no = result;

  iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
  iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
  iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
  iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);

  printk(KERN_NOTICE"init module, %s major number : %d\n", MY_DEVICE_NAME, my_dev_major_no);

  return 0;
}

void __exit my_module_exit(void)
{
  iounmap(iom_fpga_led_addr);
  iounmap(iom_fpga_fnd_addr);
  iounmap(iom_fpga_dot_addr);
  iounmap(iom_fpga_text_lcd_addr);

  if (timer_set)
    del_timer_sync(&my_timer_data.timer);

  unregister_chrdev(my_dev_major_no, MY_DEVICE_NAME);
}

static void do_action(struct my_timer_data *p_data)
{
  int i, j;
  unsigned short fnd_val;
  unsigned char lcd_val[32];

  // write to led.
  outw((unsigned short) p_data->val, (unsigned int) iom_fpga_led_addr);

  // write to fnd.
  fnd_val = p_data->val << (4 * (3 - p_data->pos));
  outw(fnd_val, (unsigned int) iom_fpga_fnd_addr);

  // write to dot matrix.
  for (i = 0; i < sizeof(fpga_number[p_data->val]); ++i)
    {
      unsigned short val = fpga_number[p_data->val][i] & 0x7F;
      outw(val, (unsigned int) iom_fpga_dot_addr + i * 2);
    }

  for (i = 0; i < 2; ++i)
    {
      for (j = 0; j < LCD_LINE_MAX_LEN; ++j)
        lcd_val[i * LCD_LINE_MAX_LEN + j] = ' ';

      for (j = 0; j < lcd_text_len[i]; ++j)
        lcd_val[i * LCD_LINE_MAX_LEN + j + lcd_text_offset[i]] = lcd_text[i][j];

      if (lcd_text_dir[i] == 1)
        {
          if (lcd_text_offset[i] + lcd_text_len[i] >= LCD_LINE_MAX_LEN)
            lcd_text_dir[i] = -1;
        }
      else if (lcd_text_dir[i] == -1)
        {
          if (lcd_text_offset[i] <= 0)
            lcd_text_dir[i] = 1;
        }

      lcd_text_offset[i] += lcd_text_dir[i];
    }
  for (i = 0; i < 32; i+=2)
    {
      unsigned short value = (lcd_val[i] & 0xFF) << 8 | (lcd_val[i + 1] & 0xFF);
      outw(value, (unsigned int)iom_fpga_text_lcd_addr+i);
    }

  if (++p_data->val > 8)
    {
      p_data->val = 1;
      if (++p_data->pos > 3)
        p_data->pos = 0;
    }
}

static void timer_callback(unsigned long arg)
{
  struct my_timer_data *p_data = (struct my_timer_data *) arg;

  if (--p_data->count == 0)
    {
      printk(KERN_INFO"Timer callback terminated gracefully.\n");
      timer_set = 0;
      return;
    }
  printk(KERN_INFO"Timer callback is called. (pos = %d, val = %d, count = %d, interval = %d)\n", 
         p_data->pos, p_data->val, p_data->count, p_data->interval);

  do_action (p_data);

  p_data->timer.expires = get_jiffies_64() + (p_data->interval * HZ / 10);
  p_data->timer.data = (unsigned long) p_data;
  p_data->timer.function = timer_callback;

  add_timer(&p_data->timer);
}

static ssize_t run_timer(const char *data)
{
  const ssize_t data_len = 4;

  if (timer_set)
    del_timer_sync(&my_timer_data.timer);

  init_timer(&my_timer_data.timer);
  lcd_text_offset[0] = lcd_text_offset[1] = 0;
  lcd_text_dir[0] = lcd_text_dir[1] = 1;

  if (copy_from_user(&my_timer_data, data, data_len))
    return -EFAULT;

  printk(KERN_INFO"New timer is set. (pos = %d, val = %d, count = %d, interval = %d\n", 
         my_timer_data.pos, my_timer_data.val, my_timer_data.count, my_timer_data.interval);

  my_timer_data.timer.expires = get_jiffies_64() + (my_timer_data.interval * HZ / 10);
  my_timer_data.timer.data = (unsigned long) &my_timer_data;
  my_timer_data.timer.function = timer_callback;
  
  do_action (&my_timer_data);

  timer_set = 1;
  add_timer(&my_timer_data.timer);

  return data_len;
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taeguk Kwon");
