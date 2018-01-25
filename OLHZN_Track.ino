/*
 * 
 * TODO:
 * add error debugging by writing to SD card for errors
 * add different LED pattern & tone for SD card failure since we can't debug at that point
 * edit the NewTone library to change timers to avoid timer conflicts
 * use json objects for settings and persistent storage inside a file on the SD card
 * 
 * EXTERNAL LIBRARY DEPENDENCIES
 * RTClib.h
 * NewTone.h
 * OneWire.h
 * DallasTemperature.h
 * TinyGPS++.h
 * DHT.h
 * SFE_BMP180.h
 * SD.h
 * 
 * FAILURE ERROR CODES
 * In the event of an initial boot failure the WARN light will flash, the Piezo will beep and the Serial
 * output will print a 3 digit error code.  Below are the codes and their meaning:
 * 
 * 501: Number of temperature sensors detected does not equal the number of expected sensors
 * 502: Bad GPS wiring. Not receiving data from GPS chip.
 * 503: Bad BMP180 wiring. Not communicating with BMP180 chip.
 * 504: Unable to retrieve pressure measurement from BMP180
 * 505: Unable to start a pressure measurement with the BMP180
 * 506: Unable to retrieve temperature measurement from BMP180
 * 507: Unable to start a temperature measurement with the BMP180
 * 508: Bad or misconfigured RTC clock
 * 509: Missing or corrupt SD card or SD card shield
 * 510: SD Card file opening failed when writing header row
 * 
 */ 
//------------------------------------------------------------------------------------------------------
// CONFIGURATION SECTION.

  // RTTY settings
//  #define RTTY_PAYLOAD_ID   "CHANGEME"          // Do not use spaces.
//  #define RTTY_FREQUENCY    144.390           // For devices that are frequency-agile
//  #define RTTY_BAUD         100               // Comment out if not using RTTY
//  #define RTTY_SHIFT        170               // Only used on boards where PWM is used for RTTY.
//  #define RTTY_PWM            1
//  #define RTTY_ENABLE         A3
//  #define RTTY_DATA           5
//  #define RTTY_INTERVAL       45 //seconds
//  #define RTTY_ATTEMPTS        3

  // APRS settings
  #define APRS_CALLSIGN    "CHANGEME"            // Max 6 characters
  #define APRS_PATH_ALTITUDE   2000              // Below this altitude, ** in metres **, path will switch to WIDE1-1, WIDE2-1.  Above it will be or path or WIDE2-1 (see below)
  #define APRS_HIGH_USE_WIDE2    1               // 1 means WIDE2-1 is used at altitude; 0 means no path is used
  
  #define APRS_PRE_EMPHASIS                      // Comment out to disable 3dB pre-emphasis.
  
  #define APRS_COMMENT     "OverlookHorizon.com"
  #define APRS_TELEM_INTERVAL  5                // How often to send telemetry packets.... every X transmissions.  Comment out to disable  
  #define SD_WRITE_TIME       1000
  #define LED_TX              22
  #define LED_OK              8
  #define LED_WARN            7
  #define APRS_ENABLE         A3
  #define APRS_DATA           9         
  #define GPS_SERIAL          Serial3
  #define WIREBUS             6
  #define EXPECTED_SENSORS    2
  #define BUZZER              A2
  #define BUZZER_ALTITUDE     2000              //value in feet.  Should be about 500 feet above your expected landing elevation. About 2000 for OLHZN flights. TODO: fix the buzzer!
  //#define CANON_PIN           4               //deprecated from rev. 2 board
  #define USE_RTC
  #define LOG_DATA
  #define LOG_PRESSURE
  #define A0_MULTIPLIER        6                //rev. 5 board is 6x    (10k and 2k resistors)
  #define A1_MULTIPLIER        2.5              //rev. 5 board is 2.5x  (15k and 10k resistors)
  #define A6_MULTIPLIER        2.5              //rev. 5 board is 2.5x  (15k and 10k resistors)
  //#define BURST_CAM_PIN       36              //deprecated from rev. 4 board
  //#define BURST_CAM_ALT       0               //deprecated from rev. 4 board
  #define ANOMALY_ALARM_PIN   52
  #define DEBUG_SERIAL        Serial
  #define USE_WATCHDOG                          //automatically reboot the Arduino if it hangs
  
  #define DHTPIN 24                             // what digital pin the humidity sensor is connected to. added in rev. 5
  
  //ONLY USE ONE OF THESE, BELOW
  //#define DHTTYPE DHT11                       // DHT 11
  #define DHTTYPE DHT22                         // DHT 22  (AM2302), AM2321 | we recommend using the DHT22 / AM2302 for larger measurement range
  //#define DHTTYPE DHT21                       // DHT 21 (AM2301)
  
