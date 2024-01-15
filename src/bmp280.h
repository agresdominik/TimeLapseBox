#ifndef B280_H
#define B280_H

#include "../lib/bmp280/Src/bmp280.c"

extern unsigned char sorroundingData[3];

void initB280(void);
void mesaureTemperatureAndPressure(void);

#endif