/*
 * GccApplication1.c
 *
 * Created: 26/10/2023 16:45:35
 * Author : agres
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 16000000UL
#define USART_BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection

#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // USART Parity Bit Selection

#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // USART Stop Bit Selection

#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // USART Data Bit Selection

enum transmitFlag {
	READVALUE,
	STARTMEASUREMENT,
	ENDMEASUREMENT
};

//This Code Initializes the UCSR0B
void initUSART() {
	//Set Baud 0 before enabeling TX and RX
	UBRR0 = 0;
	// Set Frame Format
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	// Enable Receiver and Transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	// Set the baud rate  9600 bps at 16MHz CPU clock, IMPORTANT: Last after transmitter is enabled <-- Datasheet info
	UBRR0H = BAUD_PRESCALER >> 8;
	UBRR0L = BAUD_PRESCALER;
}

void USART_TransmitPolling(transmitFlag)
{
	while (( UCSR0A & (1<<UDRE0)) == 0) {}; // Do nothing until UDR is ready

	//Check Witch mode shoudl be transmitted and upload the corresponding bits
	if(transmitFlag == READVALUE){
		UDR0 = 0x68;
		UDR0 = 0x01;
		UDR0 = 0x04;
		UDR0 = ((65536-(0x68+0x01+0x04))%256);
	} else if (transmitFlag == STARTMEASUREMENT) {
		UDR0 = 0x68;
		UDR0 = 0x01;
		UDR0 = 0x01;
		UDR0 = ((65536-(0x68+0x01+0x01))%256);
	} else if (transmitFlag == ENDMEASUREMENT) {
		UDR0 = 0x68;
		UDR0 = 0x01;
		UDR0 = 0x02;
		UDR0 = ((65536-(0x68+0x01+0x02))%256); 
	}
}

//This Code Receives 
unsigned char USARTReceive() {
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0)));
	// Get and return received data
	return UDR0;
}


int main(void)
{
	initUSART();

	_delay_ms(1000); //Temporary

	USART_TransmitPolling(READVALUE);

	uint8_t data = USARTReceive(); //TODO mehr als uint8
	
    while (1) {
    }
    
    return 0;
}