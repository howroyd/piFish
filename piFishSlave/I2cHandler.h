// I2cHandler.h

#ifndef _I2CHANDLER_h
#define _I2CHANDLER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <TinyWireS.h>
#define I2C_SLAVE_ADDRESS 0x69
#define I2C_PACKET_SIZE 4

class I2cHandler {
public:
	I2cHandler(USI_TWI_S* stream);
	~I2cHandler(void);
	void init(void);
	// Incoming I2C data
	void receiveEvent(const uint8_t howMany);

	// I2C data request
	void requestEvent(void);

	volatile bool new_data;
	volatile uint8_t ptr;
	volatile uint8_t* _inbuf;
	uint8_t* _outbuf;
protected:
	USI_TWI_S* _stream;
private:
};

//extern I2cHandler i2c_handler(&TinyWireS);

#endif