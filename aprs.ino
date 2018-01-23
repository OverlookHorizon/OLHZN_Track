
/* From Project Swift - High altitude balloon flight software                 */
/*=======================================================================*/
/* Copyright 2010-2012 Philip Heron <phil@sanslogic.co.uk>               */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/*
 * aprs symbol table:  http://ak6ak.net/Info/APRS%20Symbol%20Chart.html
 */


#ifdef APRS_DATA

#include <util/crc16.h>
#include <avr/pgmspace.h>

#define BAUD_RATE      (1200)
#define TABLE_SIZE     (512)
#define PREAMBLE_BYTES (50)
#define REST_BYTES     (5)

#define PLAYBACK_RATE    (F_CPU / 256)
#define SAMPLES_PER_BAUD (PLAYBACK_RATE / BAUD_RATE)
#define PHASE_DELTA_1200 (((TABLE_SIZE * 1200L) << 7) / PLAYBACK_RATE)
#define PHASE_DELTA_2200 (((TABLE_SIZE * 2200L) << 7) / PLAYBACK_RATE)
#define PHASE_DELTA_XOR  (PHASE_DELTA_1200 ^ PHASE_DELTA_2200)

#define APRS_DEVID "APEHAB"

char * APRS_SYMBOL = "O";
uint8_t APRS_SSID = 11;

String lastTransmissionTime;

// Our variables

boolean transmitting = false;
unsigned long NextAPRS=0;
int aprs_mode=0;
unsigned int APRSSentenceCounter;
unsigned long Seconds = 60;
volatile static uint8_t *_txbuf = 0;
volatile static uint8_t  _txlen = 0;
//unsigned long TIME_SUBTRACTOR = 1463976000;  //    5/23/2016 at  00:00:00
//unsigned long TIME_SUBTRACTOR = 1462075200;  //    5/1/2016 at  00:00:00

#ifdef WIREBUS
extern int DS18B20_Temperatures[];
#endif

static const uint8_t PROGMEM _sine_table[] = {
#include "sine_table.h"
};

// Code

void SetupAPRS(void){
  #ifdef APRS_ENABLE
    pinMode(APRS_ENABLE, OUTPUT);
    digitalWrite(APRS_ENABLE, 0);
  #endif

  // Fast PWM mode, non-inverting output on OC2A
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  
  pinMode(APRS_DATA, OUTPUT);
  randomSeed(analogRead(A6));
  
}

boolean isTX(){
  if(aprs_mode>0){
    return true;
  }else{
    return transmitting;
  }
}

