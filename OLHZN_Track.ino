/*
 * 
 * TODO:
 * update system for new pins 22 (tx led) 36 (cam 3 enable) and 52 (lipo alarm)
 * add lipo alarm logic to trigger lipo alarm not only for low battery, but also for anomalies
 * add error debugging by writing to SD card for errors
 * add different LED pattern & tone for SD card failure since we can't debug at that point
 * 
 */
//------------------------------------------------------------------------------------------------------
// CONFIGURATION SECTION.

  // RTTY settings
//  #define RTTY_PAYLOAD_ID   "KD2KPZ"          // Do not use spaces.
//  #define RTTY_FREQUENCY    144.390           // For devices that are frequency-agile
//  #define RTTY_BAUD         100               // Comment out if not using RTTY
//  #define RTTY_SHIFT        170               // Only used on boards where PWM is used for RTTY.
//  #define RTTY_PWM            1
//  #define RTTY_ENABLE         A3
//  #define RTTY_DATA           5
//  #define RTTY_INTERVAL       45 //seconds
//  #define RTTY_ATTEMPTS        3

  // Power settings
  //#define POWERSAVING	                      // Comment out to disable GPS power saving
  
  //APRS SSIDs FROM TRACKUINO
  // - Balloons:  11
  // - Cars:       9
  // - Home:       0
  // - IGate:      5

  // APRS settings
  #define APRS_CALLSIGN    "KD2KPZ"               // Max 6 characters
  #define APRS_PATH_ALTITUDE   2000              // Below this altitude, ** in metres **, path will switch to WIDE1-1, WIDE2-1.  Above it will be or path or WIDE2-1 (see below)
  #define APRS_HIGH_USE_WIDE2    1                 // 1 means WIDE2-1 is used at altitude; 0 means no path is used
  
  #define APRS_PRE_EMPHASIS                      // Comment out to disable 3dB pre-emphasis.
  
  #define APRS_COMMENT     "OverlookHorizon.com/flight-10"
  #define APRS_TELEM_INTERVAL  5                // How often to send telemetry packets.... every X transmissions.  Comment out to disable  
  #define SD_WRITE_TIME       1000
  //  #define LED_STATUS      A7
  #define LED_TX              22
  #define LED_OK              8
  #define LED_WARN            7
  #define APRS_ENABLE         A3
  #define APRS_DATA           9         
  #define GPS_SERIAL          Serial3
  #define WIREBUS             6
  #define EXPECTED_SENSORS    2
  #define BUZZER              A2
  #define BUZZER_ALTITUDE     2000 //ft  should be 2000 feet or so
  //#define CANON_PIN           4
  //#define USE_RTC
  //#define LOG_DATA
  //#define LOG_PRESSURE
  #define A0_MULTIPLIER        6   // (new board is 6x... 10k and 2k resistors)
  #define A1_MULTIPLIER        2.5 //new board is 2.5x  (15k and 10k resistors)
  #define A6_MULTIPLIER        2.5 //new board is 2.5x  (15k and 10k resistors)
  //#define BURST_CAM_PIN       36
  //#define BURST_CAM_ALT       0
  //#define ANOMALY_ALARM_PIN   52
  //#define DEBUG_SERIAL        Serial
  //#define DHTPIN 24     // what digital pin we're connected to
  //#define DHTTYPE DHT11   // DHT 11
  //#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)
  #define USE_WATCHDOG
  
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

    /*
            "$%s,%d,%02d:%02d:%02d,%s,%s,%05.5u,%d,%d,%d",
            PAYLOAD_ID,
            SentenceCounter,
	    GPS.Hours, GPS.Minutes, GPS.Seconds,
            LatitudeString,
            LongitudeString,
            GPS.Altitude,
            (int)((GPS.Speed * 13) / 7),
            GPS.Course,
            GPS.Satellites);
    */


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
  String Timestamp, LaunchCardinal, Cardinal;
  float LaunchLongitude, LaunchLatitude, LaunchDistance;
  unsigned int DateAge, HDOP, Age;
  int Speed, SpeedK, LaunchCourse, Course, Direction;
  byte Lock;
  float Pressure;
} GPS;

struct TRTC
{
  String timestamp;
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
