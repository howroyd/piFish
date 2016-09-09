#!/usr/bin/python3

#from phue import Bridge
import RPi.GPIO as gpio
import argparse, time

def hex2rgb(value):
	value = value.lstrip('#')
	lv = len(value)
	return tuple(int(value[i:i+lv//3],16) for i in range(0, lv, lv//3))

def set_rgb(pins, rgb):
	for x in range(3):
		pins[x].ChangeDutyCycle(rgb[x]/255*100)

gpio.setmode(gpio.BCM)

freq = 1000 # Hz

hex_rgb   = '#000000'
rgb_tuple = hex2rgb(hex_rgb)

pin_rgb = [20, 21, 6]
pin_pwm = []

for x in range(3):
	gpio.setup(pin_rgb[x], gpio.OUT)
	pin_pwm.append(gpio.PWM(pin_rgb[x], freq))
	pin_pwm[x].start(100)
	time.sleep(1)
	pin_pwm[x].ChangeDutyCycle(0)

for pin in pin_pwm:
	pin.ChangeDutyCycle(100)

time.sleep(1)

for pin in pin_pwm:
	pin.ChangeDutyCycle(0)


set_rgb(pin_pwm,list(hex2rgb('#ff0000')))
time.sleep(0.5)
set_rgb(pin_pwm,list(hex2rgb('#00ff00')))
time.sleep(0.5)
set_rgb(pin_pwm,list(hex2rgb('#0000ff')))
time.sleep(0.5)
set_rgb(pin_pwm,list(hex2rgb('#ffffff')))
time.sleep(0.5)
set_rgb(pin_pwm,list(hex2rgb('#000000')))

val = int(0xffffff)

while val>=0:
	set_rgb(pin_pwm,list(hex2rgb("#%x" % val)))
	val = val-255
	
gpio.cleanup()
