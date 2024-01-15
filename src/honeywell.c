/*
 * honeywell.c
 *
 * Created: 15/01/2024 17:00:00
 * Author : Dominik Agres
 */ 


#define F_CPU 16000000UL

#include <avr/io.h>			                    		// AVR Standard IO

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

/* A enum value passed to the transmit function for differentiating transmit modes in the Honeywell PM Sensor */
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
 */
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