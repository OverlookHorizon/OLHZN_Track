
bool log_started = false;


bool getLogStarted(){
  return log_started;
}

#ifdef LOG_DATA

#include <SD.h>
char filename[20];
char jsonString[200];
uint8_t jsonIndxCtr = 0;
File myFile;
File altFile;
float msrate, ftrate, mphrate;
char alt_filename[15];
char boot_filename[15];
char sdLine[600];
unsigned long last_sd_write = 0;
boolean anomaly_on = true;
boolean needs_header = true;

void SetupSD(){
  pinMode(SS, OUTPUT);   
  
  SdFile::dateTimeCallback(dateTime); 
  if (!SD.begin(10, 11, 12, 13)) {
    failure(F("SD CARD"));
  }
  snprintf(filename,12,"log.csv");
  
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
      myFile.println(F("Elapsed,Onboard Timestamp,Unix Timestamp,Satellites,GPS Timestamp,GPS Date Age,HDOP,Latitude,Longitude,GPS Age,Launch Latitude,Launch Longitude,Distance to Launch Site (mi),Course to Launch Site,Cardinal to Launch Site,Altitude (m),Altitude (ft),Course,Cardinal,Speed (kmph),Speed (mph),Temp Sensor 1,Temp Sensor 2,Internal Temp Sensor (c),Relative Altitude (m), Relative Altitude (ft),Baseline Pressure (mb),Pressure (mb),Humidity (%),Humidity Temp (c),Humidity Temp (f),Heat Index (f),Heat Index (c),Free RAM,Battery Voltage,Camera Voltage 1,Camera Voltage 2,Burst Cam,Hit Altitude,Buzzer On,Anomaly,Ascent Rate (m/s),Ascent Rate (ft/s),Ascent Rate (mph),Last Transmission,Next TX (s)"));
      myFile.close();
    }else{
      failure(F("FILE OPEN FAILED IN HEADER"));
    }
  }
  pinMode(ANOMALY_ALARM_PIN,OUTPUT);  
  snprintf(alt_filename,15,"altitude.txt");
}


void CheckSD(){
  //construct log string & write
  
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
      msrate = getAscentRate();
      ftrate = getAscentRateFT();
      mphrate = getAscentRateMPH();
      if(myFile){        
        snprintf(sdLine,
                  600,
                  "%s,%s,%lu,%u,%s,%u,%u,%s,%s,%u,%s,%s,%s,%d,%s,%lu,%lu,%d,%s,%u,%u,%d,%d,%d,%s,%s,%d,%d,%s,%s,%s,%s,%s,%d,%s,%s,%s,%u,%u,%u,%u,%s,%s,%s,%s,%lu",
                  String(millis() / 1000.00).c_str(),
                  RTCO.timestamp.c_str(),
                  RTCO.unix,
                  GPS.Satellites,
                  GPS.Timestamp.c_str(),
                  GPS.DateAge,
                  GPS.HDOP,
                  String(GPS.Latitude,6).c_str(),
                  String(GPS.Longitude,6).c_str(),
                  GPS.Age,
                  String(GPS.LaunchLatitude,6).c_str(),
                  String(GPS.LaunchLongitude,6).c_str(),
                  String(GPS.LaunchDistance).c_str(),
                  (int)GPS.LaunchCourse,
                  GPS.LaunchCardinal.c_str(),
                  (unsigned long)GPS.Altitude,
                  (unsigned long)GPS.AltitudeF,
                  (int)GPS.Course,
                  GPS.Cardinal.c_str(),
                  (int)GPS.SpeedK,
                  (int)GPS.Speed,
                  (int)DS18B20_Temperatures[0],
                  (int)DS18B20_Temperatures[1],
                  (int)cToF(tempFromPressure),
                  String(getRelativeAltitude()).c_str(),
                  String(getRelativeAltitudeF()).c_str(),
                  (int)baselinePressure,
                  (int)tempPressure[1],
                  String(readHumid()).c_str(),
                  String(readHTempC()).c_str(),
                  String(readHTempF()).c_str(),
                  String(readHIF()).c_str(),
                  String(readHIC()).c_str(),
                  freeRam(),
                  String(getVoltage(0)).c_str(),
                  String(getVoltage(1)).c_str(),
                  String(getVoltage(2)).c_str(),
                  startedBurstCam(),
                  hitAltitude(),
                  isBuzzerOn(),
                  isAnomalyOn(),
                  String(msrate).c_str(),
                  String(ftrate).c_str(),
                  String(mphrate).c_str(),
                  getLastTXtime().c_str(),
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
    altFile.println(RTCO.timestamp.c_str());
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
