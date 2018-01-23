/*
 * 
 * TODO: inside goodStart check to make sure that the following items are all enabled
 * RTTY
 * SD
 * GPS
 * PRESSURE
 * LOG_DATA
 * CAMERA
 * BUZZER
 * APRS
 * WIREBUS
 *
 *
  */


void goodStart(){
  #if defined(BUZZER) && defined(LED_WARN) && defined(LED_OK)
    #if !BAD_VOLTAGE && defined(APRS_ENABLE) && defined(APRS_DATA) && defined(GPS_SERIAL) && defined(DEBUG_SERIAL) && defined(WIREBUS) && defined(EXPECTED_SENSORS) && defined(BUZZER_ALTITUDE) && defined(USE_RTC) && defined(LOG_DATA) && defined(LOG_PRESSURE) && defined(APRS_TELEM_INTERVAL)
        //&& defined(CANON_PIN)  removed 2016-05-09
        //&& defined(RTTY_PWM) && defined(RTTY_ENABLE) && defined(RTTY_DATA) && defined(RTTY_INTERVAL) && defined(RTTY_ATTEMPTS)   removed 2017-03-24
      for(uint8_t i=0;i<3;i++){
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


void failure(String err){
  //handle boot errors
  #ifdef DEBUG_SERIAL
    #ifdef USE_RTC
      DEBUG_SERIAL.println(RTC.now().unixtime());
    #endif
    DEBUG_SERIAL.print(F("ERROR ON DEVICE: "));
    DEBUG_SERIAL.println(err);
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

