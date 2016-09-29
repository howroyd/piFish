##!/usr/bin/python3
import RPi.GPIO as gpio
#from smbus2 import SMBusWrapper
from smbus import SMBus
from datetime import datetime
import time
import os
import pithon_i2c_registry
import sys
from enum import Enum

address_nano = 0x69

register = pithon_i2c_registry.PithonI2cRegistry(8)

time_lights_on = 9
time_lights_off = 21

class reg(Enum):
    red = 0
    green = 1
    blue = 2
    fan = 3
    filter = 4
    lights = 5
    heater = 6
    air = 7

# Return CPU temperature as a character string                                      
def getCPUtemperature()->float:
    res = os.popen('vcgencmd measure_temp').readline()
    return(float(res.replace("temp=","").replace("'C\n","")))

bus = SMBus(1)

def update_register():
#    with SMBusWrapper(1) as bus:
        if register.changed:
            data = register.get()
            bus.write_i2c_block_data(address_nano, 0x00, [0xff] + data)
            register.changed = False
            print("Sent:", data) 
        time.sleep(0.1)
        data = bus.read_i2c_block_data(address_nano, 0, 8)
#        register.set(data)
        print("Received:", data) 

time_fan_on = 0
time_fan_off = time.time()

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

time.sleep(1)

# Blocker
while True:
    default_data = [0,0,0,255,1,0,1,1]
    register.set(default_data)
    update_register()
#    print("Initial register:", register.get())
    query = input("Continue? [y/n]  ")
    if query.lower() == "y":
        break
    elif query.lower() == "n":
        sys.exit()
    else:
        continue

while True:
    try:
        now = datetime.now()

        if now.hour >= time_lights_on and now.hour < time_lights_off:
            register.set(1, reg.lights.value)
        else:
            register.set(0, reg.lights.value)

        temp = getCPUtemperature()
        print(temp, "degC")

        if (getCPUtemperature() > 50.0 and register.get(reg.fan.value) != 255 and time.time()-time_fan_off > 10):
            time_fan_on = time.time()
            register.set(255, reg.fan.value)
        elif (getCPUtemperature() < 50.0 and register.get(reg.fan.value) == 255 and time.time()-time_fan_on > 10):
            time_fan_off = time.time()
            register.set(0, reg.fan.value)
        else:
            pass

        print(register.get())

        update_register()

        time.sleep(2)

    except IOError:
        pass
