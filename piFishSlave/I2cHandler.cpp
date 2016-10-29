// 
// 
// 

#include "I2cHandler.h"


I2cHandler::I2cHandler(USI_TWI_S* stream) {
	_stream = stream;
	_inbuf = (uint8_t*)malloc(sizeof(uint8_t));
	_outbuf = (uint8_t*)calloc(I2C_PACKET_SIZE, sizeof(uint8_t));
}

I2cHandler::~I2cHandler(void) {
	free(_inbuf);
	free(_outbuf);
}

void I2cHandler::init(void) {
	_stream->begin(I2C_SLAVE_ADDRESS);
	_stream->onReceive((void*)&I2cHandler::receiveEvent);
	_stream->onRequest((void*)&I2cHandler::requestEvent);
}

// Incoming I2C data
void I2cHandler::receiveEvent(const uint8_t howMany)
{
	// Sanity check
	if (howMany < 1 || howMany > I2C_PACKET_SIZE) return;

	ptr = _stream->receive();

	if (!_stream->available()) return;

	_inbuf[0] = _stream->receive();

	while (_stream->available()) _stream->receive();

	new_data = true;
}

// I2C data request
void I2cHandler::requestEvent(void) {
	_stream->send(_outbuf[ptr]);
}