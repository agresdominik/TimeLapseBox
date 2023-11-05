/*
 * GccApplication1.c
 *
 * Created: 26/10/2023 16:45:35
 * Author : agres
 */ 

#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 16000000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

// USART Baud rate and Prescaler/Divider
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

/* A value passed to the transmit function for differentiating transmit modes */
enum transmitFlag {
	READVALUE,
	STARTMEASUREMENT,
	STOPMEASUREMENT,
	STOPAUTOSEND,
	ENABLEAUTOSEND
};

/* Predeclared Command Values for the Honeywell PM Sensor */
static uint8_t readCommand[4] = {0x68, 0x01, 0x04, 0x93};
static uint8_t startMeasurementCommand[4] = {0x68, 0x01, 0x01, 0x96};
static uint8_t stopMeasurementCommand[4] = {0x68, 0x01, 0x02, 0x95};
static uint8_t stopAutosend[4] = {0x68, 0x01, 0x20, 0x77};
static uint8_t enableAutosend[4] = {0x68, 0x01, 0x40, 0x57};

	
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
	
	//TODO: Check ACK if Positive or Negative
	
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

int main(void) {
	initUSART();
	_delay_ms(1000); //Temporary
	
    while (1) {
		USART_TransmitPollingHoneywell(STOPAUTOSEND);
		_delay_ms(5000); //Temporary
		USART_TransmitPollingHoneywell(STARTMEASUREMENT);
		_delay_ms(5000); //Temporary
		USART_TransmitPollingHoneywell(READVALUE);
		_delay_ms(10000); //Temporary
		USART_TransmitPollingHoneywell(STOPMEASUREMENT);
    }
    
    return 0;
}