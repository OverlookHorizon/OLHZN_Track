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

void CheckADC(void)
{
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

double ReadADC(int Pin, float Multiplier)
{
  int sensorValue = analogRead(Pin);
  double voltage = sensorValue * (5.0 / 1023.0);
  return (voltage * Multiplier);
}

#endif

double getVoltage(uint8_t x){
  if(x<4){
    return BatteryVoltage[x];
  }else{
    return 0;
  }
}

