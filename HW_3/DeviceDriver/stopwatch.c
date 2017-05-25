#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>

#define IOM_FND_ADDRESS 0x08000004 // pysical address

static int stopwatch_open(struct inode *, struct file *);
static int stopwatch_release(struct inode *, struct file *);
static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

static irqreturn_t interrupt_home_handler(int irq, void* dev_id);
static irqreturn_t interrupt_back_handler(int irq, void* dev_id);
static irqreturn_t interrupt_volup_handler(int irq, void* dev_id);
static irqreturn_t interrupt_voldown_handler(int irq, void* dev_id);

static void update_fnd(void);
static void clear_stopwatch(void);
static void timer_callback(unsigned long _);

/////////////////////////////////////////////////////////////

static struct file_operations stopwatch_fops =
    {
        .open = stopwatch_open,
        .write = stopwatch_write,
        .release = stopwatch_release,
    };

static unsigned char *iom_fpga_fnd_addr;

static int stopwatch_usage = 0;
static int stopwatch_major = 0, stopwatch_minor = 0;
static dev_t stopwatch_dev;
static struct cdev stopwatch_cdev;

volatile int wq_blocking = 1;
wait_queue_head_t wq_write;
DECLARE_WAIT_QUEUE_HEAD(wq_write);

static int timer_set = 0;
static struct timer_list timer;
static int stopwatch_seconds = 0;

static u64 prev_hz = 1;

/////////////////////////////////////////////////////////////

static void clear_stopwatch(void)
{
  if (timer_set)
    {
      del_timer_sync (&timer);
      timer_set = 0;
    }
  stopwatch_seconds = 0;
  update_fnd ();
}

static void update_fnd(void)
{
  int mins = (stopwatch_seconds / 60) % 60;
  int secs = stopwatch_seconds % 60;
  unsigned short fnd_val =
      (mins / 10) << 12 | (mins % 10) << 8 |
      (secs / 10) << 4 | (secs % 10);

  outw(fnd_val, (unsigned int) iom_fpga_fnd_addr);
}

static void timer_callback(unsigned long _)
{
  ++stopwatch_seconds;
  update_fnd();
  timer.expires = get_jiffies_64() + HZ;
  timer.function = &timer_callback;
  add_timer (&timer);
}

static irqreturn_t interrupt_home_handler(int irq, void* dev_id)
{
  printk(KERN_ALERT "HOME button clicked. %x\n", gpio_get_value(IMX_GPIO_NR(1, 11)));

  if (!timer_set)
    {
      timer_set = 1;
      timer.expires = get_jiffies_64() + HZ;
      timer.function = &timer_callback;
      add_timer (&timer);
    }

  return IRQ_HANDLED;
}

static irqreturn_t interrupt_back_handler(int irq, void* dev_id)
{
  printk(KERN_ALERT "BACK button clicked. %x\n", gpio_get_value(IMX_GPIO_NR(1, 12)));

  if (timer_set)
    {
      del_timer_sync (&timer);
      timer_set = 0;
    }

  return IRQ_HANDLED;
}

static irqreturn_t interrupt_volup_handler(int irq, void* dev_id)
{
  printk(KERN_ALERT "VOLUP button clicked. %x\n", gpio_get_value(IMX_GPIO_NR(2, 15)));

  clear_stopwatch();

  return IRQ_HANDLED;
}

static irqreturn_t interrupt_voldown_handler(int irq, void* dev_id)
{
  unsigned int val = gpio_get_value(IMX_GPIO_NR(5, 14));

  printk(KERN_ALERT "VOLDOWN button clicked. %x\n", val);

  if (val == 0)
    {
      prev_hz = get_jiffies_64();
    }
  else
    {
      u64 cur_hz = get_jiffies_64();
      if (cur_hz - prev_hz >= 3*HZ)
        {
          clear_stopwatch();
          wq_blocking = 0;
          wake_up_interruptible(&wq_write);
        }
      prev_hz = cur_hz;
    }

  return IRQ_HANDLED;
}

