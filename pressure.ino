
double tempPressure[3];
float humidityRead, pressureRead;
double tempFromPressure, bmpAltitude;
double baselinePressure = -9999;
bool tempStarted = false;

#ifdef LOG_PRESSURE
  #ifdef PRESSURE_TYPE

  #define SEALEVELPRESSURE_HPA (1013.25)

  #if PRESSURE_TYPE == BME280
    #include <Wire.h>
    #include <Adafruit_Sensor.h>
    #include <Adafruit_BME280.h>
    Adafruit_BME280 pressure; // I2C
  #else
    #include <SFE_BMP180.h>
    SFE_BMP180 pressure;
  #endif
const unsigned int reading_interval = 1000;
unsigned long last_reading = 0;

void SetupPressure(){
  if (!pressure.begin()){
    failure(3);  
  }
}

void CheckPressure(){
    if((getLogStarted() && millis()-last_reading >= reading_interval) || last_reading==0){
      if(!isTX()){
        #if PRESSURE_TYPE == BME280
          getBMETempPressure();
        #else
          getBMPTempPressure();
          tempFromPressure = tempPressure[0]; //temperature from pressure sensor
          bmpAltitude = tempPressure[2];
          pressureRead = tempPressure[1];
          humidityRead = 0;
        #endif
        last_reading = millis();
      }
    }
}

#if PRESSURE_TYPE == BME280
  void getBMETempPressure(){
    tempFromPressure = pressure.readTemperature();
    pressureRead = (pressure.readPressure()/100.0F);
    humidityRead = pressure.readHumidity();
    bmpAltitude = pressure.readAltitude(SEALEVELPRESSURE_HPA);
    if(baselinePressure<=-9999){
      #ifdef DEBUG_SERIAL
        DEBUG_SERIAL.print(F("Set baseline pressure to "));
        DEBUG_SERIAL.println(pressureRead);
      #endif
      baselinePressure = pressureRead;
      tempStarted = true;
    }
  }
#else
  void getBMPTempPressure(){
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
          }else if (!tempStarted) failure(4);
        }
        else if (!tempStarted) failure(5);
      }
      else if (!tempStarted) failure(6);
    }
    else if (!tempStarted) failure(7);
  }
#endif

  #endif
#endif

double getTempFromPressure(){
  return tempFromPressure;
}

double getBMPAltitude(){
  return bmpAltitude;
}
double getBMPAltitudeF(){
  return mToF(bmpAltitude);
}

float getBaselinePressure(){
  return baselinePressure;
}

float getHumidityRead(){
  return humidityRead;
}

float getPressureRead(){
  return pressureRead;
}

float getBMEDewPoint(){
  #if PRESSURE_TYPE == BME280
    return dewPointFast(getTempFromPressure(),getHumidityRead());
  #else
    return -573.333;
  #endif
}

double cToF(double c){
  return ((c * 1.8) + 32);
}

double mToF(double m){
  return m*3.28084;
}

