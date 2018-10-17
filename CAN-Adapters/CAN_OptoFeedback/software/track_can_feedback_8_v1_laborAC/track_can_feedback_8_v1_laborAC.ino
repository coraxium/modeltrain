#include <EEPROM.h>
#include <Railuino.h>
#include <InputDebounce.h>
#include "StateMachine.h"
#include "mycan.h"
#include "opto_can_feedback.h"
#include "configuration.h"


// Enable global debug mode, which writes to the serial console
#define DEBUG 1

// ---------------------------------
// CAN
// ---------------------------------
TrackController ctrl(HASH, false);
TrackMessage message;


// ---------------------------------
// Contacts
// ---------------------------------
volatile byte setupCurrentSelectedPortNr = 0;
#if DEBUG
volatile byte setupLastSelectedPortNr = 255;   // used for comparing, if a portnr newly selected by switch
#endif



// ---------------------------------
// Debouncing
// ---------------------------------
// Debouncing progBtn
static InputDebounce progBtn;


// Forwards
byte getTrackFormat(word data);
word getAddressByTrackFormat(word data);
void confirmCommandLED(byte data);
void sendState(word addr, byte state);

/**
   Extracts data from a message and does
   it filter for S88 feedback messages
*/
boolean handleMessageData(TrackMessage &message)
{
  // Wenn der Setup-Mode eingeschaltet ist und ein
  // ACC-Kommando kommt, dann die Adresse lesen und speichern.
  if (isSetup
      && !message.response
      && message.command == CAN_CMD_ACC // wir programmieren mit Weichenadressen
      && setupCurrentSelectedPortNr > 0
      && setupCurrentSelectedPortNr < (MAX_FEEDBACK_OUTPUT_COUNT + 1)) // Weichen etc.
  {
    // Wir nehmen jede Adresse, auch eine DCC ist möglich.
    word addr = word(message.data[LOCID_BYTE_3], message.data[LOCID_BYTE_4]);
    addressList[setupCurrentSelectedPortNr - 1] = addr;

    confirmCommandLED(3);

    message.clear();

    return true;
  }
  else
  {
    // Pruefen, ob die letzte Statusmeldung angekommen ist
    // bzw. alternativ eine andere Stellung eingenommen wurde
    if (message.response
        && message.command == CAN_CMD_S88EVENT // S88-Eventantwort
        && message.length == 8  // DLC==8, dann ist es eine Response
       )
    {
      Serial.println();
      Serial.print("[CAN] #1  ");
      Serial.println(message);
    }
    */
    else if (message.response
             && message.command == CAN_CMD_ACC // Weichen etc.
            )
    {
      Serial.print("[CAN] #2  ");
      Serial.println(message);
    }
    else if (!message.response
             && message.command == CAN_CMD_ACC // Weichen etc.
            )
    {
      Serial.print("[CAN] #3  ");
      Serial.println(message);
    }
    */

    message.clear();

    return true;
  }

  return false;
}



void processAllPorts()
{
  for (int idx = 0; idx < MAX_FEEDBACK_OUTPUT_COUNT; idx++)
  {
    portProcessors[idx].process();
    if (portProcessors[idx].hasChanged())
    {
      // set current value to register
      bitWrite(portStateRegister, idx, portProcessors[idx].getState());

      // Send state message
      //prepareStateMessage(idx);

      Serial.print("[STATE] P");
      Serial.print(idx);
      Serial.print(" Statechange: ");
      Serial.println(portProcessors[idx].getState());

      // save into EEPROM
      EEPROM.update(EEPROM_STATE_REGISTER, portStateRegister);
    }
  }
}


/**
   Reads the data from register for every configured port
*/
void initialReadRegisterPorts()
{
  for (int idx = 0; idx < MAX_FEEDBACK_OUTPUT_COUNT; idx++)
  {
    prepareStateMessage(idx);
  }
}

void prepareStateMessage(int idx)
{
  // Nachricht senden
  sendState(addressList[idx], bitRead(portStateRegister, idx), false, false);
  sendState(addressList[idx], bitRead(portStateRegister, idx), true, false);

  // RESPONSE!
  //delay(20);
  //sendState(addressList[idx], bitRead(portStateRegister, idx), true, false);

  // CDB
  //delay(20);
  sendState(addressList[idx], bitRead(portStateRegister, idx), true, false);

  // maerklin
  ctrl.setTurnout(addressList[idx], bitRead(portStateRegister, idx));
}


/**
   Sending s88 Message into CAN-Bus

<== 0b04 R 11 8 00 00 00 19 00 01 00 00

<== 0b04 R 11 8 00 00 00 19 01 00 00 00   

*/
void sendState(word addr, byte state, bool response, bool cdb)
{
  // TODO: empty adrr. are not for sending
  TrackMessage message;
  message.command = CAN_CMD_S88EVENT;
  message.length = 5;
  message.data[LOCID_BYTE_1] = 0x0;
  message.data[LOCID_BYTE_2] = 0x0;
  message.data[LOCID_BYTE_3] = highByte(addr);  // Modul
  message.data[LOCID_BYTE_4] = lowByte(addr);   // Melderadresse

  message.data[CAN_S88_OLD] = !state;
  message.data[CAN_S88_NEW] = state;

  message.response = true;

  ctrl.sendMessage(message);

#if DEBUG
  Serial.print(F("Message: ")); Serial.println(message);
#endif

}



