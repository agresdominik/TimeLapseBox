#ifndef RASPBERRYPI_H
#define RASPBERRYPI_H

#include "../lib/twimaster/twimaster.c"

#define RaspberryPi 0x09
#define RaspberryPiWriteAddress 0x00
#define RaspberryPiReadAddress 0x01

void RaspberryPiWriteMessage(unsigned char temperature, unsigned char luftdruck, unsigned char PM25,
                             unsigned char PM10, unsigned char timestamp);
void RaspberryPiReadMessage(void);

#endif