#!/bin/sh

echo "Restarting UWSGI Daemon"
systemctl restart uwsgi.service
echo "Waiting 30s..."
sleep 30
echo "Restarting MQTT Daemon"
systemctl restart mosquitto.service
echo "FIN"