//  
//------------------------------------------------------------------------------------------------------

// Default serial port usage
#ifndef GPS_SERIAL
  #ifndef GPS_I2C
    #define GPS_SERIAL Serial3
  #endif
#endif

#ifndef DEBUG_SERIAL
  #define DEBUG_SERIAL Serial
#endif

#ifdef USE_RTC
  #include "RTClib.h"
  RTC_DS1307 RTC;
#endif

#ifdef USE_WATCHDOG
  #include <avr/wdt.h>
#endif


//#include <aJSON.h>
//aJsonObject* jsonRoot = aJson.createObject();

#ifdef BUZZER
  #include <NewTone.h>
#endif

//#define EXTRA_FIELD_FORMAT ""
//#define EXTRA_FIELD_LIST ""
//#define EXTRA_FIELD_FORMAT    ",%d,%d,%d,%d,%d"          // List of formats for extra fields. Make empty if no such fields.  Always use comma at start of there are any such fields.
//#define EXTRA_FIELD_LIST           ,(int)GPS.Speed, GPS.Course, GPS.Satellites, (int)cToF(getTempFromPressure()), (int)DS18B20_Temperatures[0]

// #define EXTRA_FIELD_FORMAT    ""   // ",%d,%d,%d,%d,%d"          // List of formats for extra fields. Make empty if no such fields.  Always use comma at start of there are any such fields.
// #define EXTRA_FIELD_LIST           // ,(int)((GPS.Speed * 13) / 7), GPS.Course, GPS.Satellites, DS18B20_Temperatures[0], Channel0Average
                                                                // List of variables/expressions for extra fields. Make empty if no such fields.  Always use comma at start of there are any such fields.
#define SENTENCE_LENGTH      100                  // This is more than sufficient for the standard sentence.  Extend if needed; shorten if you are tight on memory.


//------------------------------------------------------------------------------------------------------
//
//  Globals

struct TBinaryPacket

{
	uint8_t 	PayloadIDs;
	uint16_t	Counter;
	uint16_t	BiSeconds;
	float		Latitude;
	float		Longitude;
	int32_t  	Altitude;
};  //  __attribute__ ((packed));

struct TGPS
{
  int Hours, Minutes, Seconds;
  unsigned long SecondsInDay;					// Time in seconds since midnight
  float Longitude, Latitude;
  long Altitude, AltitudeF;
  unsigned int Satellites;
  char* Timestamp;
  float LaunchLongitude, LaunchLatitude, LaunchDistance;
  unsigned int DateAge, HDOP, Age;
  int Speed, SpeedK, LaunchCourse, Course, Direction;
  byte Lock;
  float Pressure;
} GPS;

struct TRTC
{
  char* timestamp;
  unsigned long unix;  
} RTCO;


int SentenceCounter=0;

//------------------------------------------------------------------------------------------------------

