#include <Arduino.h>
#include "ClockHand.h"
#include <AccelStepper.h>
#include <EEPROM.h>

uint8_t ClockHand::s_EEPROM_ADDR = 0;
// Step is 1 for now, but I'd like to do more sophisticated handling of position storage
// If power is lost while a hand is moving, the position will be wrong
const uint8_t ClockHand::s_EEPROM_ADDR_STEP = 1;

ClockHand::ClockHand(bool invert, uint8_t positions, float speed, uint16_t steps,
                     int mode, uint8_t pin1, uint8_t pin3, uint8_t pin2, uint8_t pin4)
  : AccelStepper(mode, pin1, pin3, pin2, pin4, false)
  , positions{positions}
  , current_position{0}
  , speed{speed}
  , steps{steps}
  , scale{static_cast<int8_t>(invert ? -1 : 1)}
{
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

void ClockHand::move(int32_t relative) {
  enableOutputs();
  AccelStepper::move(relative);
}

void ClockHand::setNewPosition(uint8_t newpos) {
  if(newpos != current_position) {
    int32_t stepPos = (static_cast<int32_t>(current_position) * (float)steps / (float)positions) + distanceToGo();
    stop();
    int32_t newStepPos = static_cast<int32_t>(newpos) * (float)steps / (float)positions;
    int32_t change = newStepPos - stepPos;
    int threshold = steps / 2;
    if (change > threshold) { change -= steps; }
    if (change < -threshold) { change += steps; }
    move(scale*change);

    current_position = newpos;
    EEPROM.write(memaddr,current_position);
    EEPROM.commit();
  }  
}

uint8_t ClockHand::getPosition() {
  return current_position;
}