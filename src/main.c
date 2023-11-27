/*
 * main.c
 *
 * Created: 26/10/2023 16:45:35
 * Author : agres
 */ 

#define __DELAY_BACKWARD_COMPATIBLE__
#define F_CPU 16000000UL

#include <avr/io.h>			                    // AVR Standard IO
#include <avr/interrupt.h>	                  // Manage interrupts
#include <util/delay.h>		//Delay					TODO: Substitute with Timer
#include "honeywell.c"

int main(void) {
	
	unsigned char ret;
	
	initUSART();
	_delay_ms(1000); //Temporary
	//initTimer();
	i2c_init();
	
	DS1307Init(0x00, 0x00, 0x00);
	
    while (1) {
		DS1307ReadToUart();
		
		//USART_TransmitPollingHoneywell(STOPAUTOSEND);
		//_delay_ms(5000); //Temporary
		//USART_TransmitPollingHoneywell(STARTMEASUREMENT);
		//_delay_ms(5000); //Temporary
		//USART_TransmitPollingHoneywell(READVALUE);
		//_delay_ms(10000); //Temporary
		//USART_TransmitPollingHoneywell(STOPMEASUREMENT);
		
		
    }
    
    return 0;
}