#!/bin/sh

if [ ! build/ ]; then
    mkdir build
fi

cd "$(dirname "$0")"
cd build/
sudo openocd -f interface/cmsis-dap.cfg -c "transport select swd" -f target/rp2040.cfg -c "adapter speed 5000" -s tcl