void CheckAPRS(void){
  if ((millis() >= NextAPRS) && (GPS.Satellites >= 4 || getLogStarted()) && (_txlen == 0)){

    if(GPS.AltitudeF>0 && GPS.AltitudeF<990 && !hitAltitude()){
      APRS_SSID = 9;
      APRS_SYMBOL = ">";
    }else{
      APRS_SSID = 11;
      APRS_SYMBOL = "O";
    }

      ////////////////////////////////////////////////////////////////////////////
      //FLIGHT DAY SETTINGS!!
      //COMMENT OUT BELOW FOR TESTING
      //UNCOMMENT BELOW FOR FLIGHT      
      //
      APRS_SSID = 11;
      APRS_SYMBOL = "O";
      //      
      ////////////////////////////////////////////////////////////////////////////
    
      Seconds = 60;
      
      transmitting = true;    
      #ifdef DEBUG_SERIAL  
        DEBUG_SERIAL.println(F("Sending APRS Packet"));      
      #endif
      tx_aprs();
      
      if (aprs_mode == 0){   
        // Normal transmission - wait another minute or whatever
        if(APRS_SSID!=11){          
          //testing with the car
          Seconds = random(25,36);
        }else if(GPS.AltitudeF<3500 && GPS.Speed > 6){
          //really low altitude and still moving
          Seconds = 12;
        }else if(GPS.AltitudeF < 3500 && GPS.Speed <= 6){
          //landed or not launched
          Seconds = random(15,31);
        }else if(GPS.AltitudeF < 18000 && GPS.AltitudeF>=3500){        
          //low altitude and still moving
          Seconds = random(25,61);
        }else if(GPS.AltitudeF > 90000){
          //real high altitude almost at burst        
          Seconds = random(25,61);
        }else{
          //higher altitude
          Seconds = random(50,91);
        }        
        if(Seconds>120 || Seconds<12){
          Seconds = 30;
        }
      }else{
        Seconds = 0;
      }

      if(Seconds>120 || Seconds<0){
        Seconds = 60;
      }
      #ifdef DEBUG_SERIAL  
        DEBUG_SERIAL.print(F("Next packet in ")); 
        DEBUG_SERIAL.print(Seconds); 
        DEBUG_SERIAL.println(F(" seconds"));
      #endif
        
      NextAPRS = millis() + (Seconds * 1000L);
      lastTransmissionTime =  RTCO.timestamp.c_str();
      #ifdef RTTY_INTERVAL
        if(millis() - getLastRTTY() >= (getRTTYInterval()*1000L)){
          #ifdef DEBUG_SERIAL          
            DEBUG_SERIAL.print(F("Set last RTTY to be "));
            DEBUG_SERIAL.print((getRTTYInterval()-5));
            DEBUG_SERIAL.println(F(" seconds ago"));
          #endif
          setLastRTTY(millis() - ((getRTTYInterval()-2)*1000L));
        }
      #endif        
      #ifdef USE_WATCHDOG
        wdt_reset();
      #endif
  }
}

void ax25_frame(const char *scallsign, const char sssid, const char *dcallsign, const char dssid, const char ttl1, const char ttl2, const char *data, ...)
{
  static uint8_t frame[100];
  uint8_t *s;
  uint16_t x;
  va_list va;

//  #ifdef DEBUG_SERIAL         
//      DEBUG_SERIAL.print(F("ax25_frame(")); DEBUG_SERIAL.print(aprs_mode); DEBUG_SERIAL.println(F(")"));
//  #endif
  
  va_start(va, data);
  
  /* Write in the callsigns and paths */
  s = _ax25_callsign(frame, dcallsign, dssid);
  s = _ax25_callsign(s, scallsign, sssid);
  if (ttl1) s = _ax25_callsign(s, "WIDE1", ttl1);
  if (ttl2) s = _ax25_callsign(s, "WIDE2", ttl2);

  /* Mark the end of the callsigns */
  s[-1] |= 1;

  *(s++) = 0x03; /* Control, 0x03 = APRS-UI frame */
  *(s++) = 0xF0; /* Protocol ID: 0xF0 = no layer 3 data */

  vsnprintf((char *) s, 100 - (s - frame) - 2, data, va);
  va_end(va);

  /* Calculate and append the checksum */
  for(x = 0xFFFF, s = frame; *s; s++)
    x = _crc_ccitt_update(x, *s);

  *(s++) = ~(x & 0xFF);
  *(s++) = ~((x >> 8) & 0xFF);

  /* Point the interrupt at the data to be transmit */
  _txbuf = frame;
  _txlen = s - frame;

  /* Enable the timer and key the radio */
  TIMSK2 |= _BV(TOIE2);
  
#ifdef LED_TX
  allLEDoff();
  if (GPS.AltitudeF < 2000){
    digitalWrite(LED_TX, 1);
  }
#endif

#ifdef APRS_ENABLE
  digitalWrite(APRS_ENABLE, 1);
#endif
}

