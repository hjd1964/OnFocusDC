/*
 * Title       OnFocusDC
 * by          Howard Dutton
 *
 * Copyright (C) 2011 to 2015 Howard Dutton
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 
 *
 * Revision History
 *
 * Date                Version           Comment
 * 11-14-2011          0.65              First release
 * 06-18-2015          1.0a1             Overhaul, replaced command processing code
*/

#define FirmwareNumber "1.0a1"
#define FirmwareName   "On-FocusDC"

#include <EEPROM.h>

// ----------------------------------------------------------------------------------------------------------
// Configuration
#define MICROS_PER_MS 10.0  // microns (1/1000 of a mm) of focus travel per millisecond

// I used a SN754410NE Quad H-Driver (2 of its 4 inputs) to run the focuser motor:
// basically a logic LOW/HIGH on the Arduino pins 9/10 causes the respective motor driver output
// to be either Vmotor or Gnd.  This allows us to configure - Gnd/Gnd (off), Vmot/Gnd (fwd), or Gnd/Vmot (rev)
// ----------------------------------------------------------------------------------------------------------

int foc_pole1 = 9;        // control motor/direction
int foc_pole2 = 10;       // control motor/direction 

char inChar;              // incoming serial byte
char buffer[80];          // command buffer
char buffer2[40];         // short buffer
char buffer3[40];         // short buffer
int bufferPtr = 0;        // position in command buffer

long lastPwmDivisor = 0;  // pulse width modulation speed control
long pwmDivisor = 1;      // pulse width modulation speed control
long moveMS = 0;          // move for this long (milliseconds)
long startTime;
long dir = 0;             // move in this direction (-1 for IN, 0 for STOP, 1 for OUT)
long thisTime = 0;        // current position in ms
long lastTime = 0;        // last position in ms
long fullIn = 0;          // full in position in milliseconds
long fullOut = 1000;      // full out position in milliseconds
long powerLevel = 50;     // power level in percent
byte powerLevel1 = 128;   // power level in byte size value

long now;

//  EEPROM addresses/offsets for permanent storage
long base=100;
#define EE_key  0
#define EE_base 4

void setup()
{
  // start serial port at 9600 bps:
  Serial.begin(9600);

  // set pins that control the focuser motor
  pinMode(foc_pole1, OUTPUT);   // forward
  pinMode(foc_pole2, OUTPUT);   // reverse
  digitalWrite(foc_pole1,0);
  digitalWrite(foc_pole2,0);
  dir=0;

  // read the settings
  long key=EEPROM_readLong(EE_key);
  if (key!=1930230197) {
    EEPROM_writeLong(EE_key,1930230197);
    EEPROM_writeLong(EE_base,100); base=100;
    EEPROM_writeLong(12,pwmDivisor);
    EEPROM_writeLong(16,powerLevel1);

    EEPROM_writeLong(base+0,lastTime);
    EEPROM_writeLong(base+4,fullIn);
    EEPROM_writeLong(base+8,fullOut);
  } else {
    base=EEPROM_readLong(EE_base);
    pwmDivisor=EEPROM_readLong(12);
    powerLevel1=EEPROM_readLong(16);

    lastTime=EEPROM_readLong(base+0);
    fullIn=EEPROM_readLong(base+4);
    fullOut=EEPROM_readLong(base+8);
    
    // move base to the next position, with these rolling writes the EEPROM should be good for >10 million focuser moves
    base+=1; if (base>1000) base=100; EEPROM_writeLong(EE_base,base);
    EEPROM_writeLong(base+0,lastTime);
    EEPROM_writeLong(base+4,fullIn);
    EEPROM_writeLong(base+8,fullOut);
  }
}

void loop()
{
  ProcessCommand();

  lastPwmDivisor=SetPWM(pwmDivisor,lastPwmDivisor);

  // keep track of time
  now=(long)millis()-startTime;

  // where we are now in the travel
  if (dir<0) { thisTime=lastTime-now; } else if (dir>0) { thisTime=lastTime+now; }

  // see if we're moving
  if ((dir!=0) && (now<moveMS) && (((thisTime>-fullIn) & (dir<0)) || ((thisTime<fullOut) & (dir>0)))) {
    if (dir==1) {
      // move out
      // no need to adjust pins until we stop
      digitalWrite(foc_pole1,0);
      analogWrite(foc_pole2,powerLevel1);
      dir=2;
    }
    if (dir==-1) {
      // move in
      digitalWrite(foc_pole2,0);
      analogWrite(foc_pole1,powerLevel1);
      // no need to adjust pins until we stop
      dir=-2;
    }
  } else {
    // if the timer expires, stop all future motion (the timer may roll over some day)
    if (dir!=0) {
      // stop
      digitalWrite(foc_pole1,0);
      digitalWrite(foc_pole2,0);
      dir=0;

      // and record where we are in the travel
      lastTime=thisTime;
      EEPROM_writeLong(base+0,lastTime);
    }
  }

}
