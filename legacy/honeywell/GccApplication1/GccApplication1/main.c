/*
 * GccApplication1.c
 *
 * Created: 26/10/2023 16:45:35
 * Author : agres
 */ 

#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 16000000UL

#include <avr/io.h>			// AVR Standard IO
#include <avr/interrupt.h>	// Manage interrupts
#include <util/delay.h>		//Delay					TODO: Substitute with Timer
#include <twimaster.c>		//RTC Clock				TODO: c file import works only?

//Defines Addresses for the ds1307 RTC TWI interface (See DS1307 data sheet for more information)
#define DS1307 0xD0					//0x68 bit shifted to left one time
#define DS1307Second 0x00			//Address for the seconds on the DS1307
#define DS1307Minute 0x01			//Address for the minutes on the DS1307
#define DS1307Hour 0x02				//Address for the Hours on the DS1307
#define DS1307Day 0x03				//Address for the Days on the DS1307
#define DS1307Date 0x04				//Address for the Date on the DS1307
#define DS1307Month 0x05			//Address for the Month on the DS1307
#define DS1307Year 0x06				//Address for the Year on the DS1307

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

//Define a pinout for controlling a transistor
#define pinOut PB0

/* A value passed to the transmit function for differentiating transmit modes in the Honeywell PM Sensor */
enum transmitFlag {
	READVALUE,
	STARTMEASUREMENT,
	STOPMEASUREMENT,
	STOPAUTOSEND,
	ENABLEAUTOSEND
};

/* Predeclared Command Values for the Honeywell PM Sensor (See Honeywell data sheet for more information) */
static uint8_t readCommand[4] = {0x68, 0x01, 0x04, 0x93};
static uint8_t startMeasurementCommand[4] = {0x68, 0x01, 0x01, 0x96};
static uint8_t stopMeasurementCommand[4] = {0x68, 0x01, 0x02, 0x95};
static uint8_t stopAutosend[4] = {0x68, 0x01, 0x20, 0x77};
static uint8_t enableAutosend[4] = {0x68, 0x01, 0x40, 0x57};
	
void initialisePinOut( void ) {
	DDRB |= (1 << DDB0);
}

void turnPiOn( void ) {
	PORTB |= (1 << PORTB0); // Set the pin to HIGH
}

void turnPiOff( void ) {
	PORTB &= ~(1 << PORTB0); // Set the pin to LOW
}

	
/* 
	Called at start of main in order to initialize the USART I/O.
	Values taken from ATMega Data sheet.
*/
void initUSART( void ) {
	/* Set baud rate */
	UBRR0H = (unsigned char) BAUD_PRESCALER >> 8;
	UBRR0L = (unsigned char) BAUD_PRESCALER;
	
	/* Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
	/* Set frame format: Asynchronous, No Parity Bit, 2stop Bit, 8 Data Bit */
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
}

/*
	Called at start of main in order to initialize the on board 8Bit Timer
*/
void initTimer( void ) {
	cli();		//Disable Global Interrupts

	//DDRA = 0xFF;

	// TIMER INITIALIZATION - setup timer and initialize them
	// timer1 - 16bit - initialize by setting the "Clear Timer on Compare (CTC)" mode
	// In CTC mode the counter is cleared to zero automatically when the counter value (TCNT1) matches the OCR1A register.
	//PRR0	|= (0 << PRTIM1); 	//make sure, that timer1 gets power - write a logical one means shut down timer module
	// TCCR1A-B = Timer/Counter Control Register A and B - clear register and set to normal mode
	TCCR1A	= 0;     		// set entire TCCR1A register to 0
	TCCR1B	= 0;     		// same for TCCR1B

	// target timer period is 1ms (0.0001s) - so the prescaler should be set to 64 - means each 64 clocks the timer counter increase by 1.
	// To match a 1ms period a maximum count of 250 should be set. Calc.: 16.000.000 / 64 => 250.000 / 250 => 1000 (clocks each second).
	// With the calculation it gets clear, why a 8bit timer is also working. His max count is 256 and the max count of timer1 is 65.536 
	OCR1A	= 250;		// OCR2A defines the top value for the counter - set compare match register to desired timer count
	// turn on CTC mode - see table 17-2 Page 145 in datasheet
	// If a period control is need the CTC mode is better as normal mode, because an automatically timer reload happens after each match
	// and the match value has to be set once. For special functions the timer logic level output could be toggle. See 17.9.2 (Table 17-3), Page 146, ds. 
	TCCR1B	|= (1 << WGM12); //set CTC mode - for OCR register (mode 4 in datasheet)
	TCCR1B	|= (1 << CS11) | (1 << CS10); // Set CS10 and CS11 bits for 64 prescaler (Table 17-6, p.157)
	// enable timer compare interrupt - because all timer interrupts can be individually masked with the "Timer Interrupt Mask Register" (TIMSKn)
	TIMSK1 |= (1 << OCIE1A); // If enabled (OCIEnx = 1 | n=1 for timer 1 | x=A for compare vector A), the Output Compare Flag generates an Output Compare interrupt. (17.7, p.141, ds)
 
	sei();          	// enable global interrupts
}

