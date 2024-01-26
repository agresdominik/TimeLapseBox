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
#include <avr/sleep.h>									// Manage sleep mode

#include <util/delay.h>									//Delay	TODO: Substitute with Timer

#include <stdlib.h>										//standard c lib
#include <string.h>										//standard c lib for manipulating strings

#include "../lib/twimaster/twimaster.c"	
#include "../lib/bmp280/Src/bmp280.c"					//library for bmp280 temperature sensor

//#include "ds1307.h"										//header file for ds1307
//#include "bmp280.h"										//header file for bmp280
//#include "raspberryPi.h"								//header file for raspberryPi
//#include "honeywell.h"									//header file for honeywell PM Sensor


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
#define RaspberryPiWriteAddress 0x09
#define RaspberryPiReadAddress 0x01

//Transistor defines for RaspberryPi:
//Defines the Pin used to controll the transistor.
#define TRANSISTOR_PIN PORTB0

// USART Timeout Value
#define TIMEOUT_VALUE 5

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

/*
	Enum used to differenciate sucess from failure when trsnmitting data to server
*/
enum transmitMessage {
	SUCCESSFULL,
	FAILED
};

//DEPRICATED:
/* A enum value passed to the transmit function for differentiating transmit modes in the Honeywell PM Sensor */
enum transmitFlag {
	READVALUE,
	STARTMEASUREMENT,
	STOPMEASUREMENT,
	STOPAUTOSEND,
	ENABLEAUTOSEND
};

//DEPRICATED:
/* Predeclared Command Values for the Honeywell PM Sensor (See Honeywell data sheet for more information) */
static uint8_t readCommand[4] = {0x68, 0x01, 0x04, 0x93};
static uint8_t startMeasurementCommand[4] = {0x68, 0x01, 0x01, 0x96};
static uint8_t stopMeasurementCommand[4] = {0x68, 0x01, 0x02, 0x95};
static uint8_t stopAutosend[4] = {0x68, 0x01, 0x20, 0x77};
static uint8_t enableAutosend[4] = {0x68, 0x01, 0x40, 0x57}; 

/* These Values are used by the timer to check if timeout has happened and to end ever waiting funtions */
volatile uint8_t timeoutFlag = 0;
volatile uint16_t counter = 0;
volatile uint16_t externalClockCounter = 0;

/* These Values are used by the bmt to write data into and send later to the pi */
volatile uint32_t temperature = 0x00;
volatile uint32_t pressure = 0x00;
volatile uint32_t altitude = 0x00;

//Placeholders
volatile uint8_t second = 0x00;
volatile uint8_t minute = 0x00;
volatile uint8_t hour = 0x00;
volatile uint8_t date = 0x00;
volatile uint8_t month = 0x00;
volatile uint8_t year = 0x00;


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

	int result = 0;

    // If one hour has passed (3600 seconds)
    if (externalClockCounter >= 3600) {
        // Reset the counter
        externalClockCounter = 0;

        // Code To Execute Every Hour:
		result = mainExecuteFunction();

		if (result == 0) {
			transmitMessage(SUCCESSFULL);
		} else if (result == 1) { 
			transmitMessage(FAILED);
		}
		
    }
}

/* 
	Called at setup in order to initialize the USART I/O.
	Values taken from ATMega322p datasheet.
*/
void initUSART( void ) {
	// Set baud rate
	UBRR0H = (unsigned char) BAUD_PRESCALER >> 8;
	UBRR0L = (unsigned char) BAUD_PRESCALER;
	
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
	// Set frame format: Asynchronous, No Parity Bit, 2stop Bit, 8 Data Bit
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
} 

/*
	Function which waits for the buffer to be emptied.
	Called between transmits and receives when using USART Interface.
	A Timer is started when the function is called and is stopped when the buffer is empty.
	If this timer overflows, the funtion will break out of the wait loop and handle the error.
	Returns a 1 or 0 value for failure or success.
*/ 
uint8_t USART_WaitUntilReady( void ) {
	//Set timeout flag to 0
	timeoutFlag = 0;

	// Start the timer
    TCNT1 = 0; // Reset timer count
    TCCR1B |= (1 << CS12); // Start timer with prescaler = 256

	// Wait for empty transmit buffer
	while ( !( UCSR0A & (1<<UDRE0)) ) {
		// Check if the timeout flag has been set, this is set once the timer overflows for the 5th time (5 Seconds)
		if (timeoutFlag == 1) {
			return 1;
		}
	}
	// Stop the timer
    TCCR1B &= ~(1 << CS12);

	return 0;
}

