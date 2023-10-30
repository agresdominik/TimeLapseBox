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

#define USART_BAUDRATE 9600					// Desired Baud Rate
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

//This Code Initializes the UCSR0B
void initUSART() {
	
	//Set Baud 0 before enabeling TX and RX
	UBRR0 = 0;
	
	// Set Frame Format
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
	
	// Enable Receiver and Transmitter
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	
	// Set the baud rate (for example, 9600 bps at 16MHz CPU clock) IMPORTANT: Last after transmitter is enabled
	UBRR0H = BAUD_PRESCALER >> 8;
	UBRR0L = BAUD_PRESCALER;
}

void USART_TransmitPolling(uint8_t DataByte)
{
	while (( UCSR0A & (1<<UDRE0)) == 0) {}; // Do nothing until UDR is ready
	UDR0 = DataByte;
}

//This Code Receives on the UCSR0A Pin -> Pin0 (Rx)
unsigned char USARTReceive() {
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0)));
	// Get and return received data
	return UDR0;
}


int main(void)
{
	initUSART();
	
    while (1) {
		USART_TransmitPolling('A');
		USART_TransmitPolling('R');
		USART_TransmitPolling('N');
		USART_TransmitPolling('A');
		USART_TransmitPolling('B');
		USART_TransmitPolling('\n');
		_delay_ms(1000);
    }
    
    return 0;
}

