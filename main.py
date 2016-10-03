###!/usr/bin/python3
import RPi.GPIO as gpio
#from smbus2 import SMBusWrapper
from smbus import SMBus
from datetime import datetime
import time
import os
import pithon_i2c_registry
import sys
from enum import Enum
#import linuxi2c

address_nano = 0x69
pin_gpio_arduino_reset = 25

register = pithon_i2c_registry.PithonI2cRegistry(8)

time_lights_on = 9
time_lights_off = 21

class reg(Enum):
    red = 0
    green = 1
    blue = 2
    fan = 3
    filter = 4
    lights = 6
    heater = 5
    air = 7

# Return CPU temperature as a character string                                      
def getCPUtemperature():
    res = os.popen('vcgencmd measure_temp').readline()
    return(float(res.replace("temp=","").replace("'C\n","")))

bus = SMBus(1)

def update_register():
#    with SMBus(1) as bus:
        if register.changed:
            #dev = linuxi2c.IIC(0x69,1)
            data_out = register.get()
            bus.write_i2c_block_data(address_nano, 0xff, data_out)
            #dev.write([0xff]+data_out)
            print("Sent:", data_out) 
            time.sleep(0.1)
            data_in = bus.read_i2c_block_data(address_nano, 0, 8)[:8]
            #data_in = dev.read(8)
#        register.set(data)
            #dev.close()
            print("Received:", data_in) 
            if (data_out == data_in): 
                register.changed = False
                print("Ok!")
            else:
                print("Set register on Arduino failed!")

time_fan_on = 0
time_fan_off = time.time()

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

gpio.setup(pin_gpio_arduino_reset, gpio.OUT, initial=gpio.LOW)

time.sleep(1)

default_data = [50,50,50,255,1,1,1,1]
register.set(default_data)
update_register()

time_last = time.time()

# Blocker
#while True:
#    update_register()
#    print("Initial register:", register.get())
#    query = input("Continue? [y/n]  ")
#    if query.lower() == "y":
#        break
#    elif query.lower() == "n":
#        sys.exit()
#    else:
#        continue

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

        if (time.time() - time_last > 10):
            time_last = time.time()
            print(register.get())

        update_register()

        time.sleep(2)

    except IOError:
        pass

#    finally:
#        dev.close()
