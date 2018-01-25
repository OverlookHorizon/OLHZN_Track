unsigned long NextLEDs=0;
uint8_t last_status = 0;
uint8_t last_light = 0;

void SetupLEDs(void)
{
#ifdef LED_WARN
  pinMode(LED_WARN, OUTPUT);
  digitalWrite(LED_WARN, 0);
  digitalWrite(LED_WARN,1);
  delay(500);
  digitalWrite(LED_WARN,0);
#endif

#ifdef LED_TX
  pinMode(LED_TX, OUTPUT);
  digitalWrite(LED_TX, 0);
  digitalWrite(LED_TX,1);
  delay(500);
  digitalWrite(LED_TX,0);
#endif

#ifdef LED_OK
  pinMode(LED_OK, OUTPUT);
  digitalWrite(LED_OK, 0);
  digitalWrite(LED_OK,1);
  delay(500);
  digitalWrite(LED_OK,0);
#endif

}

void stepLights(){
  if(last_light==0 || last_light==3){
    digitalWrite(LED_WARN,1);
    digitalWrite(LED_TX,0);
    digitalWrite(LED_OK,0);
    last_light = 1;
  }else if(last_light==1){
    digitalWrite(LED_TX,1);
    digitalWrite(LED_WARN,0);
    digitalWrite(LED_OK,0);
    last_light = 2;
  }else if(last_light==2){
    digitalWrite(LED_OK,1);
    digitalWrite(LED_WARN,0);
    digitalWrite(LED_TX,0);
    last_light = 3;
  }
}

void ControlLEDs(int LEDOK, int LEDTX, int LEDWarn)
{
  #ifdef LED_OK
    digitalWrite(LED_OK, LEDOK);
  #endif
  #ifdef LED_WARN
    digitalWrite(LED_WARN, LEDWarn);
  #endif
  #ifdef LED_TX
    digitalWrite(LED_TX, LEDTX);
  #endif
}

void allLEDoff(){
  #ifdef LED_WARN
    digitalWrite(LED_WARN,0);
  #endif
  #ifdef LED_TX
    digitalWrite(LED_TX,0);
  #endif
  #ifdef LED_OK
    digitalWrite(LED_OK,0);
  #endif
}

void CheckLEDs(void){
  if (millis() >= NextLEDs){
    static byte Flash=0;
    
    if (GPS.AltitudeF > 1500){
      // All off
      if(last_status!=5){
        #ifdef DEBUG_SERIAL  
          DEBUG_SERIAL.print(F("Soaring high..."));
          DEBUG_SERIAL.println(GPS.AltitudeF);
        #endif
        last_status = 5;
      }
      ControlLEDs(0,0,0);
    }else if ((GPS.Lock > 0) && GPS.Satellites >= 6){
      if(last_status!=4){
        #ifdef DEBUG_SERIAL  
          DEBUG_SERIAL.print(F("Good GPS..."));
          DEBUG_SERIAL.println(GPS.Satellites);
        #endif
        last_status = 4;
      }
      ControlLEDs(1, 0, 0);
    }else if ((GPS.Lock > 0) && GPS.Satellites >= 4 && GPS.Satellites <6){
      if(last_status!=3){
        #ifdef DEBUG_SERIAL  
          DEBUG_SERIAL.print(F("Waiting for better GPS..."));
          DEBUG_SERIAL.println(GPS.Satellites);
        #endif
        last_status = 3;
      }
      ControlLEDs(Flash, 0, 0);      
    }else if(GPS.Satellites >0 && (GPS.Satellites<4 || GPS.Lock<1)){
      if(last_status!=2){
        #ifdef DEBUG_SERIAL  
          DEBUG_SERIAL.print(F("Weak GPS..."));
          DEBUG_SERIAL.println(GPS.Satellites);
        #endif
        last_status = 2;
      }
      ControlLEDs(Flash,Flash,0);
    }else{
      if(last_status!=1){
        #ifdef DEBUG_SERIAL  
          DEBUG_SERIAL.print(F("No GPS..."));
          DEBUG_SERIAL.println(GPS.Satellites);
        #endif
        last_status = 1;
      }
      ControlLEDs(Flash, Flash, Flash);
    }       
    
    NextLEDs = millis() + 500L;
    Flash = 1-Flash;
  }
}

