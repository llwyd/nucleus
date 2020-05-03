CC = xtensa-lx106-elf-gcc
CFLAGS = -I. -mlongcalls -I../../../esp-open-sdk/sdk/driver_lib/include -I../../../esp-open-sdk/sdk/include
LDLIBS = -L. -L../../../esp-open-sdk/sdk/lib -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -ldriver -Wl,--end-group -lgcc 
LDFLAGS = -Teagle.app.v6.ld

node-0x00000.bin: node
	esptool.py elf2image $^

node: node.o

node.o: node.c

flash: node-0x00000.bin
	esptool.py write_flash 0 node-0x00000.bin 0x10000 node-0x10000.bin
clean:
	rm -f node node.o node-0x00000.bin node-0x10000.bin
