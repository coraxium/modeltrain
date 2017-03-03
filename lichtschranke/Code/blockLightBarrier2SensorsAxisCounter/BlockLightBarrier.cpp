

#include "BlockLightBarrier.h"


BlockLightBarrier::BlockLightBarrier() {
  this->initAllowed = false;
}

// Parametrisierter Konstruktor
BlockLightBarrier::BlockLightBarrier(unsigned int blockPin, unsigned int firstInputPin) {

  this->initAllowed = true;

  this->blockPin = blockPin;   // Output

  this->outer1Pin = firstInputPin++;
  this->inner1Pin = firstInputPin++;
  this->outer2Pin = firstInputPin++;
  this->inner2Pin = firstInputPin;
}


void BlockLightBarrier::init() {
  if (!this->initAllowed) { // Prevents misconfiguration
    return;
  }

  // Init boolean vars
  this->resetBlock();

  // Init ports
  pinMode(this->outer1Pin, INPUT_PULLUP);
  if (this->inner1Pin > 0) {
    pinMode(this->inner1Pin, INPUT_PULLUP);
  }
  pinMode(this->outer2Pin, INPUT_PULLUP);
  if (this->inner2Pin > 0) {
    pinMode(this->inner2Pin, INPUT_PULLUP);
  }

  if (this->inner1Pin > 0 && this->inner2Pin > 0)
  {
    this->singleMode = false;
  }


  if(this->useAxisCounter && !this->singleMode) {
    this->singleMode = true;
  }

  pinMode(this->blockPin, OUTPUT);
}

void BlockLightBarrier::enableAxisCounter() {
  this->useAxisCounter = true;
}

void BlockLightBarrier::resetBlock() {
  this->inner1State = false;
  this->inner2State = false;
  this->outer1State = false;
  this->outer2State = false;

  this->blockExit = false;
  this->blockEnter = false;
  this->blockState = false;

  this->direction = false;
  
  this->axisCounterDiff =  0;
}


void BlockLightBarrier::setBorderPins(byte leftOuter, byte leftInner, byte rightOuter, byte rightInner) {
  this->initAllowed = true;

  this->outer1Pin = leftOuter;
  this->inner1Pin = leftInner;
  this->outer2Pin = rightOuter;
  this->inner2Pin = rightInner;
}

void BlockLightBarrier::setBorderPins(byte leftOuter, byte rightOuter) {
  this->initAllowed = true;

  this->outer1Pin = leftOuter;
  this->inner1Pin = 0;
  this->outer2Pin = rightOuter;
  this->inner2Pin = 0;
}


void BlockLightBarrier::setBlockPin(byte blockPin) {
  initAllowed = true;  // TODO needed?
  this->blockPin = blockPin;   // Output
}

void BlockLightBarrier::setBlockPins(byte* blockPins) {
  initAllowed = true;
  this->blockPin = 0;
  memcpy(this->blockPins, blockPins, sizeof(blockPins));
}

void BlockLightBarrier::check() {

  // read current states
  this->readSensors();

  // read and calculate the current blockstate
  this->checkBlockState();

  // blockState sets now the output
  if(this->blockPin == 0)
  {
    for(int idx = 0; idx < sizeof(this->blockPins); idx++)
    {
      digitalWrite(this->blockPins[idx], this->blockState);   
    }
  }
  else
  {
    digitalWrite(this->blockPin, this->blockState);  
  }
}

/**
 * 
 */
void BlockLightBarrier::readSensors()
{
  this->outer1State = digitalRead(this->outer1Pin);
  if (this->inner1Pin > 0) {
    this->inner1State = digitalRead(this->inner1Pin);
  }

  this->outer2State = digitalRead(this->outer2Pin);
  if (this->inner2Pin > 0) {
    this->inner2State = digitalRead(this->inner2Pin);
  }
}

/**
 * 
 */
void BlockLightBarrier::checkBlockState()
{
  if (!this->blockState && this->outer2State) {
    this->direction = true;
  }
  else if (!this->blockState && this->outer1State) {
    this->direction = false;
  }

  if (this->direction)
  {
    doEnterBlock(this->outer2State, this->inner2State);
    doExitBlock(this->outer1State, this->inner1State);
  }
  else
  {
    doEnterBlock(this->outer1State, this->inner1State);
    doExitBlock(this->outer2State, this->inner2State);
  }
}

/**
 * 
 */
void BlockLightBarrier::doEnterBlock(boolean outer, boolean inner) {

  if (this->singleMode && outer && this->useAxisCounter)
  {
    this->axisCounterDiff++;
    if(this->axisCounterDiff > 0) {
      this->blockState = true;
    }
    return;    
  }
  
  if (this->singleMode && outer && !this->blockState)
  {
    this->blockState = true;
    return;
  }

  if (outer && !inner && !this->blockEnter)
  {
    this->blockEnter = true;
  }
  else if (outer && inner && blockEnter)
  {
    this->blockState = true;
    this->blockEnter = false;
  }
}


void BlockLightBarrier::doExitBlock(boolean outer, boolean inner)
{

  if (this->singleMode && outer && this->useAxisCounter)
  {
    this->axisCounterDiff--;
    
    if(this->axisCounterDiff == 0) {
      this->blockExit = false;
      this->blockState = false;
    }
    return;
  }

  if (this->singleMode) {
    if (outer && this->blockState && !this->blockExit) {
      this->blockExit = true;
    }
    else if (!outer && this->blockState && this->blockExit) {
      this->blockExit = false;
      this->blockState = false;
    }

    return;
  }

  if (!outer && inner
      && !this->blockExit
      && this->blockState)
  {
    this->blockExit = true;
  }
  else
  {
    if (inner && !outer
        && this->blockState)
    {
      this->blockExit = true;
    }
    else
    {
      if (outer && inner
          && this->blockExit
          && this->blockState)
      {
        // NOP
        return;
      }
      else
      {
        if (!outer && !inner
            && this->blockExit
            && this->blockState)
        {
          this->blockExit = false;
          this->blockState = false;
        }
      }
    }
  }
}


bool BlockLightBarrier::isBlocked() {
  return this->blockState;
}

