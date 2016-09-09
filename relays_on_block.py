#!/usr/bin/python3
import RPi.GPIO as gpio

pins = (17, 22, 23)

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

while True:
	for x in pins:
		gpio.setup(x, gpio.OUT)
		gpio.output(x, 0)