/* Function used for transmitting singular bytes of Data via the TX  */
void USART_Transmit(uint8_t data) {
	// Wait for data to be received 
	USART_WaitUntilReady();
	// Write in Registry 
	UDR0 = data;
}
	
/* Function used for reading the value received and written in the registry */
unsigned char USARTReceive( void ) {
	// Wait for data to be received 
	USART_WaitUntilReady();
	// Get and return received data from buffer 
	return UDR0;
}

//DEPRICATED:
/* Function used for Transmitting entire command to the Honeywell PM Sensor */
void USART_TransmitPollingHoneywell(transmitFlag) {
	// Wait for data to be received 
	USART_WaitUntilReady();
		
	// Check witch mode should be transmitted and upload the corresponding command bytes with the checksum 
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
	Called Upon when the Raspberry Pi is working to get command if evrything was sucessful.
*/ 
uint8_t USARTReceiveStatus( void ) {
	//Predifined ACK Values
	volatile uint8_t yesMessage = (uint8_t) 0xA5;
	volatile uint8_t noMessage = (uint8_t) 0x96;
	volatile uint8_t emptyMessage = (uint8_t) 0xFE;

	//Array for reading data
	uint8_t dataRecived[2];

	//Read the datat from UART registry
	for(int i=0;i<2;i++){
		dataRecived[i] = USARTReceive();
	}

	//Check Data and evaluate
	if (dataRecived[1] == 0xA5 && dataRecived[2] == 0xA5) {
		dataRecived[1] = 0x00;
		dataRecived[2] = 0x00;
		return 0;
	} else if (dataRecived[1] == 0x96 && dataRecived[2] == 0x96){
		dataRecived[1] = 0x00;
		dataRecived[2] = 0x00;
		return 1;
	} else {
		//Do this while waiting for information
		USART_Transmit(emptyMessage);
		return 2;
	}
	
}

/* 
	Function called to initialize the DS1307 RTC Module.
	A hexadecimal value for actual second, minute and hour should be passed and will be set as actual time on the RTC clock.
	If no i2c device can be found, a error clause will trigger.
*/
void DS1307Init (unsigned char setSecond, unsigned char setMinute, unsigned char setHour, unsigned char setDate, unsigned char setMonth, unsigned char setYear) {
	unsigned char check;
	
	check = i2c_start(DS1307+I2C_WRITE);
	if ( check ) {
		i2c_stop();
		// ERROR HANDLING: TODO: Send nack message
	} else {
		i2c_stop();
		
		//Initializes the seconds to given value, IMPORTANT: the bit 7 (see data sheet) needs to be a 0 for the clock to run
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Second);
		i2c_write(setSecond);
		i2c_stop();
	
		//Initializes the Minutes to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Minute);
		i2c_write(setMinute);
		i2c_stop();
	
		//Initializes the Hour to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Hour);
		i2c_write(setHour);
		i2c_stop();

		//Initializes the Date to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Date);
		i2c_write(setDate);
		i2c_stop();

		//Initializes the Month to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Month);
		i2c_write(setMonth);
		i2c_stop();

		//Initializes the Year to given value
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Year);
		i2c_write(setYear);
		i2c_stop();

		//Set the DS1307 Control Register to output 1Hz on the SQW/OUT Pin
		i2c_start_wait(DS1307+I2C_WRITE);
		i2c_write(DS1307Control);
		i2c_write(0x10); // See datasheed, here its 0 for RS1 0 for RS0 and 1 for SQWE e.g. 00010000 registry
		i2c_stop();
	}
}

/*
	Function called to read the RTC Values and save them in variables.
*/
void DS1307Read ( void ) {
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

	//Read Date value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Date);
	i2c_rep_start(DS1307+I2C_READ);
	date = i2c_readNak();
	i2c_stop();

	//Read Month value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Month);
	i2c_rep_start(DS1307+I2C_READ);
	month = i2c_readNak();
	i2c_stop();

	//Read Year value and save it
	i2c_start_wait(DS1307+I2C_WRITE);
	i2c_write(DS1307Year);
	i2c_rep_start(DS1307+I2C_READ);
	year = i2c_readNak();
	i2c_stop();

} 

