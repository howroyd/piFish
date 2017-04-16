import pifacedigitalio
from datetime import datetime

if __name__ == "__main__":
    pifacedigital = pifacedigitalio.PiFaceDigital()

    time_lights_on = 9
    time_lights_off = 21

    now = datetime.now()

    pifacedigital.leds[0].turn_off()
    pifacedigital.leds[2].turn_off()
    pifacedigital.leds[3].turn_off()

    if now.hour >= time_lights_on and now.hour < time_lights_off:
        pifacedigital.leds[1].turn_off()
    else:
        pifacedigital.leds[1].turn_on()

