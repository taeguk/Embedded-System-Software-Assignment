#Makefile for a basic kernel module

APP	= dev_driver_test

all: app

app:
	arm-none-linux-gnueabi-gcc -static -o $(APP) $(APP).c

install:
	cp -a fpga_text_lcd_driver.ko /nfsroot
	cp -a $(APP) /nfsroot

clean:
	rm -rf *.o
	rm -rf $(APP)
