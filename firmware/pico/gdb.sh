#!/bin/sh

arm-none-eabi-gdb -ex "target remote localhost:3333" build/node.elf

