#ifdef USE_RTC

static const unsigned int curYear = 2017;
bool time_adjusted = false;

void SetupRTC(){  
  RTC.begin();
  if (! RTC.isrunning())
    RTC.adjust(DateTime(__DATE__, __TIME__));

    
  if(RTC.now().year()!=curYear){
    RTC.adjust(DateTime(__DATE__, __TIME__));
    if(RTC.now().year()!=curYear){
      #ifdef DEBUG_SERIAL  
        DEBUG_SERIAL.print(F("RTC BAD CLOCK YEAR: "));
        DEBUG_SERIAL.print(RTC.now().year());
        DEBUG_SERIAL.print(F(" != Current Static Year: "));
        DEBUG_SERIAL.println(curYear);
      #endif
      failure(F("RTC BAD CLOCK"));
    }
  }
}

void CheckRTC(){
  if(GPS.Satellites>=3 && GPS.Lock==1 && !time_adjusted && gps.date.year()==curYear){
    RTC.adjust(DateTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second())); 
    time_adjusted = true;
  }  
  RTCO.timestamp = formatDateParam(RTC.now().year()) + "-" + formatDateParam(RTC.now().month()) + "-" + formatDateParam(RTC.now().day()) + " " + formatDateParam(RTC.now().hour()) + ":" + formatDateParam(RTC.now().minute()) + ":" + formatDateParam(RTC.now().second());  
  RTCO.unix = RTC.now().unixtime();  
}


#endif
