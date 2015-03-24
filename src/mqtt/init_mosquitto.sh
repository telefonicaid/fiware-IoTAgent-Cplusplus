#!/bin/bash

/usr/sbin/mosquitto -c /etc/iot/mosquitto.conf &
pid=$!
echo $pid > /var/run/iot/mosquitto.pid
