 #include <Joystick.h>
#include "Wire.h"

uint8_t buttonCount = 26;

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  buttonCount, 0,                  // Button Count, Hat Switch Count
  false, false, true,   // no X Y  Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

uint16_t clutchLeft = 1024; // set to 1024 because so you know when it was written by the ADC ISR -> 0 - 1023 10bit ADC
uint16_t clutchRigth = 1024; 
uint16_t clutchRigthMin = 1024;
uint16_t clutchRigthMax = 0;
uint16_t clutchLeftMin = 1024;
uint16_t clutchLeftMax = 0;
uint16_t upperRigthDeadzone, lowerRigthDeadzone, upperLeftDeadzone, lowerLeftDeadzone;
float clutchRigthValue, clutchLeftValue;
uint8_t bitePointResolution = 255;
uint16_t bitePointRegister = 1;
float bitePoint = 0.5;
bool calibrationMode = false;
unsigned long int sysTime = 0;
unsigned long int lastAnalogReadTime = 0;
unsigned long int encoderTimes[2] = {0, 0};
uint16_t analogCycleTime, encoderTimeout;
bool boolADC = true;
uint8_t buttonOutput[26];
uint8_t previousState[2] = {0, 0};

void setup() {
  // set important system parameters
  analogCycleTime = 10;
  encoderTimeout = 50;
  upperRigthDeadzone = 10;
  lowerRigthDeadzone = 5;
  upperLeftDeadzone = 10;
  lowerLeftDeadzone = 5;
  // read bite point for clutch from EEPROM
  bitePoint = calcBitePointAsFloat(readValueFromEEPROM(bitePointRegister));
  
  // run setup routines
  setupDIO();
  setupAnalog();
  setupInterupt();
  setupI2C();
  Joystick.setZAxisRange(0, 1023);
  Joystick.begin();
  previousState[0] = !((PINB >> PINB5) & 1);
  previousState[1] = !((PINB >> PINB4) & 1);
  sei();
}

void loop(){
  lastAnalogReadTime = handleAnalogRead(sysTime, lastAnalogReadTime, analogCycleTime);
  resetEncoder(sysTime, encoderTimes, encoderTimeout);
  readButtons();
  if (clutchLeft < 1024 && clutchRigth < 1024)
    handleClutch();
  updateGamepad();
  //Send controller to sleep since code is so fast that it would cause issues with other devices
  delay(1);
}

ISR(TIMER1_COMPA_vect){
  sysTime++;
}

ISR(ADC_vect){
  if(boolADC){
    clutchRigth = ADCL | (ADCH<<8);
    ADMUX |= (1<<MUX0);
    boolADC = false;
  }else{
    clutchLeft = ADCL | (ADCH<<8);
    ADMUX &= ~(1<<MUX0);
    boolADC = true;
  }
}

ISR(PCINT0_vect){
  readEncoders();
}
