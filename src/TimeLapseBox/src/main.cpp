#include <Arduino.h>
#include <stdio.h>
#include <MQUnifiedsensor.h>                                          // Prog. Bibliothek für Luftqual.-Sensor

#define MQPIN A0
#define CLEAN_AIR_FACTOR 9.83
#define RL_VALUE 20                                                   // Lastwiderstand, Angabe in kOhm

float smoke_curve[3] = {1.350,1.050,0.800};                          // Berechn. Rauchkurve (Siehe Diagramm Datenblatt)
float ro = 0;                                                         // Widerstand des Sensors bei sauberer Luft

uint32_t delay_ms;

float mqCalibration();                                                // Kalibrierung Rauchsensor (ppm-Werte sind appoximiert)
float mqResistanceCalc(int);                                          // Berechnung der Widerstandswerte zum interpretieren der Einganssp.
int smokeLogScale(float, float *);                                    // Approximation des Rauchwertes
static void mqRead();                                                 // Funktion zum Auslesen der "Rauchmenge"

void setup() {
    Serial.begin(9600);
    delay(10);

    ro = mqCalibration();                                               // Kal. Werte zuweisen (Saubere Luft)
}

float mqCalibration(){                                                // Kalibrierung des Rauch Sensors
  Serial.print("\n");
  Serial.print("\n");
  Serial.println("******************");
  Serial.print(F("Calibrating Sensor"));
  float val = 0;
  for(int i = 0; i < 20; i++) {
    val += mqResistanceCalc(analogRead(MQPIN));                       // Analogwert einlesen und an R-Berechnung übergeben
    Serial.print(F("."));
    delay(500);
  }
  val = val / 20;
  val = val / CLEAN_AIR_FACTOR;
  Serial.print("\n");
  Serial.println("Sensor Calibration is done!");
  Serial.println("******************");
  return val;
}

float mqResistanceCalc(int adc){
  return(((float)RL_VALUE * (1023 / adc) / adc));                     // Berechnung und Rückgabe des Widerstandes
}

int smokeLogScale(float ratio, float* curve){                         // Berechnung des Rauchwertes in ppm
  return pow(10,(((log(ratio) - curve[1]) / curve[2]) + curve[0]));   // mit berechneten Werten der "Rauchkurve"
}

void mqRead(){
  int ppm = 0;
  char ppm_str[4];
  float rs = 0;
  for(int i = 0; i < 5; i++){ 
    rs += mqResistanceCalc(analogRead(MQPIN));                        // Sensorwert Messen und Berechnen
    delay(50);
  }
  rs = rs / 5;
  float rs_ro_ratio = rs / ro;
  ppm = smokeLogScale(rs_ro_ratio, smoke_curve);
  Serial.print(F("CO2: "));
  Serial.print(ppm);
  Serial.println(F("ppm"));             
  sprintf(ppm_str, "%d", ppm);
}

void loop() {
  Serial.print("\n");
  mqRead();
  delay(5000);
}