// Interrupt service routine 
// handles the match for timer1 - 16bit - Clear Timer on Compare (CTC) mode
// timer period = 1ms
ISR(TIMER1_COMPA_vect){ // (keep the code inside as less as possible - each inside jump stops the main routine)
	// if a match is detected the flag is set
	//flag_timer1 = 1; // set it on - in main() the var would be set off
	// Example 3: PORTA ^= (1 << PA1);			// toggling port pin
}

/* 
	Function which waits for the buffer to be emptied.
	Called between transmits and receives.
	TODO: Implement Timer witch breaks the function on timeout / if too much time passes.
 */
void USART_WaitUntilReady( void ) {
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
}

/* Function used for transmitting singular bytes of Data via the TX */
void USART_Transmit(uint8_t data) {
	/* Wait for data to be received */
	USART_WaitUntilReady();
	/* Write in Registry */
	UDR0 = data;
}
	
/* Function used for reading the value received and written in the registry */
unsigned char USARTReceive( void ) {
	/* Wait for data to be received */
	USART_WaitUntilReady();
	/* Get and return received data from buffer */
	return UDR0;
}

/* Function used for Transmitting entire command to the Honeywell PM Sensor */
void USART_TransmitPollingHoneywell(transmitFlag) {
	/* Wait for data to be received */
	USART_WaitUntilReady();
		
	/* Check witch mode should be transmitted and upload the corresponding command bytes with the checksum */
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
 */
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
 */
void USARTReceiveValues( void ) {
	uint8_t dataRecived[8];
	for(int i=0;i<8;i++){
		dataRecived[i] = USARTReceive();
	}
	
	//TODO Evaluate Data and do something
	
}

/*
	Function called to initialize the DS1307 RTC Module.
	A hexadecimal value for actual second, minute and hour should be passed and will be set as actual time on the RTC clock.
	If no i2c device can be found, a error clause will trigger.
*/
void DS1307Init (unsigned char second, unsigned char minute, unsigned char hour) {
	
	unsigned char check;
	
	check = i2c_start(DS1307+I2C_WRITE);
	if ( check ) {
		/* Failed to issue start condition, Error message here */
		i2c_stop();
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
		
		//... are days ... important?
	}
}

/*
	Function called to read the RTC Values and (rn) transfer this data via UART
*/
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

int main(void) {
	
	unsigned char ret;
	
	initUSART();
	_delay_ms(1000); //Temporary
	//initTimer();
	i2c_init();
	
	DS1307Init(0x00, 0x00, 0x00);
	initialisePinOut();
	
    while (1) {
		//DS1307ReadToUart();
		
		//USART_TransmitPollingHoneywell(STOPAUTOSEND);
		//_delay_ms(5000); //Temporary
		//USART_TransmitPollingHoneywell(STARTMEASUREMENT);
		//_delay_ms(5000); //Temporary
		//USART_TransmitPollingHoneywell(READVALUE);
		//_delay_ms(10000); //Temporary
		//USART_TransmitPollingHoneywell(STOPMEASUREMENT);
		
		turnPiOn();
		_delay_ms(2000);
		turnPiOff();
    }
    
    return 0;
}