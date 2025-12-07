#!/bin/sh

mosquitto_pub -h localhost -t home/meta/clr -m "1" --qos 1
mosquitto_pub -h localhost -t home/reqmeta -m "1" --qos 1
