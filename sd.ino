
bool log_started = false;


bool getLogStarted(){
  return log_started;
}

#ifdef LOG_DATA

#include <SD.h>
char filename[20];
File myFile;
File altFile;
float msrate, ftrate, mphrate;
char alt_filename[15];
char boot_filename[15];
char sdLine[800];
char elapsedTime[12];
unsigned long last_sd_write = 0;
boolean anomaly_on = true;
boolean needs_header = true;

void SetupSD(){  
  pinMode(SS, OUTPUT);   //keep the hardware SS pin (53 on Mega) set to output or the SD library won't work
  
  SdFile::dateTimeCallback(dateTime); 
  if (!SD.begin(10, 11, 12, 13)) {
    failure(9);
  }
  snprintf(filename,12,"log.csv");

////  create unique file on every reboot  
//  unsigned int ctr = 0;
//  while(SD.exists(filename)){
//    ctr++;
//    snprintf(filename,12,"log%u.csv",ctr);
//    stepLights();
//    #ifdef USE_WATCHDOG
//      wdt_reset();
//    #endif
//  }
//  allLEDoff();

  snprintf(boot_filename,15,"reboots.txt");

  if(SD.exists(filename)){
    needs_header = false;
  }
  #ifdef DEBUG_SERIAL
    DEBUG_SERIAL.print(F("Debug to: "));
    DEBUG_SERIAL.println(filename);
  #endif
  myFile = SD.open(filename, FILE_WRITE);  
  if(needs_header){
    if (myFile) {
      myFile.println(F("Elapsed,Onboard Timestamp,Unix Timestamp,Satellites,GPS Timestamp,GPS Date Age,HDOP,Latitude,Longitude,GPS Age,Launch Latitude,Launch Longitude,Distance to Launch Site (mi),Course to Launch Site,Cardinal to Launch Site,Altitude (m),Altitude (ft),Course,Cardinal,Speed (kmph),Speed (mph),Temp Sensor 1,Temp Sensor 2,BME Temp Sensor (F),BME Humidity %,BME Dew Point (F),BMP Altitude (m),BMP Altitude (ft),Baseline Pressure (mb),Pressure (mb),Humidity (%),Humidity Temp (c),Humidity Temp (f),Heat Index (f),Heat Index (c),Dew Point (c),Dew Point (f),Free RAM,Battery Voltage,Camera Voltage 1,Camera Voltage 2,Burst Cam,Hit Altitude,Buzzer On,Anomaly,Ascent Rate (m/s),Ascent Rate (ft/s),Ascent Rate (mph),Last Transmission,Next TX (s)"));
      myFile.close();
    }else{
      failure(510);
    }
  }
  pinMode(ANOMALY_ALARM_PIN,OUTPUT);  
  snprintf(alt_filename,15,"altitude.txt");
}


