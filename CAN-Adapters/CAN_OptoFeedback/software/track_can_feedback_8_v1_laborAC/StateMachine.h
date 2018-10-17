#ifndef StateMachine_H
#define StateMachine_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#ifndef DEBUG
#define DEBUG 1
#endif

#define RING_BUFFER_SIZE 64

class StateMachine {
  public:
    StateMachine();
    StateMachine(byte inputPin);
    StateMachine(byte inputPin, byte outputPin);

    void setPins(byte inputPin);
    void setPins(byte inputPin, byte outputPin);
    void setLowCounts(byte counts);
    void init();
    bool process();
    void reset();

    bool hasChanged();
    bool isHigh();
    bool getState();

    void useRingbuffer(bool);

  private:

    bool _initAllowed = false;
    bool _currentState = false;
    bool _changed = false;
    
    byte _lowCounts = 30;
    byte _currentLowCounts = 0;

    byte _inputPin = 8;    // Input
    byte _outputPin = 0;   // Input

    bool _useRingbuffer = false;
    word _ringbufferPointer = 0;
    byte _ringbuffer[];

    unsigned long _lastMillis = 0;
    
    int middleValueOf(byte value);    

    
};
#endif
