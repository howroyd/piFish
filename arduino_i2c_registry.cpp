// 
// 
// 

#include "arduino_i2c_registry.h"

ArduinoI2cRegistry::ArduinoI2cRegistry(const int size) {
	_reg = (byte*)calloc(size, sizeof(byte));
	_size = size;
}

ArduinoI2cRegistry::~ArduinoI2cRegistry() {
	delete _reg;
}

byte ArduinoI2cRegistry::set(const byte val, const int reg) {
	memcpy((void*)_reg[reg], &val, sizeof(byte));
	return get(reg);
}

byte* ArduinoI2cRegistry::set(const byte* val, const int num_bytes) {
	memcpy((void*)_reg, val, num_bytes);
	return get();
}

byte ArduinoI2cRegistry::get(const int reg) {
	return _reg[reg];
}

byte* ArduinoI2cRegistry::get() {
	return _reg;
}

int ArduinoI2cRegistry::size() {
	return _size;
}