#ifndef DS1307_H
#define DS1307_H

#include "../lib/twimaster/twimaster.c"

#define DS1307 0xD0
#define DS1307Second 0x00
#define DS1307Minute 0x01
#define DS1307Hour 0x02
#define DS1307Day 0x03
#define DS1307Date 0x04
#define DS1307Month 0x05
#define DS1307Year 0x06
#define DS1307Control 0x07

extern unsigned char timeData[3];

int DS1307Init(unsigned char second, unsigned char minute, unsigned char hour);
int DS1307Read(void);

#endif