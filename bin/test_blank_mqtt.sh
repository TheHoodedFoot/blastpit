#!/bin/sh
for i in $(seq 1 20)
do
	mosquitto_pub -h rfbevan.co.uk -t lmos -m "<command id=\"${i}\">99</command>"
done
