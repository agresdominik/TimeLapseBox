/*
 * bmp280.c
 *
 * Created: 15/01/2024 17:00:00
 * Author : Dominik, Nirmi
 */ 

#include "../lib/bmp280/Src/bmp280.c"					//library for bmp280 temperature sensor

/*
    This Array contains the sorrounding temperature, pressure and altitude data.
*/
unsigned char sorroundingData[3];                       //Array to store the sorrounding temperature, pressure and altitude data from the BMP280


/*
	Function witch initializes the BMP280 Temperature Sensor.
*/
void initB280() {
	bmp280_init();
}

/*
	Function witch measures the temperature and pressure and saves them in variables.
*/
void mesaureTemperatureAndPressure() {
	bmp280_get_status();
	bmp280_measure();
	sorroundingData[0] = bmp280_gettemperature();
	sorroundingData[1] = bmp280_getpressure();
	sorroundingData[2] = 100*bmp280_getaltitude();
}