// PWM Extensions

int SetPWM(int divisor,int lastDivisor) {
  // set pwm frequency if it changed
  if (lastDivisor!=divisor) { 
    int div;
    switch(divisor) {
      case 1: div = 1; break;
      case 2: div = 8; break;
      case 3: div = 64; break;
      case 4: div = 256; break;
      case 5: div = 1024; break;
    }
    setPwmFrequency(foc_pole1, div);
    setPwmFrequency(foc_pole2, div);
    lastDivisor=divisor;
  }
  return lastDivisor;
}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
