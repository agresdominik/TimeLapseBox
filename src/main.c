/*
 * main.c
 *
 * Created: 26/10/2023 16:45:35
 * Author : agres
 */ 

#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 16000000UL

#include <avr/io.h>			                    		// AVR Standard IO
#include <avr/interrupt.h>	                  			// Manage interrupts

#include <util/delay.h>									//Delay	TODO: Substitute with Timer

#include <stdlib.h>										//standard c lib
#include <string.h>										//standard c lib for manipulating strings

#include "ds1307.h"										//header file for ds1307
#include "bmp280.h"										//header file for bmp280
#include "raspberryPi.h"								//header file for raspberryPi
#include "honeywell.h"									//header file for honeywell PM Sensor

/*
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

//RaspberryPi TWI Interface defines:
//Defines Addresses for the RaspberryPi TWI Interface
#define RaspberryPi 0x09
#define RaspberryPiWriteAddress 0x00
#define RaspberryPiReadAddress 0x01 */

//Transistor defines for RaspberryPi:
//Defines the Pin used to controll the transistor.
#define TRANSISTOR_PIN PORTB0 

// USART Timeout Value
#define TIMEOUT_VALUE 5

/*
// USART defines:
// USART Baud rate and Prescaler/Divider (See AtMega328P data sheet for more information)
#define USART_BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

// USART Mode Selection
#define ASYNCHRONOUS (0<<UMSEL00)
// USART Parity Bit Selection
#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED
// USART Stop Bit Selection
#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT
// USART Data Bit Selection
#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT

/* A enum value passed to the transmit function for differentiating transmit modes in the Honeywell PM Sensor 
enum transmitFlag {
	READVALUE,
	STARTMEASUREMENT,
	STOPMEASUREMENT,
	STOPAUTOSEND,
	ENABLEAUTOSEND
};

/* Predeclared Command Values for the Honeywell PM Sensor (See Honeywell data sheet for more information) 
static uint8_t readCommand[4] = {0x68, 0x01, 0x04, 0x93};
static uint8_t startMeasurementCommand[4] = {0x68, 0x01, 0x01, 0x96};
static uint8_t stopMeasurementCommand[4] = {0x68, 0x01, 0x02, 0x95};
static uint8_t stopAutosend[4] = {0x68, 0x01, 0x20, 0x77};
static uint8_t enableAutosend[4] = {0x68, 0x01, 0x40, 0x57};
*/

/* These Values are used by the timer to check if timeout has happened and to end ever waiting funtions*/
volatile uint8_t timeoutFlag = 0;
volatile uint16_t counter = 0;
volatile uint16_t externalClockCounter = 0;

// Overflow service routine 
// timer period = 1.04 Sec
ISR(TIMER1_OVF_vect){
	// Increment the counter value
    counter++;

    // Check if the counter has reached the timeout value
    if (counter >= TIMEOUT_VALUE) {
        // Set the timeout flag
        timeoutFlag = 1;

        // Reset the counter
        counter = 0;
    }
}

// ISR for external interrupt by DS1307 RTC Clock
ISR(INT0_vect) {
    // Increment the counter every second
    externalClockCounter++;

    // If one hour has passed (3600 seconds)
    if (externalClockCounter >= 3600) {
        // Reset the counter
        externalClockCounter = 0;

        // Code To Execute Every Hour:

    }
}

