#include <Arduino.h>
#include "ClockHand.h"
#include <AccelStepper.h>
#include <EEPROM.h>

uint8_t ClockHand::s_EEPROM_ADDR = 0;
// Step is 1 for now, but I'd like to do more sophisticated handling of position storage
// If power is lost while a hand is moving, the position will be wrong
const uint8_t ClockHand::s_EEPROM_ADDR_STEP = 1;

ClockHand::ClockHand(bool invert, int pos, int sp, int st, int m, int c1, int c2, int c3, int c4): AccelStepper(m, c1, c3, c2, c4) {
  // Position and speed related variables
  positions = pos;
  steps = st;
  speed = sp;
  scale = (invert ? -1 : 1);
  current_position = 0;

  // Grab and increment EEPROM address
  memaddr = s_EEPROM_ADDR;
  s_EEPROM_ADDR += s_EEPROM_ADDR_STEP;
}

void ClockHand::setup() {
  // Set up and read from EEPROM
  Serial.print("Pos for EEPROM address ");
  Serial.print(memaddr);
  Serial.print(": ");
  current_position = EEPROM.read(memaddr);
  if(current_position > positions-1) {
    Serial.print("Invalid value stored: ");
    Serial.print(current_position);
    Serial.print(" - defaulting to ");
    current_position = 0;
  }
  Serial.println(current_position);

  // Set up stepper params
  setMaxSpeed(1000);
  setSpeed(speed);
  setAcceleration(speed*10);
}

void ClockHand::run() {
  AccelStepper::run();
  if(!isRunning()) {
    disableOutputs();
  }
}

void ClockHand::move(long relative) {
  enableOutputs();
  AccelStepper::move(relative);
}

void ClockHand::setNewPosition(int newpos) {
  if(newpos != current_position) {
    long stepPos = ((long)current_position * (float)steps / (float)positions) + distanceToGo();
    stop();
    long newStepPos = (long)newpos * (float)steps / (float)positions;
    long change = newStepPos - stepPos;
    int threshold = steps / 2;
    if (change > threshold) { change -= steps; }
    if (change < -threshold) { change += steps; }
    move(scale*change);

    current_position = newpos;
    EEPROM.write(memaddr,current_position);
    EEPROM.commit();
  }  
}

int ClockHand::getPosition() {
  return current_position;
}