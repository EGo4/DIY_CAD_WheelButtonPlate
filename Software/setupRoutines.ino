void setupDIO(){
  // set encoder inputs
  PORTB |= (1 << PORTB4) | (1 << PORTB5);
  PORTC |= (1 << PORTC6);
  PORTD |= (1 << PORTD7) | (1 << PORTD4);
  PORTE |= (1 << PORTE6);
}

void setupAnalog(){
  // Analog setup
  ADMUX = (1 << REFS0); //=ADC0 |-> MUX0 = ADC1
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1 << ADPS2)| (0 << ADPS1)| (1 << ADPS0); 
  
}

void setupInterupt(){
  // Timer Interupt to 1ms ticks
  TCCR1A = 0; // set TCCR1A register to 0
  TCCR1B = 0; // set TCCR1B register to 0
  TCNT1  = 0; // reset counter value
  
  TCCR1B |= (1 << WGM12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  OCR1A = 16000;

  // PIN interupt for Encoders
  PCMSK0 |= (1 << PCINT4) | (1 << PCINT5);
  PCICR |= (1 << PCIE0);
  // There are none because I was to stupid to route them to actuall interupt ports.
}

void setupI2C(){
    Wire.begin(); // wake up I²C bus
    writeRegister(0x20, 0x0D, 0xFF);// GPPU register, set all of port B to pull-up
    writeRegister(0x20, 0x0C, 0xFF);// GPPU register, set all of port A to pull-up
    writeRegister(0x20, 0x03, 0xFF);// IPOLB register, set all of port B to active low
    writeRegister(0x20, 0x02, 0xFF);// IPOLA register, set all of port A to active low

    writeRegister(0x21, 0x0D, 0xFF);// GPPU register, set all of port B to pull-up
    writeRegister(0x21, 0x0C, 0xFF);// GPPU register, set all of port A to pull-up
    writeRegister(0x21, 0x03, 0xFF);// IPOLB register, set all of port B to active low
    writeRegister(0x21, 0x02, 0xFF);// IPOLA register, set all of port A to active low
}
