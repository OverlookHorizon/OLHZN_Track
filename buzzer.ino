bool hit_altitude = false;
bool wrote_file = false;


#ifdef BUZZER
  #ifdef BUZZER_ALTITUDE


static const unsigned int buzzerLength = 250; //milliseconds
unsigned long last_buzzer, last_alt_buzzer = 0;
unsigned int last_buzz_f = 880;
unsigned long turn_off = 0;
boolean buzzer_on = false;


void SetupBuzzer(){
  pinMode(BUZZER,OUTPUT);
  digitalWrite(BUZZER,LOW); 
}

void checkForFile(){
  #ifdef LOG_DATA
  if(altFileExists()){
    hit_altitude = true;
    wrote_file = true;
  }
  #endif
}

void CheckBuzzer(){
  if(!isTX()){
    if(GPS.AltitudeF>BUZZER_ALTITUDE && !hitAltitude()){
      hit_altitude = true;
      writeAltFile();
      buzzer_on = false;      
    }else if(hitAltitude() && GPS.AltitudeF < BUZZER_ALTITUDE && GPS.AltitudeF>0){
      if(!buzzer_on && !isTX()){
        NewTone(BUZZER,880,50);
        delay(60);   
        NewTone(BUZZER,880,50);
        delay(60);   
        NewTone(BUZZER,880,50);
        delay(60);   
        NewTone(BUZZER,880,50);
        delay(60);
        buzzer_on = true;
      }
      if(getVoltage(1)<2){
        if((millis()-last_alt_buzzer) >= 400){        
          last_alt_buzzer = millis();
          if(last_buzz_f!=440){
            //NewTone(BUZZER,440,400);
            last_buzz_f = 440;
          }else{
            NewTone(BUZZER,880,400);
            last_buzz_f = 880;      
            delay(410);
            #ifdef DEBUG_SERIAL
              //DEBUG_SERIAL.println(F("Sound the alarm, we're landing!!!"));
            #endif      
          }
        }
      }
    }else{
      buzzer_on = false;      
      if(GPS.Satellites<3){
        buzzer_on = true;  
        buzz(220); 
      }else if(GPS.Satellites>=3 && GPS.Satellites<6){
        buzzer_on = true;  
        buzz(440);
      }
    } 
  } 
}


void buzz(int f){
  if(!hit_altitude){
    if(millis() - last_buzzer >= (buzzerLength*8)){
      last_buzzer = millis();
      NewTone(BUZZER,f,buzzerLength);
    }  
  }
}

  #endif
#endif

void writeAltFile(){
  if(!wrote_file){    
    writeAltFileSD();
    wrote_file = true;  
  }
}

bool didWriteFile(){
  return wrote_file;
}

bool hitAltitude(){
  return hit_altitude;
}

bool isBuzzerOn(){
  return buzzer_on;
}