/*************************************************************
   Misc
 *************************************************************/


/**
   Reads the ports of the BCD-Switch and
   calculates the selected value
*/
byte checkSetupPortNrPins()
{
  // 1,2,4,8
  byte value =
    (!digitalRead(PORTNR_PIN_1)) * 1
    + (!digitalRead(PORTNR_PIN_2)) * 2
    + (!digitalRead(PORTNR_PIN_4)) * 4
    + (!digitalRead(PORTNR_PIN_8)) * 8;

  return value;
}

/**
   Flashes a LED for confirmation
*/
void confirmCommandLED(byte times)
{
  digitalWrite(LED_SETUP, LOW);

  for (int i = 0; i < times; i++)
  {
    digitalWrite(LED_SETUP, HIGH);
    delay(150);
    digitalWrite(LED_SETUP, LOW);
    delay(400);
  }

  digitalWrite(LED_SETUP, HIGH);
}




/*************************************************************
   Callbacks for buttons
 *************************************************************/

void progBtn_pressedDurationCallback(unsigned long duration)
{
  if (duration > 5000 && isSetup == false)
  {

#if DEBUG
    Serial.println(F("--------------------------------------------------"));
    Serial.println(F("Button pressed, Setup enabled"));
#endif
    // enable setup
    isSetup = true;
    confirmCommandLED(3);
  }
  else if (duration > 5000 && isSetup == true && setupCurrentSelectedPortNr == 9)
  {
    resetConfiguration();
    confirmCommandLED(3);
  }
  else if (duration > 5000  && isSetup == false && setupCurrentSelectedPortNr == 0)
  {
    // NOP
    // could be used for special functions, i.e. a trestrun or so
  }
  else if (duration > 5000 && isSetup == true)
  {
#if DEBUG
    Serial.println(F("--------------------------------------------------"));
    Serial.println(F("Button pressed, Setup DISABLED"));
#endif
    // disable setup and write the setup
    eepromSetupState = 1;
    confirmCommandLED(3);
    saveConfiguration();
    isSetup = false;
    digitalWrite(LED_SETUP, LOW);
    delay(20);
  }
}



/*************************************************************
   Setup
 *************************************************************/
void setup() {
#if defined(DEBUG)
#if DEBUG
  Serial.begin(115200);
  while (!Serial);
#endif
#endif

  // Setting LED
  pinMode(LED_SETUP, OUTPUT);

  // PrgAdrPins -- wg. Schaltungsfehler (gegen GND) negative Logik -- aber das ist evtl. nicht schlecht
  pinMode(PORTNR_PIN_1, INPUT_PULLUP);
  pinMode(PORTNR_PIN_2, INPUT_PULLUP);
  pinMode(PORTNR_PIN_4, INPUT_PULLUP);
  pinMode(PORTNR_PIN_8, INPUT_PULLUP);

  // Setting inputs
  pinMode(PORT_1, INPUT);
  pinMode(PORT_2, INPUT);
  pinMode(PORT_3, INPUT);
  pinMode(PORT_4, INPUT);
  //  pinMode(PORT_5, INPUT);
  //  pinMode(PORT_6, INPUT);
  //  pinMode(PORT_7, INPUT);
  //  pinMode(PORT_8, INPUT);


  // register callbacks
  progBtn.registerCallbacks(NULL, NULL,  progBtn_pressedDurationCallback);

  // setup input button (debounced)
  progBtn.setup(SW_PROGBTN, BUTTON_DEBOUNCE_DELAY, InputDebounce::PIM_INT_PULL_UP_RES);


  // Configuration section
  // reads the first byte of the EEPROM - if set to 1, a setup
  // is not needed, otherwise we will start the setup mode
  if (!isConfigured())
  {

#if DEBUG
    Serial.println(F("--------------------------------------------------"));
    Serial.println(F("Setupmode"));
    Serial.println(F("--------------------------------------------------"));
#endif

    setupConfiguration();
    confirmCommandLED(3);
  }
  else
  {
    readConfiguration();
  }

  // Start Railuino
  ctrl.begin();

  //// Read the register and send it into the bus
  // initialReadRegisterPorts();
}


/*************************************************************
   Eternal Loop
*************************************************************/
void loop() {

  // Sniff for a message
  ctrl.receiveMessage(message);

  // extracts data from Message and react to it
  handleMessageData(message);

  // configure Port with TrackAddress (MM2 or DCC)
  if (isSetup)
  {
    setupCurrentSelectedPortNr = checkSetupPortNrPins();
#if DEBUG
    if (setupCurrentSelectedPortNr != setupLastSelectedPortNr)
    {
      Serial.println(F("--------------------------------------------------"));
      Serial.print(  F("New selected port: ")); Serial.println( setupCurrentSelectedPortNr);
      setupLastSelectedPortNr = setupCurrentSelectedPortNr;
    }
#endif
  }

  // Check the port states
  processAllPorts();

  // poll button state
  progBtn.process(millis()); // callbacks called in context of this function

  // Delay
  delay(20);
}


