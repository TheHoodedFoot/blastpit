#!/bin/sh

HOST=hilly
TOPIC=$(hostname -s)

echo "Watching for messages with topic ${TOPIC} on host ${HOST}..."
mosquitto_sub -v --qos 2 --host ${HOST} --topic ${TOPIC} --topic "client" --topic "server"
