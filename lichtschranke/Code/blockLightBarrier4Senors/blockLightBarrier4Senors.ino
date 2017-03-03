
#include "BlockLightBarrier.h"


#define PIN_BLOCK1_LEFT 2
#define PIN_BLOCK1_RIGHT 3

#define PIN_BLOCK2_LEFT 4
#define PIN_BLOCK2_RIGHT 5

#define PIN_BLOCK3_LEFT 6
#define PIN_BLOCK3_RIGHT 7

#define PIN_BLOCK4_LEFT 8
#define PIN_BLOCK4_RIGHT 9

#define PIN_BLOCK1 10
#define PIN_BLOCK2 11
#define PIN_BLOCK3 12
#define PIN_BLOCK4 A0


BlockLightBarrier block1;
BlockLightBarrier block2;
BlockLightBarrier block3;
BlockLightBarrier block4;



void setup() {


  // Jeweils ein Sensor an jeder Seite, 
  // Ausgabe an einen Pin
  block1.setBorderPins(PIN_BLOCK1_LEFT, PIN_BLOCK1_RIGHT);
  block1.setBlockPin(PIN_BLOCK2);
  block1.init();

  /*
  block2.setBorderPins(PIN_BLOCK2_LEFT, PIN_BLOCK2_RIGHT);
  block2.setBlockPin(PIN_BLOCK2);
  block2.init();

  block3.setBorderPins(PIN_BLOCK3_LEFT, PIN_BLOCK3_RIGHT);
  block3.setBlockPin(PIN_BLOCK3);
  block3.init();

  block4.setBorderPins(PIN_BLOCK4_LEFT, PIN_BLOCK4_RIGHT);
  block4.setBlockPin(PIN_BLOCK4);
  block4.init();
  */

  
  Serial.begin(9600);

}

void loop() {
  
  block1.check();

  /*
  block2.check();

  block3.check();

  block4.check();
  */
  delay(1000);
}
