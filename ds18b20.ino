/* ========================================================================== */
/*   ds18b20.ino                                                              */
/*                                                                            */
/*   Code for reading OneWire Temperature devices/averaging ADC channels      */
/*                                                                            */
/* ========================================================================== */

// Variables

#define MAX_SENSORS  4
int DS18B20_Temperatures[MAX_SENSORS];

#ifdef WIREBUS

#include <OneWire.h>
#include <DallasTemperature.h>

// Variables

int SensorCount=0;       // Number of temperature devices found
unsigned long CheckDS18B20s=0;
int GettingTemperature=0;
OneWire oneWire(WIREBUS);                    // OneWire port
DallasTemperature sensors(&oneWire);   // Pass oneWire reference to Dallas Temperature object

void Setupds18b20(void){
  sensors.begin();
  // Grab a count of devices on the wire
  SensorCount = sensors.getDeviceCount();
  #ifdef DEBUG_SERIAL  
    DEBUG_SERIAL.print(SensorCount);
    DEBUG_SERIAL.println(F(" DS18B20's on bus"));
  #endif
  #ifdef EXPECTED_SENSORS
    if(SensorCount!=EXPECTED_SENSORS){
      //failure(1);
      //don't really need this failure, but you can uncomment to fail if the wrong number of temperature sensors is detected
    }
  #endif
  for(int i=0;i<MAX_SENSORS;i++){
    DS18B20_Temperatures[i] = 0;
  }
  SensorCount = min(SensorCount, MAX_SENSORS);
  if (SensorCount > 0)
  {
    sensors.setResolution(9);
  }
}

void Checkds18b20(void){
  if (millis() >= CheckDS18B20s && getLogStarted() && !isSendingRTTY() && !isTX()){
    if (GettingTemperature){      
      for (int i=0; i<SensorCount; i++){
        DS18B20_Temperatures[i] = sensors.getTempFByIndex(i);
        #ifdef DEBUG_SERIAL  
          DEBUG_SERIAL.print(F("Temperature ")); 
          DEBUG_SERIAL.print(i); 
          DEBUG_SERIAL.print(F(" = ")); 
          DEBUG_SERIAL.print(DS18B20_Temperatures[i]); 
          DEBUG_SERIAL.println(F("degF"));
        #endif
      }
      CheckDS18B20s = millis() + 10000L;
    }else{
      sensors.requestTemperatures();          // Send the command to get temperature
      CheckDS18B20s = millis() + 1000L;        // Leave 1 second (takes 782ms) for readings to happen
    }
    GettingTemperature = !GettingTemperature;
  }
}

#endif
