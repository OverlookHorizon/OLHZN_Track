

float h = -999;
float c = -999;
float f = -999;
float hif = -999;
float hic = -999;
double dpc = -999;
double dpf = -999;
unsigned long checkDHT=0;

char hChar[7];
char cChar[7];
char fChar[7];
char hifChar[7];
char hicChar[7];
char dpcChar[7];
char dpfChar[7];

#ifdef DHTPIN
  #ifdef DHTTYPE

#include "DHT.h"

DHT dht(DHTPIN, DHTTYPE);

void setupDHT() {
  dht.begin();
}

void readDHT() {
  if (millis() >= checkDHT && getLogStarted() && !isSendingRTTY() && !isTX()){
    h = dht.readHumidity();
    c = dht.readTemperature();
    f = dht.readTemperature(true);
    dtostrf(h,3,2,hChar);
    dtostrf(c,3,2,cChar);
    dtostrf(f,3,2,fChar);
    if (isnan(h) || isnan(c) || isnan(f)) {
      h = -999;
      c = -999;
      f = -999;
      hif = -999;
      hic = -999;
      dpc = -999;
      dpf = -999;
    }else{
      hif = dht.computeHeatIndex(f, h);
      hic = dht.computeHeatIndex(c, h, false);
      dpc = dewPointFast(c,h);
      dpf = cToF(dpc);
      dtostrf(hif,3,2,hifChar);
      dtostrf(hic,3,2,hicChar);
      dtostrf(dpf,3,2,dpfChar);
      dtostrf(dpc,3,2,dpcChar);
    }
    #if DHTTYPE == DHT11
      checkDHT = millis() + 1000L;        // readings only once every 1 second
    #else
      checkDHT = millis() + 2000L;        // readings only once every 2 seconds
    #endif
  }
}

#endif
#endif


// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double dewPoint(double celsius, double humidity){
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

        // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558 - T);
}

// delta max = 0.6544 wrt dewPoint()
// 6.9 x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity){
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}

float readHTempC(){
  return c;
}
float readHTempF(){
  return f;
}
float readHumid(){
  return h;
}
float readHIF(){
  return hif;
}
float readHIC(){
  return hic;
}
double readDPC(){
  return dpc;
}
double readDPF(){
  return dpf;
}

char* readHTempCChar(){
  return cChar;
}
char* readHTempFChar(){
  return fChar;
}
char* readHumidChar(){
  return hChar;
}
char* readHIFChar(){
  return hifChar;
}
char* readHICChar(){
  return hicChar;
}
char* readDPCChar(){
  return dpcChar;
}
char* readDPFChar(){
  return dpfChar;
}

