# Magic-Clock

Based on [brgerig's Magic-Clock](https://github.com/brgerig/Magic-Clock).
The primary changes include:

- Adding one more hand
- Replacing the NodeMCU Arduino Pro Mini setup with a single Arduino Nano ESP32

## Background

In the Harry Potter books and movies, Harry's friends, the Weasleys, have a magical clock that tracks where all of the family members are.
This project creates a functional version of that clock, using data from phone GPS by way of an MQTT message broker.

## Arduino Libraries

The following Arduino libraries are required:

- PubSubClient
- AccelStepper

## Setup Instructions

- Modify MagicClock/config.h to hold
  - Wi-Fi login info
  - MQTT server info
- Update the pinouts if necessary
- Add or remove people if necessary
  - Declarations around line 20
  - Loop `personX.run()` statements
  - `msgReceived` conditional statements
  - You may also change the names in the topics, prints, and variable names if desired
