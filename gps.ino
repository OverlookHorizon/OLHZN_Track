#include <TinyGPS++.h>

TinyGPSPlus gps;
byte LastCommand1=0;
byte LastCommand2=0;
unsigned long lastReadingTime = 0;
unsigned long lastAltitude = 0;
float lastLongitude = 0;
float lastLatitude = 0;
float ascentRateMS = 0;
float ascentRateFT = 0;
float ascentRateMPH = 0;
char ascentRateMSChar[10];
char ascentRateFTChar[10];
char ascentRateMPHChar[10];
char latitudeChar[12];
char longitudeChar[12];
char launchLatitudeChar[12];
char launchLongitudeChar[12];
char launchDistanceChar[10];
char gps_time_buffer[20];
char* cardinal_direction;
char* launch_cardinal_direction;


char Hex(char Character){
  char HexTable[] = "0123456789ABCDEF";
  
  return HexTable[Character];
}

void SetupGPS(void){
    SetFlightMode(7);
    #ifdef DEBUG_SERIAL  
      DEBUG_SERIAL.println(F("Setting navigation mode to 7 (Airborne <2g)"));
    #endif
}

void ReadGPS(void){
  while (GPS_SERIAL.available()){   
    gps.encode(GPS_SERIAL.read());
  }
  
  GPS.Satellites = (gps.satellites.isValid()) ? gps.satellites.value() : 0;
  #ifndef SIM_DATA
    GPS.Longitude = (float)(gps.location.isValid()) ? gps.location.lng() : 0;
    GPS.Latitude = (float)(gps.location.isValid()) ? gps.location.lat() : 0;
  #endif
  dtostrf(GPS.Latitude,2,6,latitudeChar);
  dtostrf(GPS.Longitude,2,6,longitudeChar);
  #ifndef SIM_DATA
    GPS.Altitude = (gps.altitude.isValid()) ? gps.altitude.meters() : 0;
    GPS.AltitudeF = (gps.altitude.isValid()) ? gps.altitude.feet() : 0;
  #endif
  GPS.Hours = (gps.time.isValid()) ? gps.time.hour() : 0;
  GPS.Minutes = (gps.time.isValid()) ? gps.time.minute() : 0;
  GPS.Seconds = (gps.time.isValid()) ? gps.time.second() : 0;
  GPS.Speed = (gps.speed.isValid()) ? gps.speed.mph() : 0;
  GPS.SpeedK = (gps.speed.isValid()) ? gps.speed.kmph() : 0;
  GPS.Direction = (gps.course.isValid()) ? gps.course.value() : 0;
  GPS.Course = (gps.course.isValid()) ? gps.course.deg() : 0;
  if(gps.course.isValid()){
    cardinal_direction = TinyGPSPlus::cardinal(gps.course.value());
  }
  
  if(gps.date.isValid()){
    sprintf(gps_time_buffer,"%04u-%02u-%02u %02u:%02u:%02u", gps.date.year(),gps.date.month(),gps.date.day(),gps.time.hour(),gps.time.minute(),gps.time.second());
  }
  GPS.Timestamp = gps_time_buffer;
  GPS.DateAge = (gps.date.isValid()) ? gps.date.age() : 0;
  GPS.HDOP = (gps.hdop.isValid()) ? gps.hdop.value() : 0;
  GPS.Age = (gps.location.isValid()) ? gps.location.age() : 0;

  if(gps.location.isValid() && GPS.LaunchLatitude==0){
      //TODO: log the launch latitude to the SD card for reuse in case the Arduino reboots mid-flight
      GPS.LaunchLatitude = gps.location.lat();
      GPS.LaunchLongitude = gps.location.lng();
      dtostrf(GPS.LaunchLatitude,2,6,launchLatitudeChar);
      dtostrf(GPS.LaunchLongitude,2,6,launchLongitudeChar);
  }else if(gps.location.isValid() && GPS.LaunchLatitude!=0){
      GPS.LaunchDistance = kmToMi(TinyGPSPlus::distanceBetween(gps.location.lat(),gps.location.lng(),GPS.LaunchLatitude,GPS.LaunchLongitude) / 1000);
      GPS.LaunchCourse = TinyGPSPlus::courseTo(gps.location.lat(),gps.location.lng(),GPS.LaunchLatitude,GPS.LaunchLongitude);
      //GPS.LaunchCardinal = TinyGPSPlus::cardinal(GPS.LaunchCourse);
      //strncpy(launch_cardinal_direction,TinyGPSPlus::cardinal(GPS.LaunchCourse),3);      
      //launch_cardinal_direction[3] = '\0';        //guarding for null terminated strings
      launch_cardinal_direction = TinyGPSPlus::cardinal(GPS.LaunchCourse);
      dtostrf(GPS.LaunchDistance,4,2,launchDistanceChar);
  }
  
  if (millis() > 5000 && gps.charsProcessed() > 10){   
    #ifdef USE_WATCHDOG
      if(GPS.Lock!=1 || GPS.Satellites<3){
        wdt_reset();
      }
    #endif
    if(gps.location.isValid() && gps.altitude.isValid())
      GPS.Lock = 1;    
  }else{
    if(millis() > 5000 && gps.charsProcessed() < 10){
      failure(2);
    }
    GPS.Lock = 0;
    
  }
  
}

