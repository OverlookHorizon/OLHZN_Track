
double tempPressure[3];
double tempFromPressure, relativeAltitude, courseToLaunchSite, altitude;
double baselinePressure = -9999;
bool tempStarted = false;
char relativeAltitudeChar[12];
char relativeAltitudeFChar[12];

#ifdef LOG_PRESSURE

#include <SFE_BMP180.h>
SFE_BMP180 pressure;
const unsigned int reading_interval = 1000;
unsigned long last_reading = 0;

void SetupPressure(){
  if (!pressure.begin()){
    failure(503);  
  }
}

void CheckPressure(){
    if((getLogStarted() && millis()-last_reading >= reading_interval) || last_reading==0){
      if(!isTX()){
        getTempPressure();
        tempFromPressure = tempPressure[0]; //temperature from pressure sensor
        relativeAltitude = tempPressure[2];
        dtostrf(relativeAltitude,7,2,relativeAltitudeChar);
        dtostrf((relativeAltitude*3.28084),7,2,relativeAltitudeFChar);
        last_reading = millis();
      }
    }
}

void getTempPressure(){
  char status;
  double T,P,p0,a;
  ReadGPS();
  status = pressure.startTemperature();
  if (status != 0){
    delay(status);
    status = pressure.getTemperature(T);
    ReadGPS();
    tempPressure[0] = T;
    if (status != 0){
      status = pressure.startPressure(3);
      if (status != 0){
        delay(status);
        status = pressure.getPressure(P,T);        
        ReadGPS();
        if (status != 0){
          tempPressure[1] = P;
          GPS.Pressure = P;
          if(baselinePressure<=-9999){
            #ifdef DEBUG_SERIAL
              DEBUG_SERIAL.print(F("Set baseline pressure to "));
              DEBUG_SERIAL.println(P);
            #endif
            baselinePressure = P;
            tempStarted = true;
          }
          tempPressure[2] = pressure.altitude(P,baselinePressure);    
        }else if (!tempStarted) failure(504);
      }
      else if (!tempStarted) failure(505);
    }
    else if (!tempStarted) failure(506);
  }
  else if (!tempStarted) failure(507);
}

#endif

double getTempFromPressure(){
  return tempFromPressure;
}

double getRelativeAltitude(){
  return relativeAltitude;
}
double getRelativeAltitudeF(){
  return relativeAltitude*3.28084;
}

char* getRelativeAltitudeChar(){
  return relativeAltitudeChar;
}
char* getRelativeAltitudeFChar(){
  return relativeAltitudeFChar;
}

double cToF(double c){
  return ((c * 1.8) + 32);
}
