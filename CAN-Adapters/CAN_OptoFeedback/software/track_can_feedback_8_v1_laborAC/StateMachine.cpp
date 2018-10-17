
#include "StateMachine.h"


StateMachine::StateMachine() {
  this->reset();
}

/**
   inputPin Signal innput
*/
StateMachine::StateMachine(byte inputPin) {
  this->reset();

  this->setPins(inputPin);
  this->init();
}

/**
   inputPin Signal innput
   outputPin Will be switched with state from inputPin, i .e .for a LED
*/
StateMachine::StateMachine(byte inputPin, byte outputPin) {

  this->reset();

  this->setPins(inputPin, outputPin);
  this->init();
}


/**
   Inits the confiugured ports
*/
void StateMachine::init() {
  if (this->_inputPin > 0)
  {
    pinMode(this->_inputPin, INPUT);
  }

  if (this->_outputPin > 0)
  {
    pinMode(this->_outputPin, OUTPUT);
  }
}

/**
   Resets the vars to "factory" values
*/
void StateMachine::reset() {
  this->_initAllowed = false;
  this->_inputPin = 0;
  this->_outputPin = 0;
  this->_currentState = false;
  this->_lowCounts = 30;
}

/**
   inputPin Signal innput
*/
void StateMachine::setPins(byte inputPin) {
  this->_initAllowed = true;
  this->_inputPin = inputPin;
  this->_outputPin = 0;
  pinMode(this->_inputPin, INPUT);
}

/**
   inputPin Signal innput
   outputPin Will be switched with state from inputPin, i .e .for a LED
*/
void StateMachine::setPins(byte inputPin, byte outputPin) {
  this->_initAllowed = true;
  this->_inputPin = inputPin;
  this->_outputPin = outputPin;
  pinMode(this->_inputPin, INPUT);
  pinMode(this->_outputPin, OUTPUT);
}

/**
   Setting threshold for the LOW-filter.After that a port is LOW.
*/
void StateMachine::setLowCounts(byte counts)
{
  this->_lowCounts = counts;
}

/**
   TRUE, when high
*/
bool StateMachine::isHigh() {
  return this->_currentState == HIGH;
}


/**
   Gets the current state
*/
bool StateMachine::getState() {
  return this->_currentState;
}

/**
   Reads the input port and calculates the state
*/
bool StateMachine::process() {

  bool thisState = false;
  this->_changed = false;
  unsigned int currentMillis = millis();

  bool pinValue = digitalRead(this->_inputPin);

  // Mit dem Ringpuffer als Filter bekommen wir u.U. bessere Werte,
  // wenn das Signal stark wechselt. ==> 10ms HIGH?
  if (this->_useRingbuffer)
  {
    pinValue = this->middleValueOf(pinValue);
  }

  /*
    HIGH 1
    +------+      +------+      +------+      +------+      +------+
           |      |      |      |      |      |      |      |
           |      |      |      |      |      |      |      |
           +------+      +------+      +------+      +------+
    LOW  0
  */

  // Wir schalten bei einem HIGH (PinValue > 0) immer sofort auf TRUE
  if (pinValue > 0)
  {
#if DEBUG
    if (this->_currentLowCounts > 3)
    {
      
      Serial.print(currentMillis);Serial.print(" ");Serial.print(currentMillis - this->_lastMillis);Serial.print(" [STATEMACHINE] HIGH");
      this->_lastMillis = currentMillis;
    }
#endif
    thisState = true;

    // LOW-Counter wird immer bei einem HIGH genullt,
    // weil ja nur direkt aufeinandefolgende LOW gezaehlt werden sollen
    this->_currentLowCounts = 0;
  }
  else
  {
    // Es gab ein LOW (<1), aber die Anzahl der LOWs zaehlen wir erstmal.
	// Koennte ja ein Ausreisser sein.
    this->_currentLowCounts++;
	
    // Wenn _currentLowCounts mindestens _lowCounts sind,
    // dann wird ein LOW im System angenommen.
    if (this->_currentLowCounts >= this->_lowCounts)
    {
#if DEBUG
      currentMillis = millis();
      Serial.print(currentMillis);Serial.print(" ");Serial.print(currentMillis - this->_lastMillis);Serial.print(" [STATEMACHINE] LOW ("); Serial.print(this->_currentLowCounts); Serial.println(")");
      this->_lastMillis = currentMillis;
#endif
      thisState = false;
      this->_currentLowCounts = 0;
    }
  }


  // change detected through diff of thisState and _currentState
  if (thisState != this->_currentState)
  {
#if DEBUG
      currentMillis = millis();
      Serial.print(currentMillis);Serial.print(" ");Serial.print(currentMillis - this->_lastMillis);Serial.print(" [STATEMACHINE] CHANGE DETECTED: "); Serial.println(thisState ? "HIGH" : "LOW");
      this->_lastMillis = currentMillis;
#endif
    
    this->_changed = true;                // Change detected
    this->_currentState = thisState;      // thisState is the new _currentState

    if (this->_outputPin > 0)             // wenn ein outputPin (LED) gesetzt wurde, schalten
    {
      digitalWrite(this->_outputPin, this->_currentState);
    }
  }

  return this->_currentState;
}

/**
   Change was detected, when TRUE
*/
bool StateMachine::hasChanged() {
  return this->_changed;
}

/**
   Using ringbuffer
*/
void StateMachine::useRingbuffer(bool use) {
  this->_useRingbuffer = use;

  // Set data of ringbuffer to defined zero
  if (this->_useRingbuffer)
  {
    this->_lowCounts = RING_BUFFER_SIZE / 8;  // empirisch
    for (int i = 0; i < RING_BUFFER_SIZE; i++)
    {
      this->_ringbuffer[i] = 0;
    }
  }
}

/**
   Mittelwertberechnung ueber einen Ringpuffer
*/
int StateMachine::middleValueOf(byte value)
{
  byte val = 0;

  // wir setzen einen hohen Wert für HIGH,
  // um die Dynamik der Messwerte zu erhöhen, die
  // bei der Mittelwertberechnung ueber den
  // Ringbuffer zum Tragen kommt
  if (value == HIGH)
  {
    val = 127;
  }

  this->_ringbuffer[this->_ringbufferPointer++] = val;
  if (this->_ringbufferPointer >= RING_BUFFER_SIZE) {
    this->_ringbufferPointer = 0;
  }

  int smoothValue = 0;
  for (int i = 0; i < RING_BUFFER_SIZE; i++) {
    smoothValue += this->_ringbuffer[i];
  }

  return (smoothValue / RING_BUFFER_SIZE);
}

