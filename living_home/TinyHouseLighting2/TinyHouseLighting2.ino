/*
  Mit charlyplexing werden mit 3 Pins 6 gleiche LEDs angesteuert,
  die Vorwiderstaende sind direkt an der LED
  Zwei Schalter steuern
    # ein/aus fuer Modus "eine Nacht 8min" bzw. "unendlich", => Schiebeschalter?
      ==> Das ist nur der Fall, wenn wir im Viertelstundenraster arbeiten!!!
    # ein Taster für Programmwahl
*/

// ATMEL ATTINY 25/45/85 / ARDUINO
// Pin 1 is /RESET
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
// Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
//            GND  4|    |5  PB0 (D 0) pwm0
//                  +----+


#include <EEPROM.h>
#include <Bounce2.h>
#include <Charlieplex2.h>
#include <avr/pgmspace.h>

// Control pins
//     5,6,7 => Chip-Pins
//   > 0,1,2 => PBx
//// #### AtTiny85 ####
#define LED_1 0
#define LED_2 1
#define LED_3 2
#define BTN_1_PIN 3
#define BTN_2_PIN 4

//// #### Arduino Nano ####
//#define LED_1 7
//#define LED_2 6
//#define LED_3 5
//#define BTN_1_PIN 10
//#define BTN_2_PIN 11


#define STARTUP_TIME 18

#define BLINK_INTERVAL 300
#define BLINK_ALL -1

#define ADRROM_ROOM 0
#define ADRROM_PRG_BEHAVIOUR 2

#define ROOM_COUNT 6

/*
  Stunden        0
  Stunden + AutoRoom 1
  Quarter        2
  Quarter + AutoRoom 4
*/
#define PRG_MODE_HOUR 0
#define PRG_MODE_HOUR_AUTO 1
#define PRG_MODE_QUARTER 2
#define PRG_MODE_QUARTER_AUTO 3



//define pins in the order you want to adress them
byte ctrlpins[] = {LED_1, LED_2, LED_3};

//initialize object
Charlieplex2 charlie(ctrlpins, sizeof(ctrlpins));     //control instance


// TODO !!!!!!!
// 6 Raeume, Uhrzeit von 18 - 6h, Halbstundenraster, wobei 1h(real) ==> 1/87h(Modell) ==> 60min ==> 0.689 ca 0.7 min ==> 41s
// 12h Realstunden sind also 8,28min (eine Nacht von 8 - 18h)
// Jedes Array im Array enthält die relativen Schaltzeiten einer Lampe
// 0 bedeutet "off", Werte > 0 ist die relative Dauer im bezug auf das
// Raster, aber nicht laenger als ein Raster dauert.
// die Leuchte wird also im Raster eingeschaltet, aber hat eine definierte Leuchtdauer
// negative Werte werden am nächsten Rasterpunkt AUSgeschaltet, beginnen aber "mittendrin" in der Sequenz


