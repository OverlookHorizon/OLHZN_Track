
bool started_burst_cam = false;

#ifdef BURST_CAM_PIN
  #ifdef BURST_CAM_ALT

void SetupBurstCamera(){
  pinMode(BURST_CAM_PIN,OUTPUT);
  digitalWrite(BURST_CAM_PIN,LOW);
}

void CheckBurstCamera(){
  //deprecated on revision 5+
  //burst camera now on for entire flight and control by onboard slide switch
  //no longer controlled by software
  
  if(GPS.Satellites>=3){
      if(GPS.AltitudeF<BURST_CAM_ALT && !startedBurstCam() && !didWriteFile()){
        //digitalWrite(BURST_CAM_PIN,LOW);
      }else if(GPS.AltitudeF>=BURST_CAM_ALT && !startedBurstCam()){
        started_burst_cam = true;
        digitalWrite(BURST_CAM_PIN,HIGH);
      }else if(GPS.AltitudeF<BUZZER_ALTITUDE && (startedBurstCam() || didWriteFile())){        
        //digitalWrite(BURST_CAM_PIN,LOW);
      } 
      digitalWrite(BURST_CAM_PIN,HIGH);
  }
}

  #endif
#endif


bool startedBurstCam(){
  return started_burst_cam;
}
