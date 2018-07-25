#ifdef USE_RTC

static const unsigned int curYear = 2018;       //be sure to change this each year
bool time_adjusted = false;
char time_buffer[20];

void SetupRTC(){  
  RTC.begin();

  #if RTC_CHIP == 1307
    if (! RTC.isrunning())
      RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  #else
    if(!RTC.initialized())
      RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  #endif

    
  if(RTC.now().year()!=curYear){
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
    if(RTC.now().year()!=curYear){
      #ifdef DEBUG_SERIAL  
        DEBUG_SERIAL.print(F("RTC BAD CLOCK YEAR: "));
        DEBUG_SERIAL.print(RTC.now().year());
        DEBUG_SERIAL.print(F(" != Current Static Year: "));
        DEBUG_SERIAL.println(curYear);
      #endif
      failure(8);
    }
  }
}

void CheckRTC(){
  if(GPS.Satellites>=6 && !time_adjusted && gps.date.year()==curYear){
    RTC.adjust(DateTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second())); 
    time_adjusted = true;
  }  
  sprintf(time_buffer,"%04u-%02u-%02u %02u:%02u:%02u", RTC.now().year(),RTC.now().month(),RTC.now().day(),RTC.now().hour(),RTC.now().minute(),RTC.now().second());
  RTCO.timestamp = time_buffer;
  RTCO.unix = RTC.now().unixtime();  
}


#endif
