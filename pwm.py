import time 
import RPi.GPIO as GPIO

def hex_to_rgb(value):
    value = value.lstrip('#')
    lv = len(value)
    return tuple(int(value[i:i + lv // 3], 16) for i in range(0, lv, lv // 3))

hexval = '#ff0022'

pin_r = 6
pin_g = 21
pin_b = 20

freq = 60

GPIO.setmode(GPIO.BCM) 
GPIO.setup(pin_r, GPIO.OUT) 
GPIO.setup(pin_g, GPIO.OUT) 
GPIO.setup(pin_b, GPIO.OUT) 

p_r = GPIO.PWM(pin_r, freq) # channel=12 frequency=50Hz
p_g = GPIO.PWM(pin_g, freq) # channel=12 frequency=50Hz
p_b = GPIO.PWM(pin_b, freq) # channel=12 frequency=50Hz

p_r.start(0)
p_g.start(0)
p_b.start(0)

try:
    while 1:
        r,g,b = hex_to_rgb(hexval)
        p_r.ChangeDutyCycle(r/255*100)
        p_g.ChangeDutyCycle(g/255*100)
        p_b.ChangeDutyCycle(b/255*100)
        time.sleep(0.1)
except KeyboardInterrupt:
    pass 
p_r.stop()
p_g.stop()
p_b.stop()
GPIO.cleanup()
