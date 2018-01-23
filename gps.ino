/* ========================================================================== */
/*   gps.ino                                                                  */
/*                                                                            */
/*   Serial and i2c code for ublox on AVR                                     */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* ========================================================================== */

#include <TinyGPS++.h>

TinyGPSPlus gps;
byte RequiredFlightMode=0;
byte LastCommand1=0;
byte LastCommand2=0;
unsigned long lastReadingTime = 0;
unsigned long lastAltitude = 0;
float ascentRateMS = 0;
float ascentRateFT = 0;
float ascentRateMPH = 0;


char Hex(char Character)
{
  char HexTable[] = "0123456789ABCDEF";
  
  return HexTable[Character];
}

void SetupGPS(void){
    SetFlightMode(7);
    #ifdef DEBUG_SERIAL  
      DEBUG_SERIAL.println(F("Setting navigation mode to 7 (Airborne <2g)"));
    #endif
}

int GPSAvailable(void)
{
  return GPS_SERIAL.available();
}

void ReadGPS(void)
{
  while (GPS_SERIAL.available()){   
    gps.encode(GPS_SERIAL.read());
  }
  
//  #ifdef DEBUG_SERIAL  
//    DEBUG_SERIAL.println(gps.location.rawLng().billionths);
//  #endif

  GPS.Satellites = (gps.satellites.isValid()) ? gps.satellites.value() : 0;
  GPS.Longitude = (float)(gps.location.isValid()) ? gps.location.lng() : 0;
  GPS.Latitude = (float)(gps.location.isValid()) ? gps.location.lat() : 0;
  GPS.Altitude = (gps.altitude.isValid()) ? gps.altitude.meters() : 0;
  GPS.AltitudeF = (gps.altitude.isValid()) ? gps.altitude.feet() : 0;
  GPS.Hours = (gps.time.isValid()) ? gps.time.hour() : 0;
  GPS.Minutes = (gps.time.isValid()) ? gps.time.minute() : 0;
  GPS.Seconds = (gps.time.isValid()) ? gps.time.second() : 0;
  GPS.Speed = (gps.speed.isValid()) ? gps.speed.mph() : 0;
  GPS.SpeedK = (gps.speed.isValid()) ? gps.speed.kmph() : 0;
  GPS.Direction = (gps.course.isValid()) ? gps.course.value() : 0;
  GPS.Course = (gps.course.isValid()) ? gps.course.deg() : 0;
  GPS.Cardinal = (gps.course.isValid()) ? TinyGPSPlus::cardinal(gps.course.value()) : "";

  GPS.Timestamp = (gps.date.isValid()) ? formatDateParam(gps.date.year()) + "-" + formatDateParam(gps.date.month()) + "-" + formatDateParam(gps.date.day()) + " " + formatDateParam(gps.time.hour()) + ":" + formatDateParam(gps.time.minute()) + ":" + formatDateParam(gps.time.second()) : "";
  GPS.DateAge = (gps.date.isValid()) ? gps.date.age() : 0;
  GPS.HDOP = (gps.hdop.isValid()) ? gps.hdop.value() : 0;
  GPS.Age = (gps.location.isValid()) ? gps.location.age() : 0;

  if(gps.location.isValid()){
//    wdt_enable(WDTO_2S);
  }

  if(gps.location.isValid() && GPS.LaunchLatitude==0){
      GPS.LaunchLatitude = gps.location.lat();
      GPS.LaunchLongitude = gps.location.lng();
  }else if(gps.location.isValid() && GPS.LaunchLatitude!=0){
      GPS.LaunchDistance = kmToMi(TinyGPSPlus::distanceBetween(gps.location.lat(),gps.location.lng(),GPS.LaunchLatitude,GPS.LaunchLongitude) / 1000);
      GPS.LaunchCourse = TinyGPSPlus::courseTo(gps.location.lat(),gps.location.lng(),GPS.LaunchLatitude,GPS.LaunchLongitude);
      GPS.LaunchCardinal = TinyGPSPlus::cardinal(GPS.LaunchCourse);
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
      failure(F("GPS WIRING"));
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

float getAscentRate(){
  if(lastReadingTime>0 && GPS.Altitude!=lastAltitude){
//    DEBUG_SERIAL.print(F("ascentRateMS = ("));
//    DEBUG_SERIAL.print(GPS.Altitude);
//    DEBUG_SERIAL.print(F(" - "));
//    DEBUG_SERIAL.print(lastAltitude);
//    DEBUG_SERIAL.print(F(") / (("));
//    DEBUG_SERIAL.print(millis());
//    DEBUG_SERIAL.print(F(" - "));
//    DEBUG_SERIAL.print(lastReadingTime);
//    DEBUG_SERIAL.println(F(")/1000)"));
    ascentRateMS = (float)((float)GPS.Altitude - (float)lastAltitude) / (((float)millis()-(float)lastReadingTime)/1000);
  }
  ascentRateFT = ascentRateMS*3.28084;
  ascentRateMPH = ascentRateMS*2.23694;
  lastReadingTime = millis();
  lastAltitude = GPS.Altitude;
  return ascentRateMS;
}

float getAscentRateFT(){
  return ascentRateFT;
}

float getAscentRateMPH(){
  return ascentRateMPH;
}

void PollGPSTime(void)
{
  ReadGPS();
}

void PollGPSLock(void)
{
  ReadGPS();
}

void PollGPSPosition(void)
{
  ReadGPS();
}
  
void CheckGPS(void)
{
  ReadGPS();
}


