// Process commands

void ProcessCommand() {
  boolean commandError=false;
  int val;
  
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    
    // get incoming byte:
    inChar = Serial.read();
    
    // and build up a command
    buffer[bufferPtr]=toupper(inChar);
    bufferPtr++; 
    buffer[bufferPtr]=(char)0;
    if (bufferPtr>78) { bufferPtr=78; }  // limit maximum command length to avoid overflow

    // process command
    if (inChar=='#') {
      buffer[bufferPtr-1]=0;          // erase the framing char
      strncpy(buffer2,&buffer[1],2);  // get the two letter command, skip the ':'
      strncpy(buffer3,&buffer[3],12); // get the optional numeric tail
      
      commandError=false;

      if (buffer[0]!=':') buffer2[0]='E'; // flag error if framing char isn't there
      
      // set PWM frequency
      if (strcmp(buffer2,"PF") == 0) {
        if ((dir==0) && (numericTail(&pwmDivisor,1,5))) { EEPROM_writeLong(12,pwmDivisor); } else commandError=true;
      } else
      // move in
      if (strcmp(buffer2,"MI") == 0) { 
        if ((dir==0) && (numericTail(&moveMS,0,999999))) { dir=-1; startTime=(long)millis(); } else commandError=true;
      } else
      // move out
      if (strcmp(buffer2,"MO") == 0) { 
        if ((dir==0) && (numericTail(&moveMS,0,999999))) { dir=1;  startTime=(long)millis(); } else commandError=true;
      } else
      // halt motion
      if (strcmp(buffer2,"MH") == 0) { 
        if (strlen(buffer3)==0) { moveMS=now; } else commandError=true;
      } else
      // ask if moving
      if (strcmp(buffer2,"IS") == 0) { 
        if (strlen(buffer3)==0) { if (dir!=0) Serial.print("M#"); else Serial.print("S#"); } else commandError=true;
      } else
      // ask for version
      if (strcmp(buffer2,"IN") == 0) { 
        if (strlen(buffer3)==0) { Serial.print(FirmwareNumber); Serial.print("#"); } else commandError=true;
      } else
      // ask for version
      if (strcmp(buffer2,"IP") == 0) { 
        if (strlen(buffer3)==0) { Serial.print(FirmwareName); Serial.print("#"); } else commandError=true;
      } else
      // ask for full in position
      if (strcmp(buffer2,"GI") == 0) {
        if (strlen(buffer3)==0) { Serial.print(fullIn); Serial.print("#"); } else commandError=true;
      } else
      // ask for max position
      if (strcmp(buffer2,"GM") == 0) { 
        if (strlen(buffer3)==0) { Serial.print(fullOut); Serial.print("#"); } else commandError=true;
      } else
      // ask for current position
      if (strcmp(buffer2,"GP") == 0) { 
        if (strlen(buffer3)==0) { Serial.print(lastTime); Serial.print("#"); } else commandError=true;
      } else
      // ask for current power
      if (strcmp(buffer2,"GL") == 0) { 
        if (strlen(buffer3)==0) { Serial.print(powerLevel); Serial.print("#"); } else commandError=true;
      } else
      // ask for scale
      if (strcmp(buffer2,"GS") == 0) { 
        if (strlen(buffer3)==0) { char reply[20]; dtostrf(MICROS_PER_MS,1,3,reply); Serial.print(reply); Serial.print("#"); } else commandError=true;
      } else
      // set current position as zero
      if (strcmp(buffer2,"SZ") == 0) { 
        if ((dir==0) && (strlen(buffer3)==0)) { lastTime=0; EEPROM_writeLong(base+0,lastTime); } else commandError=true;
      } else
      // set current position as zero
      if (strcmp(buffer2,"SZ") == 0) { 
        if ((dir==0) && (strlen(buffer3)==0)) { lastTime=0; EEPROM_writeLong(base+0,lastTime); } else commandError=true;
      } else
      // set full in position
      if (strcmp(buffer2,"SI") == 0) {
        if ((dir==0) && (numericTail(&fullIn,0,999999))) { EEPROM_writeLong(base+4,fullIn); } else commandError=true;
      } else
      // set full out position
      if (strcmp(buffer2,"SM") == 0) { 
        if ((dir==0) && (numericTail(&fullOut,0,999999))) { EEPROM_writeLong(base+8,fullOut); } else commandError=true;
      } else
      // set power level
      if (strcmp(buffer2,"SL") == 0) { 
        if ((dir==0) && (numericTail(&powerLevel,0,100))) { powerLevel1=map(powerLevel,0,100,0,255); EEPROM_writeLong(20,powerLevel); EEPROM_writeLong(16,powerLevel1); } else commandError=true;
      } else
      // command not recognized
      commandError=true;
      
      if (commandError) {
        moveMS=now; Serial.println("Error");
      }

      bufferPtr=0; buffer[bufferPtr]=0; // clear the last command
    }
  }
}

boolean numericTail(long *val, long low, long high) {
  int l;
  int i;
  boolean isNumeric;
  long temp;

  // get the numeric tail of the command
  l = strlen(buffer3);
  if (l>0) {
    // check to make sure we have a number
    isNumeric=true; for (i=0; i++; i<l) { if ((buffer3[i]<'0') || (buffer3[i]>'9')) { isNumeric=false; } }

    if (isNumeric) {
      temp=atol(buffer3);
      if ((temp>=low) and (temp<=high)) { *val=temp; return true; } else return false;
    } else return false;
    
  } else return false;
}
