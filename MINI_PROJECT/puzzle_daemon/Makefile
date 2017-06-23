CC = arm-none-linux-gnueabi-gcc
OUT = puzzle_daemon
SRC = $(OUT).c

all: $(OUT)

$(OUT): $(SRC)
		$(CC) -static $(SRC) -o $(OUT)

clean:
	rm -rf $(OUT)
