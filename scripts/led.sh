#!/bin/sh

if [ $1 = "on" ]
	then
		mosquitto_pub -h localhost -t home/livingroom/onboard_led -m "1"
		mosquitto_pub -h localhost -t home/lounge_area/debug_led -m "1"
		mosquitto_pub -h localhost -t home/bedroom_area/debug_led -m "1"
	else
		mosquitto_pub -h localhost -t home/livingroom/onboard_led -m "0"
		mosquitto_pub -h localhost -t home/lounge_area/debug_led -m "0"
		mosquitto_pub -h localhost -t home/bedroom_area/debug_led -m "0"
fi
