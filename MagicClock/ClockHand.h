#ifndef ClockHand_h
#define ClockHand_h

#include <Arduino.h>
#include <AccelStepper.h>

class ClockHand: public AccelStepper
{
  private:
    static uint8_t s_EEPROM_ADDR;

    uint8_t positions;
    uint8_t current_position;
    float speed;
    uint16_t steps;
    int8_t scale;
    uint8_t memaddr;

    void move(int32_t relative);

  public:
    ClockHand(bool invert, uint8_t positions, float speed, uint16_t steps,
              int mode, uint8_t pin1, uint8_t pin3, uint8_t pin2, uint8_t pin4);

    // Number of bytes in EEPROM per ClockHand
    static const uint8_t s_EEPROM_ADDR_STEP;

    void setup();
    void run();
    void setNewPosition(uint8_t newpos);
    uint8_t getPosition();
};

#endif
