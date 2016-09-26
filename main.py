##!/usr/bin/python3
import RPi.GPIO as gpio
import smbus
import datetime
import time
import os
from enum import Enum

bus = smbus.SMBus(1)

address_nano = 0x69

time_lights_on = 9
time_lights_off = 21

class reg(Enum):
    red = 1
    green = 2
    blue = 3
    fan = 4
    filter = 5
    lights = 6
    heater = 7
    air = 8

# Return CPU temperature as a character string                                      
def getCPUtemperature():
    res = os.popen('vcgencmd measure_temp').readline()
    return(float(res.replace("temp=","").replace("'C\n","")))

time_fan_on = time.time()

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

#bus.write_byte_data(address_nano, 1, 255)  # Red
#bus.write_byte_data(address_nano, 2, 255)  # Green
#bus.write_byte_data(address_nano, 3, 255)  # Blue
#bus.write_byte_data(address_nano, 4, 255)  # Fan
#bus.write_byte_data(address_nano, 5, True) # Filter
#bus.write_byte_data(address_nano, 6, False) # Lights
#bus.write_byte_data(address_nano, 7, True) # Heater
#bus.write_byte_data(address_nano, 8, True) # Air

#time.sleep(5)

#bus.write_byte_data(address_nano, reg['red'].value, 255)  # Red
#bus.write_byte_data(address_nano, reg['green'].value, 255)  # Green
#bus.write_byte_data(address_nano, reg['blue'].value, 255)  # Blue
#bus.write_byte_data(address_nano, reg['fan'].value, 255)  # Fan
#bus.write_byte_data(address_nano, reg['filter'].value, True) # Filter
#bus.write_byte_data(address_nano, reg['lights'].value, True) # Lights
#bus.write_byte_data(address_nano, reg['heater'].value, False) # Heater
#bus.write_byte_data(address_nano, reg['air'].value, True) # Air

test = [10,255,255,255,1,0,1,1]

while True:
	try:
		now = datetime.datetime.now()

		data = bus.read_i2c_block_data(address_nano, 0)[0:9]

		if now.hour >= time_lights_on and now.hour < time_lights_off:
			test[5] = True
			#bus.write_byte_data(address_nano, 6, True) # Lights
		else:
			test[5] = False
			#bus.write_byte_data(address_nano, 6, False) # Lights

#		bus.write_byte_data(address_nano, 5, True) # Filter
#		bus.write_byte_data(address_nano, 7, True) # Heater
#		bus.write_byte_data(address_nano, 8, True) # Air

		time.sleep(1)
		if data != test:
			print("New data")
			for i in test:
				bus.write_byte(address_nano,i)


		temp = getCPUtemperature()
		print(temp, "degC")
#		if (getCPUtemperature() > 50.0):
#			time_fan_on = time.time()
#			bus.write_byte_data(address_nano, reg['fan'].value, 255)  # Fan
#		elif (time.time() > 30):
#			bus.write_byte_data(address_nano, reg['fan'].value, 0)  # Fan

		print(data)
			
	except IOError:
		pass
