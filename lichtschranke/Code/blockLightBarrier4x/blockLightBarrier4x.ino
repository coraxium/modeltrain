


#define DEBUG 1

#define PIN_BLOCK1 10
#define PIN_BLOCK1_FIRST 2
#define PIN_BLOCK1_SECOND 3

#define PIN_BLOCK2 11
#define PIN_BLOCK2_FIRST 4
#define PIN_BLOCK2_SECOND 5

#define PIN_BLOCK3 12
#define PIN_BLOCK3_FIRST 6
#define PIN_BLOCK3_SECOND 7

#define PIN_BLOCK4 A0
#define PIN_BLOCK4_FIRST 8
#define PIN_BLOCK4_SECOND 9

#include <BlockLightBarrier.h>


BlockLightBarrier block1;
BlockLightBarrier block2;
BlockLightBarrier block3;
BlockLightBarrier block4;

void setup() {

  // * Jeweils ein Sensor (Sender und Empfänger) an jeder Seite (SingleMode)
  // * Ausgabe an einen Pin (PIN_BLOCKx)
  block1 = BlockLightBarrier(PIN_BLOCK1, PIN_BLOCK1_FIRST, PIN_BLOCK1_SECOND);
  block2 = BlockLightBarrier(PIN_BLOCK2, PIN_BLOCK2_FIRST, PIN_BLOCK2_SECOND);
  block3 = BlockLightBarrier(PIN_BLOCK3, PIN_BLOCK3_FIRST, PIN_BLOCK3_SECOND);
  block4 = BlockLightBarrier(PIN_BLOCK4, PIN_BLOCK4_FIRST, PIN_BLOCK4_SECOND);
  
  block1.init();
  block2.init();
  block3.init();
  block4.init();

}

void loop() {
  // Test auf Blockbelegung
  block1.check();
  block2.check();
  block3.check();
  block4.check();
}
