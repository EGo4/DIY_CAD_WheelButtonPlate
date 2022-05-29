uint8_t mappedButtons(byte address, uint8_t inputBegin){
  byte input = readRegister(0x12, address);
  byte left = readBitInByte(input, 7);
  byte rigth = readBitInByte(input, 6);
  byte up = readBitInByte(input, 5);
  byte down = readBitInByte(input, 4);

  buttonOutput[inputBegin] = readBitInByte(input, 0);
  buttonOutput[inputBegin + 1] = readBitInByte(input, 1);
  buttonOutput[inputBegin + 2] = readBitInByte(input, 2);
  buttonOutput[inputBegin + 3] = readBitInByte(input, 3);

  // Read next register
  input = readRegister(0x13, address);
  buttonOutput[inputBegin + 4] = readBitInByte(input, 1);
  buttonOutput[inputBegin + 5] = readBitInByte(input, 7);
  
  if (readBitInByte(input, 0)){
    if(left || rigth || up || down){
      buttonOutput[inputBegin + 6] = 0;
      buttonOutput[inputBegin + 7] = left;
      buttonOutput[inputBegin + 8] = rigth;
      buttonOutput[inputBegin + 9] = up;
      buttonOutput[inputBegin + 10] = down;      
    }else{
      buttonOutput[inputBegin + 6] = 1;
      buttonOutput[inputBegin + 7] = 0;
      buttonOutput[inputBegin + 8] = 0;
      buttonOutput[inputBegin + 9] = 0;
      buttonOutput[inputBegin + 10] = 0;   
    }
  }else{
      buttonOutput[inputBegin + 6] = 0;
      buttonOutput[inputBegin + 7] = 0;
      buttonOutput[inputBegin + 8] = 0;
      buttonOutput[inputBegin + 9] = 0;
      buttonOutput[inputBegin + 10] = 0;   
  }

  return inputBegin + 11;
}

byte readRegister(int regToRead, int controllerToRead){
  Wire.beginTransmission(controllerToRead);
  Wire.write(regToRead); // set MCP23017 memory pointer to GPIOB address
  Wire.endTransmission();
  Wire.requestFrom(controllerToRead, 1); // request one byte of data from MCP20317
  byte input = Wire.read();
  return input; // store the incoming byte into "inputs"
}

void writeRegister(int controllerToWrite, int regToWrite, int byteToWrite){
  Wire.beginTransmission(controllerToWrite);
  Wire.write(regToWrite); // GPPU register
  Wire.write(byteToWrite); // set all of port B to pull-up
  Wire.endTransmission();
}

bool readBitInByte(byte byteToRead, byte bitToRead){
  return (byteToRead >> bitToRead) & 0x01;
}
