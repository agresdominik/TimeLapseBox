/*
 * raspberryPi.c
 *
 * Created: 15/01/2024 17:00:00
 * Author : Dominik Agres
 */ 

#include "../lib/twimaster/twimaster.c"	

//RaspberryPi TWI Interface defines:
//Defines Addresses for the RaspberryPi TWI Interface
#define RaspberryPi 0x09
#define RaspberryPiWriteAddress 0x00
#define RaspberryPiReadAddress 0x01

void RaspberryPiWriteMessage ( unsigned char temperature, unsigned char luftdruck, unsigned char PM25,
		unsigned char PM10, unsigned char timestamp ) {
	
	i2c_start_wait(RaspberryPi+I2C_WRITE);
	i2c_write(RaspberryPiWriteAddress);
	i2c_write(temperature);
	i2c_stop();
		
    //TODO: Rest of the message
}

void RaspberryPiReadMessage ( void ) {
	//Placeholder
	unsigned char mesage;
	
	//Read Second value and save it
	i2c_start_wait(RaspberryPi+I2C_WRITE);
	i2c_write(RaspberryPiReadAddress);
	i2c_rep_start(RaspberryPi+I2C_READ);
	mesage = i2c_readAck();
	i2c_stop();
	
}