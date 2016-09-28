// arduino_i2c_registry.h

#ifndef _ARDUINO_I2C_REGISTRY_h
#define _ARDUINO_I2C_REGISTRY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class ArduinoI2cRegistry {
public:
	ArduinoI2cRegistry(const int);
	~ArduinoI2cRegistry();
	byte set(const byte, const int);
	byte* set(const byte*, const int);
	byte get(const int);
	byte* get();
	int size();
	byte* _reg;
	int _size;
};

#endif

