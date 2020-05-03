CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc -I$../../esp-open-sdk/sdk/include/ -L$../../esp-open-sdk/sdk/lib
LDFLAGS = -Teagle.app.v6.ld

node-0x00000.bin: node
	esptool.py elf2image $^

node: node.o

node.o: node.c

flash: node-0x00000.bin
	esptool.py write_flash 0 node-0x00000.bin 0x10000 node-0x10000.bin
clean:
	rm -f node node.o node-0x00000.bin node-0x10000.bin