// Raumprogrammdefinitionen (ROOM_COUNT) für 24-Stunden-Beleuchtung
const byte rooms[ROOM_COUNT][6][24] PROGMEM = {
  { // Name: NORMAL, Room 1
    /* Uhrzeit    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 */
    /* LED 1 */  {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1},
    /* LED 2 */  {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
    /* LED 3 */  {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0},
    /* LED 4 */  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    /* LED 5 */  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0},
    /* LED 6 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}
  },

  { // Name: Variation 01 (2)
    /* Uhrzeit    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 */
    /* LED 1 */  {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1},
    /* LED 2 */  {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0},
    /* LED 3 */  {0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1},
    /* LED 4 */  {0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0},
    /* LED 5 */  {0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0},
    /* LED 6 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1}
  },

  { // Name: Variation 02 (3)
    /* Uhrzeit    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 */
    /* LED 1 */  {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1},
    /* LED 2 */  {0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0},
    /* LED 3 */  {0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0},
    /* LED 4 */  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 2, 2},
    /* LED 5 */  {0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0},
    /* LED 6 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0}
  },

  { // Name: Tag und Nacht (4)
    /* Uhrzeit    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 */
    /* LED 1 */  {0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1},
    /* LED 2 */  {0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1},
    /* LED 3 */  {0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0},
    /* LED 4 */  {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
    /* LED 5 */  {0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0},
    /* LED 6 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0}
  },


  { // Name: Lauflicht (5)
    /* Uhrzeit    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 */
    /* LED 1 */  {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    /* LED 2 */  {0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1},
    /* LED 3 */  {0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
    /* LED 4 */  {0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0},
    /* LED 5 */  {0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
    /* LED 6 */  {0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0}
  },

  { // Name: Full (6)
    /* Uhrzeit    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 */
    /* LED 1 */  {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 4, 0, 0, 6, 0, 0},
    /* LED 2 */  {0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 5, 1, 0, 6, 0, 0},
    /* LED 3 */  {0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 6, 4, 7, 0, 0, 0},
    /* LED 4 */  {0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 7, 4, 7, 0, 2, 0},
    /* LED 5 */  {0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 0, 0},
    /* LED 6 */  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 3, 0, 0, 0, 0, 0}
  }
};


byte current_room = 0;
byte current_hour = STARTUP_TIME;
byte current_quarter = 1; // keine 0
bool isHourStepChanged = false;
bool isHalfHourStepChanged = false;
bool isQuarterStepChanged = false;


bool isAutoRoomSelectionMode = false;
byte prgBehaviour = PRG_MODE_HOUR; // Default Stundenmodus

// Zeitmessung fuer den Lichtwechsel
unsigned long lastMillies = 0;
unsigned long lastQuarterMillies = 0;

// Blinkfunktion
bool doBlink = false;
bool blinkState = false;
byte currentBlinkTimes = 0;
byte blinkCount = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated

// Buttons
Bounce prgBtn = Bounce();
Bounce autoBtn = Bounce();

// ------------
void setup() {
  lastMillies = millis();
  isHourStepChanged = true;

  // Setup the button
  pinMode( BTN_1_PIN , INPUT);
  pinMode( BTN_2_PIN , INPUT);

  // Activate internal pull-up (optional)
  digitalWrite( BTN_1_PIN , HIGH);
  digitalWrite( BTN_2_PIN , HIGH);

  prgBtn.attach(BTN_1_PIN);
  prgBtn.interval(5);

  autoBtn.attach(BTN_2_PIN);
  autoBtn.interval(5);


  // Read config data
  EEPROM.get(ADRROM_ROOM, current_room);
  if (current_room == 255)
  {
    current_room = 0;
  }

  EEPROM.get(ADRROM_PRG_BEHAVIOUR, prgBehaviour);
  if (prgBehaviour > PRG_MODE_QUARTER_AUTO)
  {
    prgBehaviour = PRG_MODE_HOUR;
  }


  // Serial.begin(9600);
  // SHOW_DEBUG_INIT();

  if ((prgBehaviour == PRG_MODE_QUARTER) || (prgBehaviour == PRG_MODE_QUARTER_AUTO))
  {
    isQuarterStepChanged = true;
  }
  else
  {
    isHourStepChanged = true;
  }
}


void loop() {

  handleButtons();

  processLEDsBlink();

  calculateCurrentTime();

  processAutoRoomSelection();

  processRoomLED();

  charlie.loop();

}


void handleButtons() {

  // When blinking, no buttonning is possible
  if (doBlink)
  {
    return;
  }

  prgBtn.update();
  autoBtn.update();

  // Programmselektion
  if (prgBtn.fell())
  {
    current_room++;
    if (current_room >= ROOM_COUNT) {
      current_room = 0;
    }

    //Serial.print("current_room: ");
    //Serial.println(current_room + 1);

    // Save current room
    EEPROM.put(ADRROM_ROOM, current_room);
    delay(100);

    // Optische Bestätigung einleiten
    blinkLED(current_room + 1);

    return;
  }


  // Automatische Programmwahl im Kreis
  if (autoBtn.fell()) {

    // Funktion
    prgBehaviour++;
    if (prgBehaviour > PRG_MODE_QUARTER_AUTO) {
      prgBehaviour = PRG_MODE_HOUR;
    }

    isAutoRoomSelectionMode = (prgBehaviour == PRG_MODE_HOUR_AUTO) || (prgBehaviour == PRG_MODE_QUARTER_AUTO);

    /*
      Serial.print("prgBehaviour: ");
      Serial.println(prgBehaviour);
      Serial.print("isAutoRoomSelectionMode: ");
      Serial.println(isAutoRoomSelectionMode);
    */

    // Save current (new)
    EEPROM.put(ADRROM_PRG_BEHAVIOUR, prgBehaviour);
    delay(100);

    // Optische Bestätigung
    blinkLED(prgBehaviour + 1);
  }
}


// Init blink <times> of LED <ledNumber> for the next
void blinkLED(byte times) {
  if (!doBlink) // no setting for blinkmode possible
  {
    doBlink = true;
    blinkCount = times;
    allLedsOff(); // Init them to off
  }
}



void processLEDsBlink() {

  if (!doBlink) {
    return;
  }


  if (blinkState == false && (currentBlinkTimes >= blinkCount))
  {
    doBlink = false;
    blinkState = false;
    currentBlinkTimes = 0;
    blinkCount = 0;
    return;
  }


  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= BLINK_INTERVAL) {

    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (blinkState == false)
    {
      blinkState = true;
      currentBlinkTimes++;
    }
    else
    {
      blinkState = false;
    }

    allLeds(blinkState);
  }
}


void processRoomLED() {

  if ((prgBehaviour == PRG_MODE_HOUR) || (prgBehaviour == PRG_MODE_HOUR_AUTO)) {
    if (isHourStepChanged)
    {
      for (int iLed = 0; iLed < 6; iLed++)
      {

        byte lvalue = (byte)pgm_read_byte(&(rooms[current_room][iLed][current_hour]));

        charlie.setLed(iLed, lvalue > 0); // 0 und 1

        // SHOW_DEBUG_HOUR_CALC(iLed, lvalue, lvalue > 0);

      }

      isHourStepChanged = false;
    }
  }
  else if ((prgBehaviour == PRG_MODE_QUARTER) || (prgBehaviour == PRG_MODE_QUARTER_AUTO)) {
    {
      if (isQuarterStepChanged)
      {
        // im Viertelstunden-Raster muss gerechnet werden:
        // 1 volle Stunde
        // 2 erste halbe Stunde
        // 3 zweite halbe Stunde
        // 4-7 erste bis vierte Viertelstunde

        for (int iLed = 0; iLed < 6; iLed++) {

          bool state = false;
          byte lvalue = (byte)pgm_read_byte(&(rooms[current_room][iLed][current_hour]));

          if (lvalue > 0)
          {
            switch (lvalue)
            {
              case 1:
                state = true;
                break;
              case 2:
                state = (current_quarter == 1) || (current_quarter == 2);
                break;
              case 3:
                state = (current_quarter == 3) || (current_quarter == 4);
                break;
              case 4:
              case 5:
              case 6:
              case 7:
                state = current_quarter == lvalue - 4;
                break;
            }
          }

          charlie.setLed(iLed, state);

          //SHOW_DEBUG_HOUR_CALC(iLed, lvalue, state);

          isQuarterStepChanged = false;
        }
      }
    }

  }
}


void processAutoRoomSelection() {

  // Automatische Programmselektion nach jedem Durchlauf
  if (isAutoRoomSelectionMode && current_hour > 23) {
    current_room++;
    if (current_room >= ROOM_COUNT) {
      current_room = 0;
    }
  }
}


// 1000ms in H0 ==> 11,494 ms Echt
// 1sec in H0   ==> 11,494 ms Echt
// 10sec in H0  ==> 114,94 ms Echt
// 60sec in H0  ==> 689,655 ms Echt
// 1h in H0     ==> 41379,3103 ms Echt
// 1:87 => 41.379,3103
// Korrekturwert: pro 1200sec H0 + 1sec H0
byte calculateCurrentTime()
{
  int currentMillies = millis();

  if ((prgBehaviour == PRG_MODE_HOUR) || (prgBehaviour == PRG_MODE_HOUR_AUTO)) {
    if (currentMillies - lastMillies >= 41413) // H0-Stunde 41379 + Korrekturfaktor 34  // 10353 ist Viertelstundenraster
    {
      updateHour();
      isHourStepChanged = true;
      lastMillies = currentMillies;

      // SHOW_DEBUG_HOUR();
    }

  } else if ((prgBehaviour == PRG_MODE_QUARTER) || (prgBehaviour == PRG_MODE_QUARTER_AUTO)) {
    if (currentMillies - lastQuarterMillies >= 10353) // H0-Viertelstunde 10353 (Mit Korrekturfaktor)
    {
      if (updateQuarter() == 1) {
        updateHour();
      }
      isQuarterStepChanged = true;
      lastQuarterMillies = currentMillies;

      // Serial.println("PRG_MODE_QUARTER");
      // SHOW_DEBUG_HOUR();
    }
  }

  return current_hour;
}

/*

  void SHOW_DEBUG_INIT() {
  // Show current state
  Serial.println("");
  Serial.println("*************************");

  Serial.print("INIT Current Room: ");
  Serial.println(current_room + 1);
  Serial.print("INIT Current Hour: ");
  Serial.print(current_hour);
  Serial.println(":00");

  Serial.print("INIT prgBehaviour: ");
  Serial.println(prgBehaviour);
  Serial.println("*************************");
  }

  void SHOW_DEBUG_HOUR_CALC(byte iLed, byte lvalue, boolean state)
  {

  Serial.println();
  Serial.print("Room: "); Serial.print(current_room + 1);
  Serial.print("  LED: "); Serial.print(iLed);
  Serial.print("  Hour: "); Serial.print(current_hour);
  Serial.print(":");

  if ((current_quarter - 1) * 15 == 0) // (1-1) *15 = 0; (2-1)*15 = 15, (3-1)*15 = 30, (4-1)*15 = 45
  {
    Serial.print("00x");
  }
  else
  {
    Serial.print((current_quarter - 1) * 15);
  }

  Serial.print(" (current_quarter: "); Serial.print(current_quarter);
  Serial.print(") =>" ); Serial.print("  Value: ");
  Serial.print(lvalue);
  if (state) {
    Serial.print("  ("); Serial.print(state); Serial.print(")");
  }
  }


  void SHOW_DEBUG_HOUR() {
  Serial.println();
  Serial.print("Current Room: "); Serial.println(current_room + 1);
  Serial.print("Current Hour: "); Serial.print(current_hour);
  Serial.print(":");
  if ((current_quarter - 1) * 15 == 0) // (1-1) *15 = 0; (2-1)*15 = 15, (3-1)*15 = 30, (4-1)*15 = 45
  {
    Serial.print("00x");
  }
  else
  {
    Serial.print((current_quarter - 1) * 15);
  }
  Serial.print(" (current_quarter: "); Serial.print(current_quarter); Serial.print(")");
  Serial.println();
  }


*/

byte updateQuarter() {
  current_quarter++;
  if (current_quarter > 4) {
    current_quarter = 1;
  }

  return current_quarter;
}

byte updateHour() {
  current_hour++;
  if (current_hour > 23) {
    current_hour = 0;
  }

  return current_hour;
}

void allLeds(bool state) {
  charlie.setLed(0, state);    // Turn on LED1
  charlie.setLed(1, state);    // Turn on LED2
  charlie.setLed(2, state);    // Turn on LED3
  charlie.setLed(3, state);    // Turn on LED4
  charlie.setLed(4, state);    // Turn on LED5
  charlie.setLed(5, state);    // Turn on LED6
}

void allLedsOn() {
  allLeds(true);
}

void allLedsOff() {
  allLeds(false);
}

