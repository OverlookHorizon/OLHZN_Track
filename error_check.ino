

void goodStart(){
  #if defined(BUZZER) && defined(LED_WARN) && defined(LED_OK)
    //these are all the systems we expect to be online at startup
    #if !BAD_VOLTAGE && defined(APRS_ENABLE) && defined(APRS_DATA) && defined(GPS_SERIAL) && defined(WIREBUS) && defined(EXPECTED_SENSORS) && defined(BUZZER_ALTITUDE) && defined(USE_RTC) && defined(LOG_DATA) && defined(LOG_PRESSURE) && defined(APRS_TELEM_INTERVAL)
        //&& defined(CANON_PIN)  removed 2016-05-09
        //&& defined(RTTY_PWM) && defined(RTTY_ENABLE) && defined(RTTY_DATA) && defined(RTTY_INTERVAL) && defined(RTTY_ATTEMPTS)   removed 2017-03-24
        //&& defined(DEBUG_SERIAL) removed 2018-01-25
      for(uint8_t i=0;i<3;i++){
        //issues a good start up alert via LEDs and Piezo
        #ifdef USE_WATCHDOG
          wdt_reset();
        #endif
        NewTone(BUZZER,110,100);
        digitalWrite(LED_WARN,LOW);
        digitalWrite(LED_OK,HIGH);
        delay(100);
        NewTone(BUZZER,220,100);
        digitalWrite(LED_WARN,HIGH);
        digitalWrite(LED_OK,LOW);
        delay(100);
        NewTone(BUZZER,440,100);
        digitalWrite(LED_WARN,LOW);
        digitalWrite(LED_OK,HIGH);
        delay(100);
        NewTone(BUZZER,880,100);
        digitalWrite(LED_WARN,HIGH);
        digitalWrite(LED_OK,LOW);
        delay(100);
      }
    #else
      for(unsigned int i=0;i<10;i++){
        //issues a bad start up alert via LEDs and Piezo, indicating one of the major components is offline
        #ifdef USE_WATCHDOG
          wdt_reset();
        #endif
        NewTone(BUZZER,500,100);
        digitalWrite(LED_WARN,HIGH);
        digitalWrite(LED_OK,HIGH);
        delay(250);
        NewTone(BUZZER,500,100);
        digitalWrite(LED_WARN,LOW);
        digitalWrite(LED_OK,LOW);
        delay(250);
      }
    #endif
  #endif
}


void failure(uint8_t errNo){
  //handle boot errors
  //if something breaks or fails on initial boot, flash the led then reboot
  #ifdef DEBUG_SERIAL
    #ifdef USE_RTC
      DEBUG_SERIAL.println(RTC.now().unixtime());
    #endif
    DEBUG_SERIAL.print(F("ERROR ON DEVICE: "));
    DEBUG_SERIAL.println(errNo);
  #endif
  for(unsigned int i=0;i<20;i++){
    #ifdef USE_WATCHDOG
      wdt_reset();
    #endif
    #ifdef BUZZER
      NewTone(BUZZER,100,100);
    #endif
    #ifdef LED_WARN
      digitalWrite(LED_WARN,HIGH);
    #endif
    delay(250);
    #ifdef BUZZER
      NewTone(BUZZER,100,100);
    #endif
    #ifdef LED_WARN
      digitalWrite(LED_WARN,LOW);
    #endif
    delay(250);
  }  
  digitalWrite(LED_WARN,LOW);
  digitalWrite(LED_OK,LOW);
  asm volatile ("  jmp 0"); 
}

