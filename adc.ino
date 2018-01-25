/* ========================================================================== */
/*   adc.ino                                                                  */
/*                                                                            */
/*   Code for reading analog battery voltages                                 */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* ========================================================================== */

// Variables

double BatteryVoltage[4];
bool BAD_VOLTAGE = true;

#ifdef A0_MULTIPLIER

unsigned long CheckADCChannels = 0;
char voltageHolder0[8];
char voltageHolder1[8];
char voltageHolder2[8];

void SetupADC(void)
{
  analogReference(DEFAULT);
  #ifdef DEBUG_SERIAL
    DEBUG_SERIAL.println(F("Setup A0 Voltage"));
  #endif
  #ifdef A0_MULTIPLIER
    pinMode(A0, INPUT);  
    BatteryVoltage[0] = ReadADC(A0, A0_MULTIPLIER);
  #endif
  #ifdef A1_MULTIPLIER
    pinMode(A1, INPUT);  
  #endif
  #ifdef A6_MULTIPLIER
    pinMode(A6, INPUT);  
  #endif
  CheckADCChannels = millis() + 1000L;
  if(BatteryVoltage[0]<=1){
    #ifdef DEBUG_SERIAL
      DEBUG_SERIAL.println(F("BAD VOLTAGE"));
    #endif
  }else{
    BAD_VOLTAGE = false;
    #ifdef DEBUG_SERIAL
      DEBUG_SERIAL.print(F("Initial Readings: "));
      DEBUG_SERIAL.println(BatteryVoltage[0]);
    #endif
  }
}

void CheckADC(void){
  if (millis() >= CheckADCChannels && !isTX()){
    #ifdef A0_MULTIPLIER
      BatteryVoltage[0] = ReadADC(A0, A0_MULTIPLIER);
    #endif
    #ifdef A1_MULTIPLIER
      BatteryVoltage[1] = ReadADC(A1, A1_MULTIPLIER);
    #endif
    #ifdef A6_MULTIPLIER
      BatteryVoltage[2] = ReadADC(A6, A6_MULTIPLIER);
    #endif
    CheckADCChannels = millis() + 5000L;
  }
}

double ReadADC(int Pin, float Multiplier){
  int sensorValue = analogRead(Pin);
  double voltage = sensorValue * (5.0 / 1023.0);
  return (voltage * Multiplier);
}

#endif

double getVoltage(uint8_t x){
  if(x<3){
    return BatteryVoltage[x];
  }else{
    return 0;
  }
}

char* getVoltageChar(uint8_t x){
  switch(x){
    case 0:
      dtostrf(BatteryVoltage[0],3,2,voltageHolder0);
      return voltageHolder0;
    case 1:
      dtostrf(BatteryVoltage[1],3,2,voltageHolder1);
      return voltageHolder1;
    case 2:
      dtostrf(BatteryVoltage[2],3,2,voltageHolder2);
      return voltageHolder2;
  }
  return 0;
}