static int stopwatch_open(struct inode *minode, struct file *mfile)
{
  int ret;
  int irq;

  if(stopwatch_usage != 0)
    return -EBUSY;

  stopwatch_usage = 1;

  printk(KERN_ALERT "stopwatch: open.\n");

  // int for home
  gpio_direction_input(IMX_GPIO_NR(1,11));
  irq = gpio_to_irq(IMX_GPIO_NR(1,11));
  printk(KERN_ALERT "HOME - IRQ Number : %d\n",irq);
  ret=request_irq(irq, &interrupt_home_handler, IRQF_TRIGGER_FALLING, "home", 0);

  // int for back
  gpio_direction_input(IMX_GPIO_NR(1,12));
  irq = gpio_to_irq(IMX_GPIO_NR(1,12));
  printk(KERN_ALERT "BACK - IRQ Number : %d\n",irq);
  ret=request_irq(irq, &interrupt_back_handler, IRQF_TRIGGER_FALLING, "back", 0);

  // int for volup
  gpio_direction_input(IMX_GPIO_NR(2,15));
  irq = gpio_to_irq(IMX_GPIO_NR(2,15));
  printk(KERN_ALERT "VOLUP - IRQ Number : %d\n",irq);
  ret=request_irq(irq, &interrupt_volup_handler, IRQF_TRIGGER_FALLING, "volup", 0);

  // int for voldown
  gpio_direction_input(IMX_GPIO_NR(5,14));
  irq = gpio_to_irq(IMX_GPIO_NR(5,14));
  printk(KERN_ALERT "VOLDOWN - IRQ Number : %d\n",irq);
  ret=request_irq(irq, &interrupt_voldown_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "voldown", 0);

  return 0;
}

static int stopwatch_release(struct inode *minode, struct file *mfile)
{
  free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

  stopwatch_usage = 0;
  printk(KERN_ALERT "stopwatch: release.\n");

  return 0;
}

static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos )
{
  while (wq_blocking)
    {
      printk(KERN_NOTICE "sleep on.\n");
      interruptible_sleep_on(&wq_write);
      printk(KERN_NOTICE "wake up.\n");
    }

  wq_blocking = 1;
  printk(KERN_NOTICE "write\n");

  return 0;
}

static int stopwatch_register_cdev(void)
{
  int error;
  if(stopwatch_major)
    {
      stopwatch_dev = MKDEV(stopwatch_major, stopwatch_minor);
      error = register_chrdev_region(stopwatch_dev, 1, "stopwatch");
    }
  else
    {
      error = alloc_chrdev_region(&stopwatch_dev, stopwatch_minor, 1, "stopwatch");
      stopwatch_major = MAJOR(stopwatch_dev);
    }
  if(error < 0)
    {
      printk(KERN_WARNING "stopwatch: can't get major %d\n", stopwatch_major);
      return -1;
    }

  printk(KERN_ALERT "stopwatch: major number = %d\n", stopwatch_major);
  cdev_init(&stopwatch_cdev, &stopwatch_fops);
  stopwatch_cdev.owner = THIS_MODULE;
  stopwatch_cdev.ops = &stopwatch_fops;
  error = cdev_add(&stopwatch_cdev, stopwatch_dev, 1);
  if(error)
    {
      printk(KERN_NOTICE "stopwatch Register Error %d\n", error);
      return -1;
    }

  return 0;
}

static int __init my_module_init(void)
{
  int result;
  if((result = stopwatch_register_cdev()) < 0 )
    return result;

  iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);

  init_timer(&timer);

  printk(KERN_ALERT "Init Module Success \n");
  printk(KERN_ALERT "Device : /dev/stopwatch, Major Num : %d\n", stopwatch_major);

  return 0;
}

static void __exit my_module_exit(void)
{
  iounmap(iom_fpga_fnd_addr);

  if (timer_set)
    del_timer_sync(&timer);

  cdev_del(&stopwatch_cdev);
  unregister_chrdev_region(stopwatch_dev, 1);

  printk(KERN_ALERT "Remove Module Success \n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Taeguk Kwon");
