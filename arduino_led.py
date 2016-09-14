#!/usr/bin/python3

import smbus, time
import RPi.GPIO as gpio

def hex_to_rgb(value):
    value = value.lstrip('#')
    lv = len(value)
    return tuple(int(value[i:i + lv // 3], 16) for i in range(0, lv, lv // 3))

def rgb_to_hex(rgb):
    return '#%02x%02x%02x' % rgb

def set_rgb(rgb):
    try:
        bus = smbus.SMBus(1) 
        bus.write_i2c_block_data(DEVICE_ADDRESS, DEVICE_REGISTER, rgb)
    except IOError:
        print("IOError")
        time.sleep(1)
        set_rgb(rgb)
    

pins = (20, 21, 6)

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

for x in pins:
       	gpio.setup(x, gpio.IN)

DEVICE_ADDRESS = 0x69      #7 bit address (will be left shifted to add the read write bit)
DEVICE_REGISTER= 0x0

led_val = [(0, 0, 0), hex_to_rgb("#4885ed"), hex_to_rgb("#db3236"), hex_to_rgb("#f4c20d"), hex_to_rgb("#4885ed"), hex_to_rgb("#3cba54"), hex_to_rgb("#db3236")]

while True:
	for i in led_val:
		set_rgb(list(i))
		time.sleep(1)