void CheckSD(){
  //construct log string & write to sd card
  
  if(log_started || (GPS.Lock==1 && GPS.Satellites>=3)){
    if(millis() - last_sd_write >= SD_WRITE_TIME && !isTX()){ 
      if(!log_started){
        log_started = true;        
        myFile = SD.open(boot_filename, FILE_WRITE);
        myFile.print(F("REBOOT "));
        myFile.print(RTCO.timestamp);
        myFile.print(F(" "));
        myFile.println(GPS.Timestamp);
        myFile.close();  
      }
      myFile = SD.open(filename, FILE_WRITE);
      calculateAscentRate();
      dtostrf((millis()/1000.00),6,2,elapsedTime);
      if(myFile){        
        snprintf(sdLine,
                  600,
                  "%s,%s,%lu,%u,%s,%u,%u,%s,%s,%u,%s,%s,%s,%d,%s,%lu,%lu,%d,%s,%u,%u,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%s,%s,%s,%s,%s,%s,%d,%s,%s,%s,%u,%u,%u,%u,%s,%s,%s,%s,%d",
                  elapsedTime,
                  RTCO.timestamp,
                  RTCO.unix,
                  GPS.Satellites,
                  GPS.Timestamp,
                  GPS.DateAge,
                  GPS.HDOP,
                  getLatitudeChar(),
                  getLongitudeChar(),
                  GPS.Age,
                  getLaunchLatitudeChar(),
                  getLaunchLongitudeChar(),
                  getLaunchDistanceChar(),
                  (int)GPS.LaunchCourse,
                  getLaunchCardinalDirection(),
                  (unsigned long)GPS.Altitude,
                  (unsigned long)GPS.AltitudeF,
                  (int)GPS.Course,
                  getCardinalDirection(),
                  (int)GPS.SpeedK,
                  (int)GPS.Speed,
                  (int)DS18B20_Temperatures[0],
                  (int)DS18B20_Temperatures[1],
                  (int)cToF(tempFromPressure),
                  (int)getHumidityRead(),       
                  (int)cToF(getBMEDewPoint()),        
                  (int)getBMPAltitude(),
                  (int)getBMPAltitudeF(),
                  (int)getBaselinePressure(),
                  (int)getPressureRead(),
                  readHumidChar(),
                  readHTempCChar(),
                  readHTempFChar(),
                  readHIFChar(),
                  readHICChar(),
                  readDPCChar(),
                  readDPFChar(),
                  freeRam(),
                  getVoltageChar(0),
                  getVoltageChar(1),
                  getVoltageChar(2),
                  startedBurstCam(),
                  hitAltitude(),
                  isBuzzerOn(),
                  isAnomalyOn(),
                  getAscentRateMSChar(),
                  getAscentRateFTChar(),
                  getAscentRateMPHChar(),
                  getLastTXtime(),
                  getNextAPRS()
                  );
        #ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(sdLine);
        #endif
        
        myFile.println(sdLine);
        myFile.close();
        #ifdef USE_WATCHDOG
          wdt_reset();
        #endif
        if(GPS.AltitudeF<BUZZER_ALTITUDE && (hitAltitude() || didWriteFile())){    
          digitalWrite(ANOMALY_ALARM_PIN,LOW);
          anomaly_on = true;
          #ifdef DEBUG_SERIAL
            DEBUG_SERIAL.println(F("Altitude file detected and low altitude.  Sound anomaly alarm!"));
          #endif
        }else{    
          digitalWrite(ANOMALY_ALARM_PIN,HIGH);
          anomaly_on = false;
        }        
        last_sd_write = millis();
      }else{    
        digitalWrite(ANOMALY_ALARM_PIN,LOW);
        anomaly_on = true;
        last_sd_write = millis();
        #ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("Unable to write to SD card"));
        #endif
      }
    }
  }
}

void dateTime(uint16_t* date, uint16_t* time) {
 *date = FAT_DATE(RTC.now().year(), RTC.now().month(), RTC.now().day());
 *time = FAT_TIME(RTC.now().hour(), RTC.now().minute(), RTC.now().second());
}

bool altFileExists(){
  bool retVal = SD.exists(alt_filename);
  #ifdef DEBUG_SERIAL
    if(retVal){
      #ifdef DEBUG_SERIAL
        DEBUG_SERIAL.println(F("HAS ALTITUDE FILE!"));
      #endif
    }else{
      #ifdef DEBUG_SERIAL
        DEBUG_SERIAL.println(F("NO ALTITUDE FILE YET."));      
      #endif
    }
  #endif
  return retVal;
} 

bool writeAltFileSD(){
  if(!SD.exists(alt_filename)){
    altFile = SD.open(alt_filename,FILE_WRITE);
    altFile.println(RTCO.timestamp);
    altFile.close();
    return true;
  }
  return false;
}

bool isAnomalyOn(){
  return anomaly_on;
}

#else
  bool writeAltFileSD(){
    return false;
  }
#endif
