#!/usr/bin/python3
import RPi.GPIO as gpio
import datetime

on = 9
off = 18

pins = (22, 23)
lights = 17

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

gpio.setup(lights, gpio.OUT)
for x in pins:
	gpio.setup(x, gpio.OUT)


while True:
	now = datetime.datetime.now()
	if now.hour >= on and now.hour < off:
		gpio.output(lights, 0)
	else:
		gpio.output(lights, 1)

	for x in pins:
		gpio.output(x, 0)

