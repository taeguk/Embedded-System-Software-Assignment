echo "7 6 1 7" > /proc/sys/kernel/printk

insmod external/fpga_push_switch_driver.ko
insmod puzzle_dev/puzzle_dev.ko
mknod /dev/puzzle_dev c 242 0
chmod 777 /dev/puzzle_dev
mknod /dev/fpga_push_switch c 265 0

chmod 700 puzzle_daemon/puzzle_daemon
puzzle_daemon/puzzle_daemon
