//deprecated since revision 2 board
//no longer using canon cameras via the CHDK

bool focusing = false;

#ifdef CANON_PIN

uint8_t photoInterval = 10; //in seconds
static const unsigned int focus_time = 1000; //milliseconds
unsigned long last_photo = 0;

void SetupCamera(){
  pinMode(CANON_PIN,OUTPUT);
  digitalWrite(CANON_PIN,LOW);
}

void CheckCamera(){
  if(GPS.Satellites>=3 || focusing){
    if(millis() - last_photo >= (photoInterval*1000) && !focusing){ 
        digitalWrite(CANON_PIN,HIGH);
        focusing = true;
        last_photo = millis();
    }
    if(focusing && ((millis() - last_photo) >= focus_time)){
      digitalWrite(CANON_PIN,LOW);
      focusing = false;
      last_photo = millis();
      if(GPS.AltitudeF<900){
        photoInterval = 30;
      }else if(GPS.AltitudeF>=900 && GPS.AltitudeF<5000){
        photoInterval = 10;
      }else if(GPS.AltitudeF>=5000 && GPS.AltitudeF<17000){
        photoInterval = 15;
      }else if(GPS.AltitudeF>=17000&&GPS.AltitudeF<34000){
        photoInterval = 20;
      }else if(GPS.AltitudeF>=34000&&GPS.AltitudeF<51000){
        photoInterval = 25;
      }else if(GPS.AltitudeF>=51000&&GPS.AltitudeF<68000){
        photoInterval = 20;
      }else if(GPS.AltitudeF>=68000&&GPS.AltitudeF<90000){
        photoInterval = 15;
      }else if(GPS.AltitudeF>=90000){
        photoInterval = 10;
      }else{
        photoInterval = 15;
      }
    }
  }
}

#endif


bool isFocusing(){
  return focusing;
}