/*
	This function is called evry Hour for it to send all the senor data connected to it to the raspberry pi via UART interface.
*/
void RaspberryPiWriteMessage ( ) {
	//Split Tempreature value from a uint32 to 4 uint8 values
	volatile uint8_t startMessageValue = (uint8_t) 0xFF;
	volatile uint8_t temperature_1 = (uint8_t) (temperature & 0xFF);
	volatile uint8_t temperature_2 = (uint8_t) ((temperature >> 8) & 0xFF);
	volatile uint8_t temperature_3 = (uint8_t) ((temperature >> 16) & 0xFF);
	volatile uint8_t temperature_4 = (uint8_t) ((temperature >> 24) & 0xFF);

	//Split Pressure value from a uint32 to 4 uint8 values
	volatile uint8_t pressure_1 = (uint8_t) (pressure & 0xFF);
	volatile uint8_t pressure_2 = (uint8_t) ((pressure >> 8) & 0xFF);
	volatile uint8_t pressure_3 = (uint8_t) ((pressure >> 16) & 0xFF);
	volatile uint8_t pressure_4 = (uint8_t) ((pressure >> 24) & 0xFF);

	//Split Altitude value from a uint32 to 4 uint8 values
	volatile uint8_t altitude_1 = (uint8_t) (altitude & 0xFF);
	volatile uint8_t altitude_2 = (uint8_t) ((altitude >> 8) & 0xFF);
	volatile uint8_t altitude_3 = (uint8_t) ((altitude >> 16) & 0xFF);
	volatile uint8_t altitude_4 = (uint8_t) ((altitude >> 24) & 0xFF);

	//Transmit all the data saved on the microcontroller
	UDR0 = (uint8_t) startMessageValue;		//Start Of Message 0xFF
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) second;				//Second of day
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) minute;				//Minute of day
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) hour;					//Hour of day
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) date;					//Date of day
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) month;					//Month of day
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) year;					//Year of day
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) temperature_1;			//Temperature split in 4 4byte values
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) temperature_2;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) temperature_3;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) temperature_4;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) pressure_1;			//Pressure split in 4 4byte values
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) pressure_2;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) pressure_3;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) pressure_4;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) altitude_1;			//Altitude split in 4 4byte values
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) altitude_2;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) altitude_3;
	USART_WaitUntilReady();
	_delay_ms(500);
	UDR0 = (uint8_t) altitude_4;
	USART_WaitUntilReady();
	_delay_ms(500);
}

/*
	Function witch initializes the BMP280 Temperature Sensor.
*/
void initB280() {
	bmp280_init();
}

/*
	Function witch measures the temperature and pressure and saves them in variables.
*/
void mesaureTemperatureAndPressure() {
	bmp280_get_status();
	bmp280_measure();
	temperature = bmp280_gettemperature();
	pressure = bmp280_getpressure();
	altitude = 100*bmp280_getaltitude();
}

/*
	Function witch turns on the portb0 digital pinout for sensors.
*/
void turnOnTransistorPin() {
	PORTB |= (1 << TRANSISTOR_PIN);
}

/*
	Function witch turns off the portb0 digital pinout for sensors.
*/
void turnOffTransistorPin() {
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

	// Initialise the different devices and interfaces.
	initUSART();
	i2c_init();
	initB280();
	DS1307Init(0x00, 0x00, 0x0C, 0x16, 0x01, 0x18); //<-- Call this method only once to setup correct time and enable sqw output
}

/*
	This function is called evry hour and is used to execute the main functions of the device.
*/
uint8_t mainExecuteFunction() {
	//Start teh devices connected to the pb0 pin
	turnOnTransistorPin();
	_delay_ms(1000);
	//Read the time from the RTC Clock
	DS1307Read();
	_delay_ms(100);
	if (hour >= 0x13 && hour <= 0x08) {
		return 0;
	}
	//Measure the temperature and pressure
	mesaureTemperatureAndPressure();
	_delay_ms(100);
	//Send the data to the raspberry pi
	while (true) {
		if (USARTReceiveStatus() == 0) {
			break;
		} else if (USARTReceiveStatus() == 1) {
			return 1;
		} else if (USARTReceiveStatus() == 2) {
			RaspberryPiWriteMessage();
		}
	}
	//As soon as the data is sent and recived, go into busy waiting for 3 minutes so that the pi can take enough time to process the data and save it.
	_delay_ms(180000);
	// Wait for pi to give a final ACK if the entire process was sucessfull
	while (true) {
		if (USARTReceiveStatus() == 0) {
			turnOffTransistorPin();
			break;
		} else if (USARTReceiveStatus() == 1) {
			return 1;
		} else if (USARTReceiveStatus() == 2) {}
	}

}

void callServer(transmitMessage message) {
	if (message == SUCCESSFULL) {
	} else if (message == FAILED) {
	}
	
	//See other file/AT Commands for code
}

/*
	In main the setup() method is called once and then a while loop starts witch sets the device into sleep mode.
	In the sleep method the time is checked so that the device can enter the recording and transmiting mode.
*/
int main(void) {
	setup();
    while (1) {
		enterSleep(); //<-- this is called in the final version of the programm only main the main while function
    }
    
    return 0;
}