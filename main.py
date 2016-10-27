#!/usr/bin/python3

import RPi.GPIO as gpio
import smbus
from datetime import datetime
import time
import os
import pithon_i2c_registry
import sys
from enum import Enum
from w1thermsensor import W1ThermSensor
import socket
import crcmod

address_nano = 0x69
pin_gpio_arduino_reset = 20
pin_gpio_arduino_interrupt = 21
pin_gpio_switch_lid = 16

time_lights_on = 9
time_lights_off = 21
time_leds_on = 21
time_leds_off = 23

gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

class pin_arduino(Enum):
    red = 0
    green = 1
    blue = 2
    fan = 3
    status = 4

flag_lid_open = False

class FanController(object):
    def __init__(self):
        self.time_on = 0
        self.time_off = time.time()
        self.state = False
        self.pending = False
    def get_state(self):
        return self.state
    def set_state(self, state):
        if state:
            self.time_on = time.time()
            self.state = True
            self.pending = False
        elif ((time.time() - self.time_on) > 10): # 10s overrun
            self.time_off = time.time()
            self.state = False
            self.pending = False
        else:
            self.pending = True
    def update(self):
        if self.pending:
            self.set_state(False) 

class Heartbeat(object):
    def __init__(self, sweep=False):
        self._time_last = time.time()
        self._val = 0
        self._sweep = sweep
        self._sweep_dir = True

    def get(self):
        return self._val

    def set_sweep(self, state):
        self._sweep = state

    def update(self):
        if self._sweep:
            if (self._val >= 240):
                self._sweep_dir = False
            elif (self._val <= 10):
                self._sweep_dir = True
            if self._sweep_dir:
                self._val -= 10
            else:
                self._val += 10
        else:
            now = time.time()
            if ((now - self._time_last) > 1000):
                self._time_last = now
                self._val != self._val
        
# Return CPU temperature as a character string                                      
def getCPUtemperature():
    res = os.popen('vcgencmd measure_temp').readline()
    return(float(res.replace("temp=","").replace("'C\n","")))

def update_register(reg):
#    with SMBus(1) as bus:
        if reg.changed:
            crc = crcmod.Crc(0x11021, rev=False, initCrc=0x0000, xorOut=0x0000)
            data_out = reg.get()
            for x in range(reg.size):
                crc.update(bytearray((reg.get(x-1))))
            data_out.append(crc.crcValue)

            bus.write_i2c_block_data(address_nano, data_out[0], data_out[1:])

            time.sleep(0.1)
            print("Sent:", data_out) 
            time.sleep(0.1)

            data_in = [None] * reg.size
            for x in range(9):
#            data_in = bus.read_i2c_block_data(address_nano, 0, 9)[:9]
                data_in[x] = bus.read_byte_data(address_nano, x)
                time.sleep(0.1)
            #data_in = dev.read(8)
#        register.set(data)
            #dev.close()
            print("Received:", data_in) 
            if (data_out == data_in): 
                register.changed = False
                print("Ok!")
            else:
                print("Set register on Arduino failed!")

# Lid switch interrupt callbacks
def lid_open(channel):  
    if gpio.input(pin_gpio_switch_lid):
        flag_lid_open = True
    else:
        flag_lid_open = False


gpio.setup(pin_gpio_arduino_reset, gpio.OUT, initial=gpio.HIGH) # Pulse LOW for > 700ns to reset arduino
gpio.setup(pin_gpio_arduino_interrupt, gpio.OUT, initial=gpio.HIGH) # Heartbeat watchdog
gpio.setup(pin_gpio_switch_lid, gpio.IN, pull_up_down=gpio.PUD_UP) # Lid switch
gpio.add_event_detect(pin_gpio_switch_lid, gpio.BOTH, callback=lid_open, bouncetime=300)

#time.sleep(1) #TODO needed?

bus = smbus.SMBus(1)
temp_sensor = W1ThermSensor()
fan_controller = FanController()
heartbeat = Heartbeat()

## OWL Intuitiuon
UDP_IP = "127.0.0.1"
UDP_PORT = 9000
UDP_KEY = "385D23B0"
#sock = socket.socket(socket.AF_INET, # Internet
#                     socket.SOCK_DGRAM) # UDP
# Set some options to make it multicast-friendly
#sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#try:
#    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
#except AttributeError:
#    pass # Some systems don't support SO_REUSEPORT
#sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_TTL, 20)
#sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_LOOP, 1)
# Bind to the port
#sock.bind(('', UDP_PORT))
# Set some more multicast options
#intf = socket.gethostbyname(socket.gethostname())
#sock.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_IF, socket.inet_aton(intf))
#sock.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(UDP_IP) + socket.inet_aton(intf))

try:
# Fish Tank Control
    default_data = [ 128, 128, 128, 255, 0b11111111 ]
    register = pithon_i2c_registry.PithonI2cRegistry(5)
    register.set(default_data)
    update_register()
    print("ATTiny found!")
except IOError:
    print("No ATTiny found!")


## Main loop ##
while True:
    try:
        now = datetime.now()
        temp = getCPUtemperature()
#        temp_tank = temp_sensor.get_temperature()

        # Heartbeat
        heartbeat.update()
        gpio.output(pin_gpio_arduino_interrupt, heartbeat.get())

        # Disable all bar dim red led if lid is open
        if flag_lid_open:
            register.set(0b10000000, pin_arduino.status.value)
            register.set(100, pin_arduino.red.value)
        else:
            register.set(0b11111111, pin_arduino.status.value)
            # Set main lights
            if now.hour >= time_lights_on and now.hour < time_lights_off:
                register.set(register.get(pin_arduino.status.value)|0b00000100, pin_arduino.status.value)
            else:
                register.set(register.get(pin_arduino.status.value)&0b11111011, pin_arduino.status.value)

            # Set LEDs
            if now.hour >= time_leds_on and now.hour < time_leds_off:
                register.set(255, pin_arduino.red.value)
                register.set(255, pin_arduino.green.value)
                register.set(255, pin_arduino.blue.value)
            else:
                register.set(0, pin_arduino.red.value)
                register.set(0, pin_arduino.green.value)
                register.set(0, pin_arduino.blue.value)
    
            # Set cooling fan with debounce
            if (getCPUtemperature() > 50.0):
                fan_controller.set_state(True)
            else:
                fan_controller.set_state(False)
            fan_controller.update()
            register.set(fan_controller.get_state(), pin_arduino.fan.value)

        update_register(register)

        #time.sleep(2)

    except IOError:
        pass

#        sock.setsockopt(socket.SOL_IP, socket.IP_DROP_MEMBERSHIP, socket.inet_aton(UDP_IP) + socket.inet_aton('0.0.0.0'))
#        sock.close()
    except KeyboardInterrupt:
        gpio.cleanup()           # clean up GPIO on normal exit  
#sock.setsockopt(socket.SOL_IP, socket.IP_DROP_MEMBERSHIP, socket.inet_aton(UDP_IP) + socket.inet_aton('0.0.0.0'))
#sock.close()