/* 
	Called at setup in order to initialize the USART I/O.
	Values taken from ATMega322p datasheet.

void initUSART( void ) {
	// Set baud rate
	UBRR0H = (unsigned char) BAUD_PRESCALER >> 8;
	UBRR0L = (unsigned char) BAUD_PRESCALER;
	
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
	// Set frame format: Asynchronous, No Parity Bit, 2stop Bit, 8 Data Bit
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
} 
	Function which waits for the buffer to be emptied.
	Called between transmits and receives when using USART Interface.
	A Timer is started when the function is called and is stopped when the buffer is empty.
	If this timer overflows, the funtion will break out of the wait loop and handle the error.
 
void USART_WaitUntilReady( void ) {
	//Set timeout flag to 0
	timeoutFlag = 0;

	// Start the timer
    TCNT1 = 0; // Reset timer count
    TCCR1B |= (1 << CS12); // Start timer with prescaler = 256

	// Wait for empty transmit buffer
	while ( !( UCSR0A & (1<<UDRE0)) ) {
		// Check if the timeout flag has been set, this is set once the timer overflows for the 5th time (5 Seconds)
		if (timeoutFlag == 1) {
			// Break out of the loop
			break;
			//TODO: Error Handling
		}
	}
	// Stop the timer
    TCCR1B &= ~(1 << CS12);
}

/* Function used for transmitting singular bytes of Data via the TX 
void USART_Transmit(uint8_t data) {
	/* Wait for data to be received 
	USART_WaitUntilReady();
	/* Write in Registry 
	UDR0 = data;
}
	
/* Function used for reading the value received and written in the registry 
unsigned char USARTReceive( void ) {
	/* Wait for data to be received 
	USART_WaitUntilReady();
	/* Get and return received data from buffer 
	return UDR0;
}

/* Function used for Transmitting entire command to the Honeywell PM Sensor 
void USART_TransmitPollingHoneywell(transmitFlag) {
	/* Wait for data to be received 
	USART_WaitUntilReady();
		
	/* Check witch mode should be transmitted and upload the corresponding command bytes with the checksum 
	if(transmitFlag == READVALUE){
		for(int i = 0; i < 4; i++) {
			UDR0 = (uint8_t) readCommand[i];
			USART_WaitUntilReady();
		}
		
		//TODO: Here Call the USARTReceiveValue() Function and process data -> next Step
		
	} else if (transmitFlag == STARTMEASUREMENT) {
		for(int i = 0; i < 4; i++) {
			UDR0 = (uint8_t) startMeasurementCommand[i];
			USART_WaitUntilReady();
		}
		
		//TODO: Here Call the USARTReceiveStatus() Function and check if a Pos. ACK was sent
	
	} else if (transmitFlag == STOPMEASUREMENT) {
		for(int i = 0; i < 4; i++) {
			UDR0 = (uint8_t) stopMeasurementCommand[i];
			USART_WaitUntilReady();
		}
		
		//TODO: Here Call the USARTReceiveStatus() Function and check if a Pos. ACK was sent
		
	} else if (transmitFlag == STOPAUTOSEND) {
		for(int i = 0; i < 4; i++) {
			UDR0 = (uint8_t) stopAutosend[i];
			USART_WaitUntilReady();
		}
		
		//TODO: Here Call the USARTReceiveStatus() Function and check if a Pos. ACK was sent
		
	} else if (transmitFlag == ENABLEAUTOSEND) {
		for(int i = 0; i < 4; i++) {
			UDR0 = (uint8_t) enableAutosend[i];
			USART_WaitUntilReady();
		}
		
		//TODO: Here Call the USARTReceiveStatus() Function and check if a Pos. ACK was sent
		
	}
}

/* 
	Function witch saves the 2 Byte Positive or Negative ACK in a variable and checks if it is valid.
	Called Upon when the Honeywell sensor gets a command witch changes its behavior.
 
void USARTReceiveStatus( void ) {
	uint8_t dataRecived[2];
	for(int i=0;i<2;i++){
		dataRecived[i] = USARTReceive();
	}

	if (dataRecived[1] == 0xA5 && dataRecived[2] == 0xA5) {
		//Positive ACK
	} else if (dataRecived[1] == 0x96 && dataRecived[2] == 0x96){
		//Negative ACK
	} else {
		//Unknown Error, e.g. Cable Loose, interference...
	}
	
}

/* 
	Function witch saves the 2 Byte Positive or Negative ACK in a variable and checks if it is valid.
	Called Upon when the Honeywell sensor gets a command witch changes its behavior.
 
void USARTReceiveValues( void ) {
	uint8_t dataRecived[8];
	for(int i=0;i<8;i++){
		dataRecived[i] = USARTReceive();
	}
	
	//TODO Evaluate Data and do something
	
}

/* Function Exported to a different c file.
	Function called to initialize the DS1307 RTC Module.
	A hexadecimal value for actual second, minute and hour should be passed and will be set as actual time on the RTC clock.
	If no i2c device can be found, a error clause will trigger.
void DS1307Init (unsigned char second, unsigned char minute, unsigned char hour) {
	
	unsigned char check;
	
	check = i2c_start(DS1307+I2C_WRITE);
	if ( check ) {
		i2c_stop();
		// Failed to issue start condition, Error message here
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
	}
}

/*
	Function called to read the RTC Values and (rn) transfer this data via UART

void DS1307ReadToUart ( void ) {
	//Placeholders
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	
	//Read Second value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Second);
	i2c_rep_start(DS1307+I2C_READ);
	second = i2c_readNak();
	i2c_stop();
	
	//Read Minute value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Minute);
	i2c_rep_start(DS1307+I2C_READ);
	minute = i2c_readNak();
	i2c_stop();
	
	//Read Hour value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Hour);
	i2c_rep_start(DS1307+I2C_READ);
	hour = i2c_readNak();
	i2c_stop();
	
	//Transmit the saved values via a UART interface
	USART_Transmit(second);
	USART_Transmit(minute);
	USART_Transmit(hour);
	_delay_ms(1000);
} 

void RaspberryPiWriteMessage ( unsigned char temperature, unsigned char luftdruck, unsigned char PM25,
		unsigned char PM10, unsigned char timestamp ) {
	
	i2c_start_wait(RaspberryPi+I2C_WRITE);
	i2c_write(RaspberryPiWriteAddress);
	i2c_write(temperature);
	i2c_stop();
		
	USART_Transmit(temperature);
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

	// check if message is ok or nack
	USART_Transmit(mesage);
	
}

/*
	Function witch initializes the BMP280 Temperature Sensor.

void initB280() {
	bmp280_init();
}

/*
	Function witch measures the temperature and pressure and saves them in variables.

void mesaureTemperatureAndPressure() {
	unsigned char temperature;
	unsigned char pressure;
	unsigned char altitude;
	
	bmp280_get_status();
	bmp280_measure();
	temperature = bmp280_gettemperature();
	pressure = bmp280_getpressure();
	altitude = 100*bmp280_getaltitude();
} */

