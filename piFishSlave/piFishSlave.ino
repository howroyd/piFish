#include <Arduino.h>
#include <RBD_Timer.h>
#include <RBD_Light.h>

#define VERSION  "2.1"
#define I2C_SLAVE_ADDRESS 0x69
#define I2C_PACKET_SIZE 2

#include <TinyWireS.h>
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

#define LED_HEARTBEAT 10

class Pin {
public:
	Pin(const uint8_t pin, const bool value) {
		this->pin = pin;
		this->value = value;
	}
	bool value;
	uint8_t pin;
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
	int get_counter() {
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
};

void receiveEvent(const uint8_t);
void requestEvent(void);
void update(void);
Blinky blinky(LED_HEARTBEAT);

volatile byte    statusRegister = 0b11111111; // Toggle on/off a bit for each pin on/off
volatile bool    newData;
volatile uint8_t reg_ptr = 0;
volatile byte    i2c_in;

Pin red_p(8, true);
Pin green_p(7, true);
Pin blue_p(5, true);
Pin fan_p(9, true);
Pin pump_p(0, true);
Pin lights_p(1, true);
Pin heater_p(2, true);
Pin air_p(3, true);

RBD::Light red(red_p.pin);
RBD::Light green(green_p.pin);
RBD::Light blue(blue_p.pin);

// Blinky test(3, 3000);
// PB0 10
// PB1 9
// PB2 8
// PA7 7
// PA5 5
// PA3 3
// PA2 2
// PA1 1
// PA0 0


void setup() {

	//  test.init();
	//
	//  test.set_counter(255);
	//  while (true) test.update();

	blinky.init();
	pinMode(pump_p.pin, OUTPUT);
	pinMode(lights_p.pin, OUTPUT);
	pinMode(heater_p.pin, OUTPUT);
	pinMode(air_p.pin, OUTPUT);
	pinMode(red_p.pin, OUTPUT);
	pinMode(green_p.pin, OUTPUT);
	pinMode(blue_p.pin, OUTPUT);
	pinMode(fan_p.pin, OUTPUT);

	TinyWireS.begin(I2C_SLAVE_ADDRESS);
	TinyWireS.onReceive(receiveEvent);
	TinyWireS.onRequest(requestEvent);

	blinky.set_counter(3);
	while (blinky.get_counter() > 0) {
		blinky.update();
	}


}

void loop() {
	static unsigned long last_hs;
	static unsigned long last_ms;
	static unsigned long last_ls;
	unsigned long now = millis();

	// High speed loop //
	if ((now - last_hs) > 10) {
		last_hs = now;

		// Blink the onboard LED
		blinky.update();

		// Update RGB and FAN
		red.update();
		green.update();
		blue.update();
	}

	// Medium speed loop //
	if ((now - last_ms) > 100) {
		last_ms = now;
		if (newData) {
			update();
		}
	}

	// Low speed loop //
	if ((now - last_ls) > 4000) {
		last_ls = now;

		if (!red_p.value)   red.off();
		if (!green_p.value) green.off();
		if (!blue_p.value)  blue.off();
		digitalWrite(fan_p.pin, fan_p.value);
		digitalWrite(pump_p.pin, pump_p.value);
		digitalWrite(lights_p.pin, lights_p.value);
		digitalWrite(heater_p.pin, heater_p.value);
		digitalWrite(air_p.pin, air_p.value);

	}

	TinyWireS_stop_check();
}

void receiveEvent(const uint8_t howMany)
{
	// Sanity check
	if (howMany < 1 || howMany > TWI_RX_BUFFER_SIZE) return;

	reg_ptr = TinyWireS.receive();
	if (!TinyWireS.available()) return;
	//if (reg_ptr == 0xff) blinky.set_counter(4);
	//else blinky.set_counter(reg_ptr);
	i2c_in = TinyWireS.receive();

	while (TinyWireS.available()) TinyWireS.receive();

	newData = true;
}

void requestEvent(void)
{
	uint8_t data;
	switch (reg_ptr) {
	case 0:
		data = red.getBrightness();
		break;
	case 1:
		data = green.getBrightness();
		break;
	case 2:
		data = blue.getBrightness();
		break;
	case 0xFF:
		data = statusRegister;
		break;
	}
	TinyWireS.send(data);
}

void update(void) {
	switch (reg_ptr) {
	case 0:
		red.setBrightness(i2c_in);
		break;
	case 1:
		green.setBrightness(i2c_in);
		break;
	case 2:
		blue.setBrightness(i2c_in);
		break;
	case 0xFF:
		statusRegister = i2c_in;
		int x = 7;
		red_p.value = bitRead(statusRegister, x--);
		green_p.value = bitRead(statusRegister, x--);
		blue_p.value = bitRead(statusRegister, x--);
		fan_p.value = bitRead(statusRegister, x--);
		pump_p.value = bitRead(statusRegister, x--);
		lights_p.value = bitRead(statusRegister, x--);
		heater_p.value = bitRead(statusRegister, x--);
		air_p.value = bitRead(statusRegister, x--);
		break;
	}

	newData = false;
}