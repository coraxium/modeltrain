#ifndef BlockLightBarrier_H
#define BlockLightBarrier_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif


class BlockLightBarrier {
  public:
    BlockLightBarrier();
  	BlockLightBarrier(unsigned int, unsigned int);
    void init();
    void check();
    void resetBlock();
    bool isBlocked();

    void enableAxisCounter();

    void setBorderPins(byte, byte, byte, byte);  // set double beam mode
    void setBorderPins(byte, byte);  // inits single beam mode
    void setBlockPin(byte);
    void setBlockPins(byte *blockPins);
    
  private:
    bool singleMode = true;
    bool useAxisCounter = false;
  
    bool initAllowed = false;
    bool blockState = false;    
    bool blockEnter = false;
    bool blockExit = false;

    word axisCounterDiff;
    
    bool direction = false; // kommt von Links, geht nach Rechts oder umgekehrt
    
    bool outer1State = false;
    bool inner1State = false;
    bool inner2State = false;
    bool outer2State = false; 

    byte blockPin;   // Output
    byte blockPins[];   // Output
    
    byte outer1Pin;
    byte inner1Pin;
    byte inner2Pin;
    byte outer2Pin; 
    
    void checkBlockState();
    void readSensors();
    void doExitBlock(boolean, boolean);
    void doEnterBlock(boolean, boolean);
};
#endif
