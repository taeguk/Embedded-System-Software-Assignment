SOURCES=$(shell find . -type f -iname '*.c')
TARGET=HW_1_20141500.out

all: $(TARGET)

$(TARGET): $(SOURCES)
	arm-none-linux-gnueabi-gcc -static --std=gnu99 $(SOURCES) -o $(TARGET) \
	  -lpthread -lrt \
	  -W -Wall -Wno-sign-compare -Wno-missing-field-initializers

clean:
	rm -f $(TARGET)
