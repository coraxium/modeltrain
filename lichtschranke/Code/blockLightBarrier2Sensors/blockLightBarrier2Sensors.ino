
#include "BlockLightBarrier.h"

#define PIN_BLOCK1 11
#define PIN_BLOCK1_INPUT_START 2

#define PIN_BLOCK2 12
#define PIN_BLOCK2_LEFT_OUTER 6
#define PIN_BLOCK2_LEFT_INNER 7
#define PIN_BLOCK2_RIGHT_OUTER 8
#define PIN_BLOCK2_RIGHT_INNER 9


BlockLightBarrier block1;
BlockLightBarrier block2;


void setup() {

  // Jeweils ein Sensorenpaar an jeder Seite, 
  // Alles Sensorenpaare angegeben (bzw. die Ports, wo sie angeschlossen werden sollten
  // Die Adressierung der Pins geschieht automatisch von gegebene Adresse
  // Ausgabe an einen Pin (PIN_BLOCK2)
  block1 = BlockLightBarrier(PIN_BLOCK1, PIN_BLOCK1_INPUT_START);
  block1.init();
  
  // Jeweils ein Sensorenpaar an jeder Seite, 
  // Alles Sensorenpaare angegeben (bzw. die Ports, wo sie angeschlossen werden sollten
  // Ausgabe an einen Pin (PIN_BLOCK2)
  block2.setBlockPin(PIN_BLOCK2);
  block2.setBorderPins(PIN_BLOCK2_LEFT_OUTER, PIN_BLOCK2_LEFT_INNER, PIN_BLOCK2_RIGHT_OUTER, PIN_BLOCK2_RIGHT_INNER);
  block2.init();
}

void loop() {
  
  block1.check();

  block2.check();
  
}
