/* ========================================================================== */
/*   rtty.ino                                                                 */
/*                                                                            */
/*   Interrupt-driven RTTY transmission for MTX2 or HX1                       */
/*                                                                            */
/* ========================================================================== */


bool SendingNow = false;
unsigned long last_rtty = 0;


#ifdef RTTY_BAUD
#ifdef RTTY_DATA

// Our variables

char TxLine[SENTENCE_LENGTH];
volatile int SendBit = 0;
int StopBits, DataBits;
int Timer2Count;
int RTTY_Counter;
int SettingFrequency;
unsigned long nextRTTY = 0;

// Code

void SetupRTTY(void){
  pinMode(RTTY_DATA, OUTPUT);
    
  #ifdef RTTY_ENABLE
    pinMode(RTTY_ENABLE, OUTPUT);
    digitalWrite(RTTY_ENABLE, LOW);
  #endif
   
  DataBits = 7;
  StopBits = 2;

  #ifdef RTTY_PWM   
    TCCR3B = TCCR3B & 0b11111000 | 0x01;  //pin 5 only
  #endif
}

void CheckRTTY(void){
  if(((GPS.Satellites>=4 && GPS.Lock==1) || getLogStarted()) && (GPS.AltitudeF >= 5000 || GPS.AltitudeF < 2000) && !isTX()){
    if ((millis() - last_rtty) >= (getRTTYInterval() * 1000L)){
      SendingNow = true;
      BuildSentence(TxLine, RTTY_PAYLOAD_ID);    
      #ifdef DEBUG_SERIAL
        DEBUG_SERIAL.print(TxLine);
        DEBUG_SERIAL.print('\r');
      #endif

      for(uint8_t i=0;i<RTTY_ATTEMPTS;i++){
        rtty_txstring(TxLine);
      }
      
      if((millis() >= NextAPRS)){
        #ifdef DEBUG_SERIAL
          DEBUG_SERIAL.println(F("Delaying NextAPRS by 5 seconds"));
        #endif
        NextAPRS = millis() + 5000L;
      }

      SendingNow = false;
      last_rtty = millis();      
    }
  }
}

void rtty_txbit (int bit) {
    int upper = 114;
  
    if (bit) {
        analogWrite(RTTY_DATA,upper);
    }else {
        analogWrite(RTTY_DATA,100);
    }    
    if(RTTY_BAUD==600){
      delayMicroseconds(1667); //600 baud
    }else if(RTTY_BAUD==300){
      delayMicroseconds(3370); // 300 baud   
    }else if(RTTY_BAUD==100){
      delayMicroseconds(10000);
    }else if(RTTY_BAUD==75){
      delayMicroseconds(13333);
    }else{
      delayMicroseconds(10000);
      delayMicroseconds(10150); // You can't do 20150
    }
}

void rtty_txstring (char * string) {
    char c;
    #ifdef RTTY_ENABLE
      digitalWrite(RTTY_ENABLE,HIGH);
    #endif
    #ifdef LED_TX
      allLEDoff();
      if (GPS.AltitudeF < 2000){
        digitalWrite(LED_TX,HIGH);
      }
    #endif
    c = *string++; 
    while ( c != '\0') {
        rtty_txbyte (c);
        c = *string++;
    }    
    #ifdef DEBUG_SERIAL
      DEBUG_SERIAL.println(F("Done RTTY"));
    #endif
    #ifdef RTTY_ENABLE
      digitalWrite(RTTY_ENABLE,LOW);
    #endif
    #ifdef LED_TX
      digitalWrite(LED_TX,LOW);
    #endif
}

void rtty_txbyte (char c) {
    int i; 
    rtty_txbit (0); // Start bit

    //if(debug) DEBUG_SERIAL.print(c);
    for (i=0;i<DataBits;i++) {
        if (c & 1) rtty_txbit(1);
        else rtty_txbit(0);
 
        c = c >> 1;
    }
    for(i=0;i<StopBits;i++){
      rtty_txbit (1); // Stop bits
    }
}


#endif
#endif


bool isSendingRTTY(){
  return SendingNow;
}
void setLastRTTY(unsigned long v){
  last_rtty = v;
}
unsigned long getLastRTTY(){
  return last_rtty;
}
int getRTTYInterval(){
  if(GPS.AltitudeF>40000){
    return 600;    
  }else if(GPS.AltitudeF>20000){
    return 300;
  }else if(GPS.AltitudeF>10000){
    return 120;
  }else if(GPS.AltitudeF>1000){
    return 60;
  }else{
    #ifdef RTTY_INTERVAL
      return RTTY_INTERVAL;
    #else
      return 45;
    #endif
  }
}
int getRTTYAttempts(){
  //TODO: perform more broadcast sentence repeats when landed and more when long gaps between transmissions?
  //TODO: move this to a dedicated transmitter on a different frequency from APRS
}

