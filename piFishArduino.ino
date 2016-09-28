#include "arduino_i2c_registry.h"
#include <Wire.h>

#define VERSION  "1.1"
#define I2C_ADDR 0x69

ArduinoI2cRegistry reg = ArduinoI2cRegistry(8);

struct PiFish
{
	byte* rgb_red;
	byte* rgb_green;
	byte* rgb_blue;
	byte* fan;
	byte* relay_pump;
	byte* relay_lights;
	byte* relay_heater;
	byte* relay_air;
};

// Structure to contain which output is connected to which pin
struct Pins
{
	unsigned char rgb_red;
	unsigned char rgb_green;
	unsigned char rgb_blue;
	unsigned char fan;
	unsigned char relay_pump;
	unsigned char relay_lights;
	unsigned char relay_heater;
	unsigned char relay_air;
};

//PiFish piFish = { &reg._reg[0], &reg._reg[1], &reg._reg[2], &reg._reg[3], &reg._reg[4], &reg._reg[5], &reg._reg[6], &reg._reg[7] };
const PiFish* piFish = (PiFish*)reg.get();

const Pins pin = { A0, A1, A3, A6, 3, 4, 5, 6 };

// Onboard LED on Arduino Nano connected to D13
const char led_board = 13;

// Mains input relay (sprung, normally off) to entire system
//   prevents any mains output if there is no power to Arduino
const char relay_safety = 12;

// Small function to blink the onboard light at 1Hz
static const inline void blinky(const int);

void receive_event(int);
void request_event();

void setup()
{
	// Ensure mains input if off
	pinMode(relay_safety, OUTPUT);
	digitalWrite(relay_safety, HIGH);

	Wire.begin(I2C_ADDR);
	Wire.onReceive(receive_event);
	Wire.onRequest(request_event);

	Serial.begin(115200);
	Serial.println(sprintf("PiFish Version %s\nI2C address 0x%02X", VERSION, I2C_ADDR));

	// Setup output pin modes
	pinMode(pin.rgb_red, OUTPUT);
	pinMode(pin.rgb_green, OUTPUT);
	pinMode(pin.rgb_blue, OUTPUT);
	pinMode(pin.fan, OUTPUT);
	pinMode(pin.relay_pump, OUTPUT);
	pinMode(pin.relay_heater, OUTPUT);
	pinMode(pin.relay_air, OUTPUT);
	pinMode(pin.relay_lights, OUTPUT);

	pinMode(led_board, OUTPUT);
	pinMode(2, INPUT); // Always 5V due to PCB wiring, high impedence

					   // All ok, activate mains relay
	digitalWrite(relay_safety, LOW);
}

void loop()
{
	// Blink the onboard LED
	blinky(led_board);

  /* add main program code here */
  // Write PWM values
	analogWrite(pin.rgb_red, (int)*piFish->rgb_red);
	analogWrite(pin.rgb_green, (int)*piFish->rgb_green);
	analogWrite(pin.rgb_blue, (int)*piFish->rgb_blue);
	analogWrite(pin.fan, (int)*piFish->fan);

	// Write relay states
	digitalWrite(pin.relay_pump, (bool)*piFish->relay_pump);
	digitalWrite(pin.relay_heater, (bool)*piFish->relay_heater);
	digitalWrite(pin.relay_air, (bool)*piFish->relay_air);
	digitalWrite(pin.relay_lights, (bool)*piFish->relay_lights);

	// Print time & data to Serial
	Serial.print(millis());
	byte* ptr = (byte*)piFish;
	for (int x = 0; x < 9; x++) {
		Serial.print(',');
		Serial.print(*ptr++);
	}
	Serial.println();

  // Sleep for a bit to save a bit of energy
  //   Not sure if this actually helps!
	delay(1000);
}

void receive_event(int how_many) {
	byte reg_ptr = Wire.read();
	if (reg_ptr == 0) {
		byte* buf = (byte*)calloc(how_many, sizeof(byte));
		while (Wire.available()) {
			memset((void*)buf++, Wire.read(), sizeof(byte));
		}
		reg.set(buf, how_many);
	}
	else {
		byte buf;
		memset(&buf, Wire.read(), sizeof(byte));
		reg.set(buf, reg_ptr);
	}
}

void request_event() {
	Wire.write(reg.get(), reg.size());
}

// Toggle an LED
static const inline void blinky(const int pin) {
	digitalWrite(pin, !digitalRead(pin));
	return;
}