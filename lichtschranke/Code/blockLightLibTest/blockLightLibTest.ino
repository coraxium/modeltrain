#define DEBUG 1

#define PIN_BLOCK1 10
#define PIN_BLOCK1_INPUT_START 2

#define LED 13

// #define FLOPTIME 750

#include "BlockLightBarrier.h"


BlockLightBarrier block1;


void setup() {

  Serial.begin(9600);
  Serial.println("Online...");  

  // * Jeweils ein Sensorenpaar (Sender und Empfänger) an jeder Seite (SingleMode)
  // * Die Adressierung der Pins geschieht automatisch von der ersten
  //   gegebenen Adresse bzw. dem ersten Pin aus (PIN_BLOCK1_INPUT_START)
  // * Ausgabe an einen Pin (PIN_BLOCK1)
  // * Achsenzähler eingeschaltet, implizit wird dabei auf den Singlemode geschaltet
  block1 = BlockLightBarrier(PIN_BLOCK1, PIN_BLOCK1_INPUT_START, 3);
  //block1.enableAxisCounter();
  block1.init();

  pinMode(LED, OUTPUT);

}

void loop() {
  // Test auf Blockbelegung
  block1.check();

  digitalWrite(LED, block1.isBlocked());

}
