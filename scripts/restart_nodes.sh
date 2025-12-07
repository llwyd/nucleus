#!/bin/sh

mosquitto_pub -h localhost -t home/reset -m "1" --qos 1

