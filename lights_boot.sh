#!/bin/bash

time_on_main=0900
time_off_main=2000

time_on_twilight=0700
time_off_twilight=2300

time_on_main=$(echo $time_on_main | sed 's/^0*//')
time_off_main=$(echo $time_off_main | sed 's/^0*//')
time_on_twilight=$(echo $time_on_twilight | sed 's/^0*//')
time_off_twilight=$(echo $time_off_twilight | sed 's/^0*//')

NOW=$(date +"%H%M")

if (($NOW >= $time_on_main && $NOW < $time_off_main)); then
        python3 /home/pi/lights_switch.py 23 -on
        python3 /home/pi/lights_switch.py 24 -off
elif (($NOW >= $time_on_twilight && $NOW < $time_off_twilight)); then
        python3 /home/pi/lights_switch.py 23 -off
        python3 /home/pi/lights_switch.py 24 -on
else
        python3 /home/pi/lights_switch.py 23 -off
        python3 /home/pi/lights_switch.py 24 -off
fi
