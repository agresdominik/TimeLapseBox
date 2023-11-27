/*
 * TemperarurBMP.c
 *
 * Created: 29.10.2023 16:33:38
 * Author : Nirmi
 */ 

#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define F_CPU 16000000UL
#define UART_BAUD_RATE 9600
#include <uart.h>
#include <i2cmaster.h>
#include <bmp280.h>

// this function can also be used as BMP280_DEBUG(name, val) in bmp280.c.
#define UART_BUFLEN 10
void uart_print(char *name, long val)
{
	char debug_buffer[UART_BUFLEN];

	uart_puts(name);
	uart_puts(" = ");

	ltoa((val), debug_buffer, UART_BUFLEN);
	uart_puts(debug_buffer);
	uart_puts("\n");
}

int main(void) {
	uart_init(UART_BAUD_SELECT(UART_BAUD_RATE, F_CPU));
//	uart_puts("start\n ");
	i2c_init(); //not needed - called from bmp280_init()
	bmp280_init();
	// enable IRQs
	sei();
	uart_print("status", bmp280_get_status());
	bmp280_measure();
	uart_print(" temperature x 100 ", bmp280_gettemperature());
	uart_print(" pressure x 100   ", bmp280_getpressure());
	uart_print(" altitude x 100   ", 100*bmp280_getaltitude());
	//uart_puts("\n");

	while(1) {
		/*
		uart_print("status", bmp280_get_status());
		bmp280_measure();
		uart_print("temperature x 100", bmp280_gettemperature());
		uart_print("pressure x 100   ", bmp280_getpressure());
		uart_print("altitude x 100   ", 100*bmp280_getaltitude());
		uart_puts("\n");
		_delay_ms(5000);
		*/
	}

	return 0;
}




