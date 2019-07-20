#!/bin/sh

# Are rules enabled?
INPUT_ENABLED=$(sudo iptables -nL INPUT --line-numbers | grep spt:1883 | head -n1 | awk '{ print $1}' )
OUTPUT_ENABLED=$(sudo iptables -nL OUTPUT --line-numbers | grep dpt:1883 | head -n1 | awk '{ print $1}' )

if [[ -z "${INPUT_ENABLED}" ]]
then
	echo "Disabling MQTT communications..."
	sudo iptables --append INPUT --protocol tcp --sport 1883 --jump REJECT
	sudo iptables --append OUTPUT --protocol tcp --dport 1883 --jump REJECT
else
	echo "Enabling MQTT communications..."
	sudo iptables --delete INPUT ${INPUT_ENABLED}
	sudo iptables --delete OUTPUT ${OUTPUT_ENABLED}
fi