void tx_aprs(void)
{
  char slat[5];
  char slng[5];
  char stlm[75];
  char *ptr;
  static uint16_t seq = 0;
  int32_t aprs_lat, aprs_lon, aprs_alt;
  char Wide1Path, Wide2Path;
  
  // Convert the UBLOX-style coordinates to the APRS compressed format
  aprs_lat = 380926 * (90.0 - GPS.Latitude);
  aprs_lon = 190463 * (180.0 + GPS.Longitude);

  if(GPS.Satellites>=3){
    aprs_alt = GPS.Altitude * 32808 / 10000;
  }else{
    aprs_alt = getRelativeAltitude() * 32808 / 10000;
  }
  
  if (GPS.Altitude > APRS_PATH_ALTITUDE)
  {
    Wide1Path = 0;
    Wide2Path = APRS_HIGH_USE_WIDE2;
  }
  else
  {
    Wide1Path = 1;
    Wide2Path = 1;
  }
  
  ptr = stlm;
  ax25_base91enc(ptr, 2, seq);
  ptr += 2;
  ax25_base91enc(ptr, 2, GPS.Satellites);
  ptr += 2;
#ifdef WIREBUS  
  ax25_base91enc(ptr, 2, (int)DS18B20_Temperatures[0] + 100);
  ptr += 2;
  ax25_base91enc(ptr, 2, (int)cToF(getTempFromPressure()) + 100);
  ptr += 2;
  ax25_base91enc(ptr, 2, (int)DS18B20_Temperatures[1] + 100);
  ptr += 2;
#endif
ax25_base91enc(ptr, 2, (getVoltage(0)*100));  

    
  if (aprs_mode == 0){
//    DEBUG_SERIAL.println(GPS.Latitude,6);
//    DEBUG_SERIAL.println(aprs_lat);
    /* Construct the compressed telemetry format */
    ax25_frame(
      APRS_CALLSIGN, APRS_SSID,
      APRS_DEVID, 0,
      Wide1Path, Wide2Path,
      "!/%s%s%s   /A=%06ld|%s|%s",
      ax25_base91enc(slat, 4, aprs_lat),
      ax25_base91enc(slng, 4, aprs_lon),
      APRS_SYMBOL,
      aprs_alt, stlm, APRS_COMMENT);  // comment,APRS_CALLSIGN, ++APRSSentenceCounter);
  
    #ifdef APRS_TELEM_INTERVAL
      // Send the telemetry definitions every 10 packets
      if(seq>0){
        if(seq % (APRS_TELEM_INTERVAL) == 0){
          aprs_mode = 1;
        }
      }
    #endif
    if(seq==0){
      aprs_mode = 0;
      delay(1000);
    }
    seq++;
  }
#ifdef APRS_TELEM_INTERVAL  
  #define APRS_PARM1    ":%-9s:PARM.Satellites"
  #define APRS_UNIT1    ":%-9s:UNIT.Sats"
  #define APRS_EQNS1    ":%-9s:EQNS.0,1,0"

  #define APRS_PARM2   ",External"
  #define APRS_UNIT2   ",deg.F"
  #define APRS_EQNS2   ",0,1,-100"
  
  #define APRS_PARM3   ",Onboard"
  #define APRS_UNIT3   ",deg.F"
  #define APRS_EQNS3   ",0,1,-100"

  #define APRS_PARM4   ",Internal"
  #define APRS_UNIT4   ",deg.F"
  #define APRS_EQNS4   ",0,1,-100"
  
  #define APRS_PARM5   ",Battery"
  #define APRS_UNIT5   ",Volts"
  #define APRS_EQNS5   ",0,0.01,0"

  else if (aprs_mode >= 1){
    char s[20];

    strncpy_P(s, PSTR(APRS_CALLSIGN), 7);
    if(APRS_SSID) snprintf_P(s + strlen(s), 4, PSTR("-%i"), APRS_SSID);

    if (aprs_mode == 1){
      // Transmit telemetry definitions
      ax25_frame(
        APRS_CALLSIGN, APRS_SSID,
        APRS_DEVID, 0,
        0, 0,
        APRS_PARM1 APRS_PARM2 APRS_PARM3 APRS_PARM4 APRS_PARM5,
        s);
      aprs_mode++;
    }else if (aprs_mode == 2){
      ax25_frame(
        APRS_CALLSIGN, APRS_SSID,
        APRS_DEVID, 0,
        0, 0,
        APRS_UNIT1 APRS_UNIT2 APRS_UNIT3 APRS_UNIT4 APRS_UNIT5,
        s);
      aprs_mode++;
    }else if (aprs_mode == 3){
      ax25_frame(
        APRS_CALLSIGN, APRS_SSID,
        APRS_DEVID, 0,
        0, 0,
        APRS_EQNS1 APRS_EQNS2 APRS_EQNS3 APRS_EQNS4 APRS_EQNS5,
        s);
      aprs_mode = 0;
    }else{
      aprs_mode = 0;
    }
  }
#endif  
}

