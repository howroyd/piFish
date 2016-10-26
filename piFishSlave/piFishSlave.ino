#include <Arduino.h>
#include <Crc16.h>
#include <RBD_Timer.h>
#include <RBD_Light.h>

#define VERSION  "2.1"
#define I2C_SLAVE_ADDRESS 0x69
#define I2C_PACKET_SIZE 6

#include <TinyWireS.h>
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( I2C_PACKET_SIZE )
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
			if ((now - _time_last) > _period) {
				this->blinky(_pin);
				_time_last = now;
			}
		}
	}
	void set_counter(const int number) {
		_counter = number;
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
volatile byte    i2cReceiveRegister[I2C_PACKET_SIZE];
volatile bool    newData;
volatile uint8_t reg_ptr = 0;

Crc16 crc;

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
RBD::Light fan(fan_p.pin);


void setup() {
	blinky.init();

	pinMode(pump_p.pin, OUTPUT);
	pinMode(lights_p.pin, OUTPUT);
	pinMode(heater_p.pin, OUTPUT);
	pinMode(air_p.pin, OUTPUT);

	TinyWireS.begin(I2C_SLAVE_ADDRESS);
	TinyWireS.onReceive(receiveEvent);
	TinyWireS.onRequest(requestEvent);

	for (int x = 0; x < 3; x++) {
		digitalWrite(lights_p.pin, false);
		delay(500);
		digitalWrite(lights_p.pin, true);
		delay(500);
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
		fan.update();
	}

	// Medium speed loop //
	if ((now - last_ms) > 100) {
		last_ms = now;
		if (newData) {
			update();
			blinky.set_counter(1);
		}
	}

	// Low speed loop //
	if ((now - last_ls) > 1000) {
		last_ls = now;

		if (!red_p.value)   red.off();
		if (!green_p.value) green.off();
		if (!blue_p.value)  blue.off();
		if (!fan_p.value)   fan.off();
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

	if (howMany == 1) {
		reg_ptr = TinyWireS.receive();
		return;
	}

	crc.clearCrc();
	for (int a = 1; a <= howMany; a++) {
		if (a <= I2C_PACKET_SIZE) {
			i2cReceiveRegister[a - 1] = TinyWireS.receive();
			if (a <= (I2C_PACKET_SIZE - 1)) crc.updateCrc(i2cReceiveRegister[a - 1]);
		}
		else TinyWireS.receive();  // if we receive more data than allowed just throw it away
	}

	if (crc.getCrc() == (unsigned short)i2cReceiveRegister[I2C_PACKET_SIZE - 1]) newData = true;
	else blinky.set_counter(3);
}

void requestEvent(void)
{
	uint8_t data;
	switch (reg_ptr) {
	case 7:
		data = red.getBrightness();
		break;
	case 6:
		data = green.getBrightness();
		break;
	case 5:
		data = blue.getBrightness();
		break;
	case 4:
		data = fan.getBrightness();
		break;
	case 3:
		data = fan_p.value;
		break;
	case 2:
		data = lights_p.value;
		break;
	case 1:
		data = heater_p.value;
		break;
	case 0:
		data = air_p.value;
		break;
	default:
		data = statusRegister;
		break;
	}
	TinyWireS.send(data);
}

void update(void) {
	uint8_t x = 0;
	red.setBrightness(i2cReceiveRegister[x++]);
	green.setBrightness(i2cReceiveRegister[x++]);
	blue.setBrightness(i2cReceiveRegister[x++]);
	fan.setBrightness(i2cReceiveRegister[x++]);

	uint8_t status = i2cReceiveRegister[I2C_PACKET_SIZE - 2];
	x = 7;
	red_p.value = bitRead(status, x--);
	green_p.value = bitRead(status, x--);
	blue_p.value = bitRead(status, x--);
	fan_p.value = bitRead(status, x--);
	pump_p.value = bitRead(status, x--);
	lights_p.value = bitRead(status, x--);
	heater_p.value = bitRead(status, x--);
	air_p.value = bitRead(status, x--);

	newData = false;
}