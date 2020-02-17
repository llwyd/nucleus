#!/bin/sh

# Sometimes the package managers bork 'coz IPv6
# https://stackoverflow.com/questions/7366775/what-does-the-line-bin-sh-mean-in-a-unix-shell-script

sudo sysctl net.ipv6.conf.all.disable_ipv6=1