ISR(TIMER2_OVF_vect)
{

  static uint16_t phase  = 0;
  static uint16_t step   = PHASE_DELTA_1200;
  static uint16_t sample = 0;
  static uint8_t rest    = PREAMBLE_BYTES + REST_BYTES;
  static uint8_t byte;
  static uint8_t bit     = 7;
  static int8_t bc       = 0;
  uint8_t value;
  
  /* Update the PWM output */
  value = pgm_read_byte(&_sine_table[(phase >> 7) & 0x1FF]);
  #ifdef APRS_PRE_EMPHASIS
  if (step == PHASE_DELTA_1200)
  {
    value = (value >> 1) + 64;
  }
  #endif
  OCR2B = value;
  phase += step;

  if(++sample < SAMPLES_PER_BAUD) return;
  sample = 0;

  /* Zero-bit insertion */
  if(bc == 5)
  {
    step ^= PHASE_DELTA_XOR;
    bc = 0;
    return;
  }

  /* Load the next byte */
  if(++bit == 8)
  {
    bit = 0;

    if(rest > REST_BYTES || !_txlen)
    {
      if(!--rest)
      {
        // Disable radio, Tx LED off, disable interrupt

        #ifdef APRS_ENABLE
          digitalWrite(APRS_ENABLE, 0);
        #endif

        #ifdef LED_TX
          digitalWrite(LED_TX, 0);
        #endif
        transmitting = false;
        
        TIMSK2 &= ~_BV(TOIE2);

        /* Prepare state for next run */
        phase = sample = 0;
        step  = PHASE_DELTA_1200;
        rest  = PREAMBLE_BYTES + REST_BYTES;
        bit   = 7;
        bc    = 0;
        return;
      }

      /* Rest period, transmit ax.25 header */
      byte = 0x7E;
      bc = -1;
    }
    else
    {
      /* Read the next byte from memory */
      byte = *(_txbuf++);
      if(!--_txlen) rest = REST_BYTES + 2;
      if(bc < 0) bc = 0;
    }
  }

  /* Find the next bit */
  if(byte & 1)
  {
    /* 1: Output frequency stays the same */
    if(bc >= 0) bc++;
  }
  else
  {
    /* 0: Toggle the output frequency */
    step ^= PHASE_DELTA_XOR;
    if(bc >= 0) bc = 0;
  }

  byte >>= 1;
}
char *ax25_base91enc(char *s, uint8_t n, uint32_t v)
{
  /* Creates a Base-91 representation of the value in v in the string */
  /* pointed to by s, n-characters long. String length should be n+1. */

  for(s += n, *s = '\0'; n; n--)
  {
    *(--s) = v % 91 + 33;
    v /= 91;
  }

  return(s);
}

static uint8_t *_ax25_callsign(uint8_t *s, const char *callsign, const char ssid)
{
  char i;
  for(i = 0; i < 6; i++)
  {
    if(*callsign) *(s++) = *(callsign++) << 1;
    else *(s++) = ' ' << 1;
  }
  *(s++) = ('0' + ssid) << 1;
  return(s);
}

int getAPRSMode(){
  return aprs_mode;
}

unsigned long getNextAPRS(){
  return ((NextAPRS - millis())/1000);
}

String getLastTXtime(){
  return lastTransmissionTime;
}

#endif
