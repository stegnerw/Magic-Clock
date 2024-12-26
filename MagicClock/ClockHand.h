#ifndef ClockHand_h
#define ClockHand_h

#include <Arduino.h>
#include <AccelStepper.h>

class ClockHand: public AccelStepper
{
  private:
    static uint8_t s_EEPROM_ADDR;
    long steps;
    int speed;
    int scale;
    int positions;
    int current_position;
    int memaddr;

  public:
    ClockHand(bool invert, int p, int sp, int st, int m, int c1, int c2, int c3, int c4);

    // Number of bytes in EEPROM per ClockHand
    static const uint8_t s_EEPROM_ADDR_STEP;

    void setup();
    void run();
    void move(long relative);
    void setNewPosition(int newpos);
    int getPosition();
};

#endif