void setup(){
  #ifdef USE_WATCHDOG
    wdt_enable(WDTO_4S);
  #endif
  #ifdef DEBUG_SERIAL
    DEBUG_SERIAL.begin(115200);
    DEBUG_SERIAL.println(F(""));
    DEBUG_SERIAL.print(F("OLHZN Track Flight Computer, payload ID(s)"));
    #ifdef RTTY_DATA
      DEBUG_SERIAL.print(F(" "));
      DEBUG_SERIAL.print(RTTY_PAYLOAD_ID);
    #endif  
    #ifdef APRS_DATA
      DEBUG_SERIAL.print(F(" "));
      DEBUG_SERIAL.print(APRS_CALLSIGN);
    #endif  
      
    DEBUG_SERIAL.println(F(""));
    DEBUG_SERIAL.println(F(""));
  #endif
  
  #ifdef USE_RTC
    SetupRTC();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  // Serial port(s)
  #ifdef GPS_SERIAL
    GPS_SERIAL.begin(9600);
  #endif    
  
  #ifdef DEBUG_SERIAL
    #ifdef GPS_I2C
      DEBUG_SERIAL.println(F("I2C GPS"));
    #else
      DEBUG_SERIAL.println(F("Serial GPS"));
    #endif   
    #ifdef RTTY_BAUD
      #ifdef RTTY_DATA
        DEBUG_SERIAL.println(F("RTTY telemetry enabled"));
      #endif
    #endif
    
    #ifdef APRS_DATA 
      DEBUG_SERIAL.println(F("APRS telemetry enabled"));
    #endif
  
    DEBUG_SERIAL.print(F("Free memory = "));
    DEBUG_SERIAL.println(freeRam());
  #endif
  
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  SetupLEDs();
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  SetupGPS();
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  #ifdef A0_MULTIPLIER
  SetupADC();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  #ifdef RTTY_BAUD
  #ifdef RTTY_DATA
    SetupRTTY();
  #endif
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
    
  #ifdef APRS_DATA
    SetupAPRS();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
    
  #ifdef WIREBUS
    Setupds18b20();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  #ifdef BUZZER
  #ifdef BUZZER_ALTITUDE
    SetupBuzzer();
  #endif
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
    
  #ifdef LOG_DATA
    SetupSD();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif

  #ifdef BUZZER
  #ifdef BUZZER_ALTITUDE
    checkForFile();
  #endif
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif

  #ifdef LOG_PRESSURE
    SetupPressure();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  #ifdef DHTPIN
    #ifdef DHTTYPE
      setupDHT();
    #endif
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  #ifdef CANON_PIN
    SetupCamera();
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  #ifdef BURST_CAM_PIN
    #ifdef BURST_CAM_ALT
      SetupBurstCamera();
    #endif
  #endif
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif
  
  goodStart();
  #ifdef USE_WATCHDOG
    wdt_reset();
  #endif

}


void loop(){    
  CheckGPS();
  #ifdef USE_RTC
    CheckRTC();
  #endif
  
  #ifdef APRS_DATA
    CheckAPRS();
  #endif
  
  #ifdef RTTY_BAUD
  #ifdef RTTY_DATA
    CheckRTTY();
  #endif
  #endif
 
  #ifdef A0_MULTIPLIER
    CheckADC();    
  #endif
    CheckLEDs();
  
  #ifdef WIREBUS
    Checkds18b20();
  #endif
  
  #ifdef BUZZER
  #ifdef BUZZER_ALTITUDE
    CheckBuzzer();
  #endif
  #endif
  
  #ifdef LOG_PRESSURE
    CheckPressure();
  #endif

  #ifdef DHTPIN
    #ifdef DHTTYPE
      readDHT();
    #endif
  #endif
  
  #ifdef CANON_PIN
    CheckCamera();
  #endif
  
  #ifdef BURST_CAM_PIN
    #ifdef BURST_CAM_ALT
      CheckBurstCamera();
    #endif
  #endif
  
  #ifdef LOG_DATA
    CheckSD();
  #endif
  
}

int freeRam(void){
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
