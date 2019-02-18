#include "uStepperEncoder.h"

uStepperEncoder::uStepperEncoder(void)
{
	I2C.begin();
}

float uStepperEncoder::getAngleMoved(void)
{
	return (float)this->angleMoved*0.087890625;
}

float uStepperEncoder::getAngleMovedRaw(void)
{
	return (float)this->angleMovedRaw*0.087890625;
}

float uStepperEncoder::getSpeed(void)
{
	return this->curSpeed;
}

void uStepperEncoder::setup()
{
	TCNT1 = 0;
	ICR1 = 16000;
	TIFR1 = 0;
	TIMSK1 = (1 << OCIE1A);
	TCCR1A = (1 << WGM11);
	TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS10);
}

void uStepperEncoder::setHome(void)
{
	cli();
	uint8_t data[2];
	TIMSK1 &= ~(1 << OCIE1A);
	I2C.read(ENCODERADDR, ANGLE, 2, data);
	TIMSK1 |= (1 << OCIE1A);
	this->encoderOffset = (((uint16_t)data[0]) << 8 ) | (uint16_t)data[1];

	pointer->stepsSinceReset = 0;
	this->angle = 0;
	this->oldAngle = 0;
	this->angleMoved = 0;
	this->angleMovedRaw = 0;
}

float uStepperEncoder::getAngle()
{
	return (float)this->angle*0.087890625;
}

uint16_t uStepperEncoder::getStrength()
{
	uint8_t data[2];

	TIMSK1 &= ~(1 << OCIE1A);
	I2C.read(ENCODERADDR, MAGNITUDE, 2, data);
	TIMSK1 |= (1 << OCIE1A);

	return (((uint16_t)data[0]) << 8 )| (uint16_t)data[1];
}

uint8_t uStepperEncoder::getAgc()
{
	uint8_t data;
	TIMSK1 &= ~(1 << OCIE1A);
	I2C.read(ENCODERADDR, AGC, 1, &data);
	TIMSK1 |= (1 << OCIE1A);
	return data;
}

uint8_t uStepperEncoder::detectMagnet()
{
	uint8_t data;
	TIMSK1 &= ~(1 << OCIE1A);
	I2C.read(ENCODERADDR, STATUS, 1, &data);
	TIMSK1 |= (1 << OCIE1A);
	data &= 0x38;					//For some reason the encoder returns random values on reserved bits. Therefore we make sure reserved bits are cleared before checking the reply !

	if(data == 0x08)
	{
		return 1;					//magnet too strong
	}

	else if(data == 0x10)
	{
		return 2;					//magnet too weak
	}

	else if(data == 0x20)
	{
		return 0;					//magnet detected and within limits
	}

	return 3;						//Something went horribly wrong !
}
