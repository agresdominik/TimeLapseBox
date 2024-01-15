#ifndef HONEYWELL_H
#define HONEYWELL_H

#include <avr/io.h>

#define F_CPU 16000000UL
#define USART_BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#define ASYNCHRONOUS (0<<UMSEL00)
#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED
#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT
#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT

enum transmitFlag {
    READVALUE,
    STARTMEASUREMENT,
    STOPMEASUREMENT,
    STOPAUTOSEND,
    ENABLEAUTOSEND
};

extern uint8_t readCommand[4];
extern uint8_t startMeasurementCommand[4];
extern uint8_t stopMeasurementCommand[4];
extern uint8_t stopAutosend[4];
extern uint8_t enableAutosend[4];

void initUSART( void );
void USART_WaitUntilReady( void );
void USART_Transmit(uint8_t data);
unsigned char USARTReceive( void );
void USART_TransmitPollingHoneywell(transmitFlag);
void USARTReceiveStatus( void );
void USARTReceiveValues( void );

#endif