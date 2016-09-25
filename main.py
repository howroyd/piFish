##!/usr/bin/python3
import RPi.GPIO as gpio
import smbus
import datetime
import time

water_change = False

bus = smbus.SMBus(1)

address_nano = 0x69

on = 9
off = 21

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

bus.write_byte_data(address_nano, 1, 255)  # Fan
bus.write_byte_data(address_nano, 2, 255)  # Fan
bus.write_byte_data(address_nano, 3, 255)  # Fan
bus.write_byte_data(address_nano, 4, 255)  # Fan
#bus.write_byte_data(address_nano, 5, True) # Filter
#bus.write_byte_data(address_nano, 6, True) # Lights
#bus.write_byte_data(address_nano, 7, True) # Heater
#bus.write_byte_data(address_nano, 8, True) # Air

while True:
	try:
		now = datetime.datetime.now()

		if now.hour >= on and now.hour < off:
			bus.write_byte_data(address_nano, 6, True) # Lights
		else:
			bus.write_byte_data(address_nano, 6, False) # Lights

		if water_change is True:
			bus.write_byte_data(address_nano, 5, False) # Filter
			bus.write_byte_data(address_nano, 7, False) # Heater
		else:
			bus.write_byte_data(address_nano, 5, True) # Filter
			bus.write_byte_data(address_nano, 7, True) # Heater
		bus.write_byte_data(address_nano, 8, True) # Air

		time.sleep(1)

		data = bus.read_i2c_block_data(address_nano, 0)

#		for i in range(9):
#			print(str(data[i]), end=',')
#		print()

#		time.sleep(1)

	except OSError:
		pass
