bool clutchMode = false;

unsigned long int handleAnalogRead( unsigned long int sysTime, 
                                    unsigned long int lastAnalogReadTime, 
                                    uint8_t analogReadCycle){
  // check wheter a analog read should be performed and do so
  if (sysTime - lastAnalogReadTime > analogReadCycle)
  {
    ADCSRA |= (1 << ADSC);
    return sysTime;
  }
  return lastAnalogReadTime;
}

void readButtons(){
  uint8_t index = mappedButtons(0x20, 4);
  mappedButtons(0x21, index);
}

bool checkForClutchMode(){
  bool x;
  (buttonOutput[8] == 1 & buttonOutput[19] == 1) ? x = true : x = false;
  return x;
}

void calcClutchValues() {
  // check if min or max of both clutches have changed
  clutchRigth < clutchRigthMin ? clutchRigthMin = clutchRigth : 0;
  clutchRigth > clutchRigthMax ? clutchRigthMax = clutchRigth : 0;
  clutchLeft < clutchLeftMin ? clutchLeftMin = clutchLeft : 0;
  clutchLeft > clutchLeftMax ? clutchLeftMax = clutchLeft : 0;

  uint16_t clutchRigthRange = clutchRigthMax - clutchRigthMin;
  uint16_t clutchLeftRange = clutchLeftMax - clutchLeftMin;

  // take respect of the lower deadzone
  uint16_t clutchRigthWithDeadzone = clutchRigth;
  clutchRigthWithDeadzone < (clutchRigthMin + lowerRigthDeadzone) ? clutchRigthWithDeadzone = 0 : clutchRigthWithDeadzone -= (clutchRigthMin + lowerRigthDeadzone);
  uint16_t clutchLeftWithDeadzone = clutchLeft;
  clutchLeftWithDeadzone < (clutchLeftMin + lowerLeftDeadzone) ? clutchLeftWithDeadzone = 0 : clutchLeftWithDeadzone -= (clutchLeftMin + lowerLeftDeadzone);

  // calculate final value incl. upper deadzone
  clutchRigthValue = 1.0 * clutchRigthWithDeadzone / (clutchRigthRange - upperRigthDeadzone - lowerRigthDeadzone);
  clutchLeftValue = 1.0 * clutchLeftWithDeadzone / (clutchLeftRange - upperLeftDeadzone - lowerLeftDeadzone);

  // saturate the values
  clutchRigthValue > 1 ? clutchRigthValue = 1 : 0;
  clutchLeftValue > 1 ? clutchLeftValue = 1 : 0;

  // apply root curve
  clutchRigthValue = sqrt(clutchRigthValue);
  clutchLeftValue = sqrt(clutchLeftValue);
}

void handleClutch(){
  calcClutchValues();  
  
  bool previousClutchMode = clutchMode;
  clutchMode = checkForClutchMode();
  
  if ( (!clutchMode && previousClutchMode || calibrationMode) && (clutchRigthValue > 0 || clutchLeftValue > 0)  ){
    calibrationMode = true;
  }else if (calibrationMode){
    calibrationMode = false;
    saveValueToEEPROM(calcBitePointAsByte(bitePoint), bitePointRegister);
  }

  float clutchFusioned = clutchRigthValue * bitePoint;
  if (clutchLeftValue > 0 && clutchLeftValue > clutchFusioned){
    clutchFusioned = clutchLeftValue;
  }
  Joystick.setZAxis(clutchFusioned * 1023);
}

void resetEncoder(unsigned long int sysTime,
                  unsigned long int encoderTimes[2],
                  uint8_t encoderTimeout){
  // check wheter an encoder was on for a certain time and reset its output if so
  if(sysTime - encoderTimes[0] > encoderTimeout){
    buttonOutput[0] = 0;
    buttonOutput[1] = 0;
  }
  if(sysTime - encoderTimes[1] > encoderTimeout){
    buttonOutput[2] = 0;
    buttonOutput[3] = 0;
  }
                    
}

void readEncoders(){
  if (sysTime - encoderTimes[0] > encoderTimeout)
  {
    if(!((PINB >> PINB5) & 1) != previousState[0]){
      if (!((PINC >> PINC6) & 1) != previousState[0]){
        buttonOutput[0] = 1;
        buttonOutput[1] = 0;
      }else{
        buttonOutput[0] = 0;
        buttonOutput[1] = 1;
      }
      encoderTimes[0] = sysTime;
      previousState[0] = !((PINB >> PINB5) & 1);
    }
  }
  if (sysTime - encoderTimes[1] > encoderTimeout)
  {
    if(!((PINB >> PINB4) & 1) != previousState[1]){
      if (!((PIND >> PIND7) & 1) != previousState[1]){
        if (calibrationMode){
          if(calcBitePointAsByte(bitePoint) < 255)
            bitePoint = calcBitePointAsFloat(calcBitePointAsByte(bitePoint) + 1); 
        }else{
          buttonOutput[2] = 1;
          buttonOutput[3] = 0;
        }
      }else{
        if (calibrationMode){
          if(calcBitePointAsByte(bitePoint) > 0)
            bitePoint = calcBitePointAsFloat(calcBitePointAsByte(bitePoint) - 1); 
        }else{
          buttonOutput[2] = 0;
          buttonOutput[3] = 1;
        }
      }
      encoderTimes[1] = sysTime;
      previousState[1] = !((PINB >> PINB4) & 1);
    }
  }
}

void updateGamepad(){
  for(uint8_t i = 0; i < buttonCount; ++i)
  {
    Joystick.setButton(i, buttonOutput[i]);
  }
}

uint8_t calcBitePointAsByte(float input){
  uint8_t out = input * bitePointResolution;
  return out;
}

float calcBitePointAsFloat(uint8_t input){
  float out = 1.0 * input / bitePointResolution;
  return out;
}

void saveValueToEEPROM(uint8_t valueToSave, uint16_t registerToSaveTo){
  // wait for the CPU to be ready
  while((EECR >> EEPE) & 0x01){}
  //while((SPMCSR >> SELFPRGEN) & 0x01){}
  // set the correct register
  EEARH = (registerToSaveTo >> 8);
  EEARL = registerToSaveTo & 0xFF;

  // setup the data
  EEDR = valueToSave;

  // write process
  cli();
  EECR |= (1 << EEMPE);
  EECR |= (1 << EEPE);
  sei();
}

uint8_t readValueFromEEPROM(uint16_t registerToReadFrom){
  // set the correct register
  EEARH = (registerToReadFrom >> 8);
  EEARL = registerToReadFrom & 0xFF;

  // set the read flag
  EECR |= (1 << EERE);
  
  // read data
  uint8_t out = EEDR;
  return out;
}
