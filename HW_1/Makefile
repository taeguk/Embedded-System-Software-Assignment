SOURCES=$(shell find . -type f -iname '*.c')
TARGET=HW_1_20141500.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	arm-none-linux-gnueabi-gcc --std=gnu99 $(SOURCES) -o $(TARGET) -lpthread -lrt -W -Wall -Wno-sign-compare

clean:
	rm -f $(TARGET)