/*
	Function witch turns on the transistor digital pinout.
*/
void turnOnTransistor() {
	PORTB |= (1 << TRANSISTOR_PIN);
}

/*
	Function witch turns off the transistor digital pinout.
*/
void turnOffTransistor() {
	PORTB &= ~(1 << TRANSISTOR_PIN);
}

/*
	This function is used to put the device to sleep, and to wake it up once the device has been interrupted by the clock.
	The Time is checked each time, if the time is appropriate then further functions are called.
*/
void enterSleep() {
    // Sets appropriate sleep mode.
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Enables the sleep mode.
    sleep_enable();

    // Go to sleep.
    sleep_cpu();

    // Wake up here after interrupt.
    // Disable sleep mode.
    sleep_disable();

	//TODO: Check time.
}

/* 
	This Function configures the ATMega328P to trigger an interrupt on the rising edge of the INT0(PD2) pin.
	It Also enables global interrupts for the DS1307 RTC Clock to wake itself up and for the onboard timer.
	Also multiple init functions are called.
*/
void setup() {

    // Configure INT0 (pin PD2) to trigger an interrupt on the rising edge.
    EICRA |= (1 << ISC01) | (1 << ISC00);
    // Enable INT0.
    EIMSK |= (1 << INT0);

	// Set the transistor pin as output.
    DDRB |= (1 << TRANSISTOR_PIN);

    // Enable global interrupts.
    sei();

	// Initialise the different devices.
	initUSART();
	i2c_init();
	DS1307Init(0x00, 0x00, 0x00);
}

/*
	In main the setup() method is called once and then a while loop starts witch sets the device into sleep mode.
	In the sleep method the time is checked so that the device can enter the recording and transmiting mode.
*/
int main(void) {
	setup();

    while (1) {
		i2c_init();
    }
    
    return 0;
}