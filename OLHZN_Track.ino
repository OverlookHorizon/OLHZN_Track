/*
 * 
 * TODO:
 * add error debugging by writing to SD card for errors
 * add different LED pattern & tone for SD card failure since we can't debug at that point
 * edit the NewTone library to change timers to avoid timer conflicts
 * use json objects or some other method for settings and persistent data storage inside a file on the SD card
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
 * Wire.h
 * SPI.h
 * Adafruit_Sensor.h
 * Adafruit_BME280.h
 * 
 * FAILURE ERROR CODES
 * In the event of an initial boot failure the WARN light will flash, the Piezo will beep and the Serial
 * output will print a 3 digit error code.  Below are the codes and their meaning:
 * 
 * 1: Number of temperature sensors detected does not equal the number of expected sensors
 * 2: Bad GPS wiring. Not receiving data from GPS chip.
 * 3: Bad BMP180 wiring. Not communicating with BMP180 chip.
 * 4: Unable to retrieve pressure measurement from BMP180
 * 5: Unable to start a pressure measurement with the BMP180
 * 6: Unable to retrieve temperature measurement from BMP180
 * 7: Unable to start a temperature measurement with the BMP180
 * 8: Bad or misconfigured RTC clock
 * 9: Missing or corrupt SD card or SD card shield
 * 10: SD Card file opening failed when writing header row
 * 
 */ 
//------------------------------------------------------------------------------------------------------
// CONFIGURATION SECTION.

  // RTTY settings
//  #define RTTY_PAYLOAD_ID   "CHANGEME"        // Do not use spaces.
//  #define RTTY_FREQUENCY    144.390           // For devices that are frequency-agile
//  #define RTTY_BAUD         100               // Comment out if not using RTTY
//  #define RTTY_SHIFT        170               // Only used on boards where PWM is used for RTTY.
//  #define RTTY_PWM            1
//  #define RTTY_ENABLE         A3              //PTT to enable radio
//  #define RTTY_DATA           5               //data output
//  #define RTTY_INTERVAL       45 //seconds    //time between transmission sessions
//  #define RTTY_ATTEMPTS        3              //number of times to repeat the broadcast, per transmission session

  // APRS settings
  //#define SIM_DATA                            // uncomment to manually provide flight data (APRS.ino tab) for simulating a flight path for testing purposes
  #define APRS_CALLSIGN    "CHANGEME"             // Max 6 characters
  #define APRS_PATH_ALTITUDE   2000             // Below this altitude, ** in metres **, path will switch to WIDE1-1, WIDE2-1.  Above it will be or path or WIDE2-1 (see below)
  #define APRS_HIGH_USE_WIDE2    1              // 1 means WIDE2-1 is used at altitude; 0 means no path is used
  
  #define APRS_PRE_EMPHASIS                     // Comment out to disable 3dB pre-emphasis.
  #define APRS_SSID             11               //set APRS_SSID to 9 for testing mode, 11 (or 12) for flight mode
  #define APRS_SYMBOL          "O"              //set APRS_SYMBOL to > for car icon (for testing), O for balloon icon (for flying)

  
  #define APRS_COMMENT     "OverlookHorizon.com"    //enter whatever you want (within reason)
  #define APRS_TELEM_INTERVAL  5                // How often to send telemetry packets.... every X transmissions.  Comment out to disable  
  #define SD_WRITE_TIME       1000              //how often to write to the SD card in milliseconds... don't go less than 1000, it's not a good idea.
  #define LED_TX              22                //on when HX1 is transmitting
  #define LED_OK              8                 //solid when all systems good. Flashing while acquiring GPS signal.
  #define LED_WARN            7                 //off when all systems good. Flashing solo when major error. Flashing with TX and OK when acquiring GPS signal.
  #define APRS_ENABLE         A3                //PTT for the HX1
  #define APRS_DATA           9                 
  #define GPS_SERIAL          Serial3
  #define WIREBUS             6                 //for reading temperature sensors
  #define EXPECTED_SENSORS    2                 //number of temperature sensors you intend to use
  #define BUZZER              A2                //piezo. beeps while acquiring GPS signal.  Supposed to beep during landing, but this part is broken right now.
  #define BUZZER_ALTITUDE     2000              //value in feet.  Should be about 500 feet above your expected landing elevation. About 2000 FT for OLHZN flights. TODO: fix the buzzer!
  //#define CANON_PIN           4               //deprecated from rev. 2 board
  #define USE_RTC                               //whether or not to use the RTC clock onboard a data logging shield
  #define RTC_CHIP            8523              //enter either 1307 (DS1307) or 8523 (PCF8523) for Adafruit logging shields.  DS1307 is the older model RTC chip.  PCF8523 is the new upgraded RTC chip on Rev. B Adafruit logging shields.
  #define LOG_DATA                              //whether or not to log data to an SD card.  If you turn this off, the watchdog timer may reboot the system repeatedly so turn it off or add a reset somewhere else.
  
  #define LOG_PRESSURE                          //whether to use the BMP180 or BME280 breakout boards
  //#define PRESSURE_TYPE       BMP180          //type of pressure breakout board. you can use either the BMP180 breakout board
  #define PRESSURE_TYPE       BME280            //or the BME280 breakout board... this is new (2018-01-31). 
                                                //We like the BME280 better since it also reads humidity.
                                                
  #define A0_MULTIPLIER        6                //voltage multiplier for main arduino battery. rev. 5 board is 6x    (10k and 2k resistors)
  #define A1_MULTIPLIER        2.5              //voltage multiplier for camera 1 battery. rev. 5 board is 2.5x  (15k and 10k resistors)
  #define A6_MULTIPLIER        2.5              //voltage multiplier for camera 2 battery. rev. 5 board is 2.5x  (15k and 10k resistors)
  #define A7_MULTIPLIER        2.5              //voltage multiplier for camera 3 battery. not actually connected on revision 5 board. coming soon via revision 6.
  //#define BURST_CAM_PIN       36              //deprecated from rev. 4 board
  //#define BURST_CAM_ALT       0               //deprecated from rev. 4 board
  #define ANOMALY_ALARM_PIN    52               //LiPo low voltage alarm. Used to report anomalies. GND to GND, PWR to LiPo alarm pin 2.
  //#define DEBUG_SERIAL        Serial          //uncomment for debugging via the serial terminal
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

#ifdef USE_RTC
  #include "RTClib.h"
  #if RTC_CHIP == 1307
    RTC_DS1307 RTC;
  #else
    RTC_PCF8523 RTC;
  #endif
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

struct TBinaryPacket{
	uint8_t 	PayloadIDs;
	uint16_t	Counter;
	uint16_t	BiSeconds;
	float		Latitude;
	float		Longitude;
	int32_t  	Altitude;
};  //  __attribute__ ((packed));

struct TGPS{
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

struct TRTC{
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
