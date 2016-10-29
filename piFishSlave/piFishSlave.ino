#include <Arduino.h>
#include <RBD_Timer.h>
#include <RBD_Light.h>
#include <TinyWireS.h>
#include "TimedLoops.h"
#include "I2cHandler.h"

#define VERSION  "2.1"

// ATTiny84 pin definitions
#define PB0 10
#define PB1 9
#define PB2 8
#define PA7 7
#define PA5 5
#define PA3 3
#define PA2 2
#define PA1 1
#define PA0 0

class Pin {
public:
	Pin(const uint8_t pin, const bool value, const uint8_t bit) {
		this->pin = pin;
		this->value = value;
		this->bit = bit;
	}
	bool value;
	uint8_t pin;
	uint8_t bit;
};

class Blinky {
public:
	Blinky(const int pin, const unsigned long period) {
		_pin = pin;
		_period = period;
	}
	Blinky(const int pin) {
		_pin = pin;
		_period = 1000;
	}
	void init(void) {
		pinMode(_pin, OUTPUT);
		_time_last = millis();
	}
	void update(void) {
		unsigned long now = millis();
		if (_counter > 0) {
			if ((now - _time_last) > _period / 10) {
				this->blinky(_pin);
				_counter--;
				_time_last = now;
			}
		}
		else {
			return;
			if ((now - _time_last) > _period) {
				this->blinky(_pin);
				_time_last = now;
			}
		}
	}
	void set_counter(const int number) {
		_counter = number * 2;
	}
	const int get_counter() {
		return _counter % 2 == 0 ? _counter / 2 : _counter + 1 / 2;
	}
	// Toggle an LED
	static inline void blinky(const int pin) {
		digitalWrite(pin, !digitalRead(pin));
	}
private:
	int           _pin;
	unsigned long _time_last;
	unsigned long _period;
	volatile int  _counter;
} blinky(PB0);

I2cHandler i2c_handler(&TinyWireS);
TimedLoops fast_loop(10);
TimedLoops medium_loop(100);
TimedLoops slow_loop(4000);

void update(void);
void toggle_outputs(void);

byte statusRegister = 0b11111111; // Toggle on/off a bit for each pin on/off

Pin* pin_array[] = { new Pin(PA0, true, 3),
						new Pin(PA1, true, 2),
						new Pin(PA2, true, 1),
						new Pin(PA3, true, 0),
						new Pin(PB2, true, 7),
						new Pin(PA7, true, 6),
						new Pin(PA5, true, 5),
						new Pin(PB1, true, 4)};

RBD::Light* led_array[] = { new RBD::Light(pin_array[7]->pin),
								new RBD::Light(pin_array[6]->pin),
								new RBD::Light(pin_array[5]->pin)};

// MAIN SETUP //
void setup() {
	// Initialise the heartbead LED
	blinky.init();

	// Add functions to loop handler
	fast_loop.add_function((func_t)&blinky.update);
	for (uint8_t x=0 ; x < 3 ; x++) fast_loop.add_function((func_t)&led_array[x]->update);
	fast_loop.add_function((func_t)&TinyWireS_stop_check);
	medium_loop.add_function((func_t)&update);
	slow_loop.add_function((func_t)&toggle_outputs);

	// Setup output pins
	for (uint8_t x = 0; x < 8; x++) pinMode(pin_array[x]->pin, OUTPUT);

	// Start the I2C slave
	i2c_handler.init();

	// Flash the heartbeat 3 times to indicate setup is finished
	blinky.set_counter(3);
	while (blinky.get_counter() > 0) blinky.update();
	
	delay(2000);
}

// MAIN LOOP //
void loop() {
	slow_loop.run();
	medium_loop.run();
	fast_loop.run();
}

// Update internal registry
void update(void) {
	if (!i2c_handler.new_data) return;

	switch (i2c_handler.ptr) {
	case 0:
	case 1:
	case 2:
		led_array[i2c_handler.ptr]->setBrightness(i2c_handler._inbuf[0]);
		i2c_handler._outbuf[i2c_handler.ptr] = led_array[i2c_handler.ptr]->getBrightness();
		break;
	case 0xFF:
		statusRegister = i2c_handler._inbuf[0];
		i2c_handler._outbuf[i2c_handler.ptr] = statusRegister;
		break;
	}
	blinky.set_counter(3);
	i2c_handler.new_data = false;
}

void toggle_outputs(void) {
	for (uint8_t x = 0 ; x < 3 ; x++) if (!bitRead(statusRegister, pin_array[7 - x]->bit)) led_array[x]->off();
	for (uint8_t x = 4 ; x >= 0 ; x--) digitalWrite(pin_array[x]->pin, bitRead(statusRegister, pin_array[x]->bit));

	blinky.set_counter(1);
}