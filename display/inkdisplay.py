import argparse
import paho.mqtt.client as mqtt
import time

rcv_lounge_temp = False
rcv_lounge_hum = False
rcv_bedroom_temp = False
rcv_bedroom_bum = False

lounge_temp = ""
lounge_hum = ""
bedroom_temp = ""
bedroom_hum = ""

client = mqtt.Client()

parser = argparse.ArgumentParser()
parser.add_argument("ip", help = "IP address of MQTT broker")
args = parser.parse_args()

broker_ip = args.ip