void SetFlightMode(byte NewMode){
  // Send navigation configuration command
  unsigned char setNav[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};

  setNav[8] = NewMode;

  FixUBXChecksum(setNav, sizeof(setNav));
  
  SendUBX(setNav, sizeof(setNav));
}

void FixUBXChecksum(unsigned char *Message, int Length){ 
  unsigned char CK_A, CK_B;
 
  CK_A = 0;
  CK_B = 0;

  for (int i=2; i<(Length-2); i++){
    CK_A = CK_A + Message[i];
    CK_B = CK_B + CK_A;
  }
  
  Message[Length-2] = CK_A;
  Message[Length-1] = CK_B;
}

void SendUBX(unsigned char *Message, int Length){
  LastCommand1 = Message[2];
  LastCommand2 = Message[3];
  
  for (int i=0; i<Length; i++){
    GPS_SERIAL.write(Message[i]);
  }
}

double kmToMi(double km){
  return (km * 0.621371);
}

void calculateAscentRate(){
  if(lastReadingTime>0 && GPS.Altitude!=lastAltitude && (GPS.Latitude!=lastLatitude || GPS.Longitude!=lastLongitude)){
    ascentRateMS = (float)((float)GPS.Altitude - (float)lastAltitude) / (((float)millis()-(float)lastReadingTime)/1000);
    ascentRateFT = mToF(ascentRateMS);
    ascentRateMPH = ascentRateMS*2.23694;
    dtostrf(ascentRateMS,3,2,ascentRateMSChar);
    dtostrf(ascentRateFT,3,2,ascentRateFTChar);
    dtostrf(ascentRateMPH,3,2,ascentRateMPHChar);
    lastReadingTime = millis();
    lastAltitude = GPS.Altitude;
    lastLatitude = GPS.Latitude;
    lastLongitude = GPS.Longitude;
  }
  if(lastReadingTime<=0){    
    lastReadingTime = millis();
    lastAltitude = GPS.Altitude;
    lastLatitude = GPS.Latitude;
    lastLongitude = GPS.Longitude;
  }
}

float getAscentRate(){
  return ascentRateMS;
}

float getAscentRateFT(){
  return ascentRateFT;
}

float getAscentRateMPH(){
  return ascentRateMPH;
}

char* getAscentRateMSChar(){
  return ascentRateMSChar;
}

char* getAscentRateFTChar(){
  return ascentRateFTChar;
}

char* getAscentRateMPHChar(){
  return ascentRateMPHChar;
}
  
void CheckGPS(void){
  ReadGPS();
}

char* getLatitudeChar(){
  return latitudeChar;
}

char* getLongitudeChar(){
  return longitudeChar;
}

char* getLaunchLatitudeChar(){
  return latitudeChar;
}

char* getLaunchLongitudeChar(){
  return longitudeChar;
}

char* getLaunchDistanceChar(){
  return launchDistanceChar;
}

char* getCardinalDirection(){
  return cardinal_direction;
}

char* getLaunchCardinalDirection(){
  return launch_cardinal_direction;
}

