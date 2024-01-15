/*
 * ds1307.c
 *
 * Created: 15/01/2024 17:00:00
 * Author : Dominik Agres
 */ 

#include "../lib/twimaster/twimaster.c"	

//DS1307 RTC Clock defines:
//Defines Addresses for the DS1307 RTC TWI interface (See DS1307 data sheet for more information)
#define DS1307 0xD0					//0x68 bit shifted to left one time
#define DS1307Second 0x00			//Address for the seconds on the DS1307
#define DS1307Minute 0x01			//Address for the minutes on the DS1307
#define DS1307Hour 0x02				//Address for the Hours on the DS1307
#define DS1307Day 0x03				//Address for the Days on the DS1307
#define DS1307Date 0x04				//Address for the Date on the DS1307
#define DS1307Month 0x05			//Address for the Month on the DS1307
#define DS1307Year 0x06				//Address for the Year on the DS1307
#define DS1307Control 0x07			//Address for the Control Register on the DS1307

/*
    This Array consists of three values. At index 0 the seconds are stored, at index 1 the minutes and at index 2 the hours.
*/
unsigned char timeData[3];          //Array to store the time data from the DS1307

/**
 	Function called to initialize the DS1307 RTC Module.
	A hexadecimal value for actual second, minute and hour should be passed and will be set as actual time on the RTC clock.
	If no i2c device can be found, a error clause will trigger.
    Function returns a 0 if initialization was successful and a 1 if not.
*/
int DS1307Init (unsigned char second, unsigned char minute, unsigned char hour) {
	
	unsigned char check;
	
	check = i2c_start(DS1307+I2C_WRITE);

	if ( check ) {
		i2c_stop();
		return 1;
	} else {
		i2c_stop();
		
		//Initializes the seconds to given value, IMPORTANT: the bit 7 (see data sheet) needs to be a 0 for the clock to run
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Second);
		i2c_write(second);
		i2c_stop();
	
		//Initializes the Minutes to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Minute);
		i2c_write(minute);
		i2c_stop();
	
		//Initializes the Hour to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Hour);
		i2c_write(hour);
		i2c_stop();

		//Set the DS1307 Control Register to output 1Hz on the SQW/OUT Pin
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Control);
		i2c_write(0x10);
		i2c_stop();

        return 0;
	}
}

/*
	Function called to read the RTC Values.
    Function returns a 0 if reading was successful and a 1 if not.
*/
int DS1307Read ( void ) {
	
	//Read Second value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Second);
	i2c_rep_start(DS1307+I2C_READ);
	timeData[0] = i2c_readNak();
	i2c_stop();
	
	//Read Minute value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Minute);
	i2c_rep_start(DS1307+I2C_READ);
	timeData[1] = i2c_readNak();
	i2c_stop();
	
	//Read Hour value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Hour);
	i2c_rep_start(DS1307+I2C_READ);
	timeData[2] = i2c_readNak();
	i2c_stop();

    return 0;
	
}