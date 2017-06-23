obj-m = puzzle_dev.o 
CC := arm-none-linux-gnueabi-gcc

KDIR := /work/achroimx_kernel/
PWD := $(shell pwd)

all: 
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean : 
	rm -rf *.o
	rm -rf *.ko
	rm -rf *.mod.c
	rm -rf *.order
	rm -rf *.symvers
	rm -rf .*.cmd
	rm -rf .tmp_versions
