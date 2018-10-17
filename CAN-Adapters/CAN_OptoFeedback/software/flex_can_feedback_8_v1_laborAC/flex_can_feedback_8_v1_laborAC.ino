 #include <EEPROM.h>
#include <RailuinoExt.h>
#include <InputDebounce.h>
#include "candefines.h"
#include "flex_can_feedback.h"
#include "configuration.h"



// Enable global debug mode, which writes to the serial console // only on Arduino, not standalone
#define DEBUG 0

// ---------------------------------
// CAN
// ---------------------------------
TrackController ctrl;
TrackMessage message;

CanDevice device; // [MäCAN]

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


// ---------------------------------
// Code block
// ---------------------------------
/**
   Extracts data from a message and does
   it filter for S88 feedback messages
*/
boolean handleMessageData(TrackMessage &message)
{
  ////////////////////////////////
  // Wenn der Setup-Mode eingeschaltet ist und ein
  // ACC-Kommando kommt, dann die Adresse lesen und speichern.
  if (isSetup
      && !message.response
      && message.command == CAN_CMD_ACC
      && setupCurrentSelectedPortNr < (MAX_FEEDBACK_OUTPUT_COUNT + 1)) // Weichen etc.
  {

    word addr = word(message.data[LOCID_BYTE_3], message.data[LOCID_BYTE_4]);

    // Ab der gesendeten Adresse werden im gesetzten Port "0" (aka Mode 0)
    // alle Ports inkrementierend gesetzt
    // Also: Gesendete Adresse=20 => 1, 21 => 2 usw.
    if (setupCurrentSelectedPortNr == 0)
    {
      for (int i = 0; i < 8; i++)
      {
        // Wir nehmen jede Adresse, auch eine DCC ist möglich.
        addressList[i] = addr + i;
      }

      saveCurrentSetupAndFinish(); // Das Setup wird sofort gespeichert
    }
    else  // Hier werden normal Port 1-8 je nach gewaehlter Portnummer eingestellt
    {
      // Wir nehmen jede Adresse, auch eine DCC ist möglich.
      addressList[setupCurrentSelectedPortNr - 1] = addr;
      confirmCommandLED(3);
    }

    message.clear();

    return true;
  }
  else
  {
    ////////////////////////////////
    // Pruefen, ob die letzte Statusmeldung angekommen ist
    // bzw. alternativ eine andere Stellung eingenommen wurde
    if (!message.response
        && message.command == CAN_CMD_ACC // Weichen etc.
        && message.data[CAN_CMD_ACC_POWER] == 0x00
       )
    {
#if DEBUG
      Serial.println();
      Serial.print("[CAN] #1  ");
      Serial.println(message);
#endif
    }
    /// Die CS2 und die cs2.exe hat die Geräteid 0xeeee.
    ////////////////////////////////
    // Check for a sent PING
    else if (
      message.length == 0
      && message.command == CAN_CMD_PING
    )
    {
      if (pingCounter > MAX_PING_COUNT)
      {
        // Responds with message
        pingResponse(device);

        // and responds with current data
        initialReadRegisterPorts();

        pingCounter = 0;
      }
      else
      {
        pingCounter++;
      }
    }
    else if (
      message.length == 8
      && message.command == CAN_CMD_PING
      && message.data[DATA_BYTE_7] == 0xff
      && message.data[DATA_BYTE_8] == 0xff
    )
    {
#if DEBUG
      Serial.print("[CAN] PING  ");
      Serial.println(message);
#endif

      // Responds with message
      pingResponse(device);

      // and responds with current data
      initialReadRegisterPorts();
    }

    ////////////////////////////////
    // Check for a message response for a accessory command message
    else if (message.response
             && message.command == CAN_CMD_ACC // Weichen etc.
            )
    {
#if DEBUG
      Serial.print("[CAN] #2  ");
      Serial.println(message);
#endif
    }

    ////////////////////////////////
    // Check for a  sent accessory command
    else if (!message.response
             && message.command == CAN_CMD_ACC // Weichen etc.
            )
    {
#if DEBUG
      Serial.print("[CAN] #3  ");
      Serial.println(message);
#endif
    }

    message.clear();

    return true;
  }

  return false;
}


/**
   Checks the port with the given index (one of eight) for its current state
*/
void readCurrentPortState(byte idx)
{
  // Im Setup-Mode kucken wir nicht nach.
  if (isSetup)
  {
    return;
  }

  bool inputData = digitalRead(portProcessors[idx].input);
  unsigned long currentMillis = millis();

  ////////////////////////////////
  // Statusdetektion

  // Input is HIGH
  if (inputData == true 
        && portProcessors[idx].state == false 
        && portProcessors[idx].stateDebounceCounter + 1 == EVENT_WAIT_COUNT)  // is true
  {
    portProcessors[idx].state = true;
    portProcessors[idx].lastMillis = currentMillis;
    portProcessors[idx].stateDebounceCounter = 0;
  }
  else if(inputData == true 
        && portProcessors[idx].state == false 
        && portProcessors[idx].stateDebounceCounter + 1 < EVENT_WAIT_COUNT)
  {
    portProcessors[idx].stateDebounceCounter++;
    return;
  }

  ////////////////////////////////
  // Oder Wechsel zu LOW
  else if (inputData == false && portProcessors[idx].state == true
           && (currentMillis - portProcessors[idx].lastMillis) > millisIntervall)
  {
    portProcessors[idx].state = false;
    portProcessors[idx].lastMillis = currentMillis;
    portProcessors[idx].stateDebounceCounter = 0;
  }
  else
  {
    portProcessors[idx].stateDebounceCounter = 0;
    return;
  }


  ////////////////////////////////
  // Writes state of port to the port state register
  if (portProcessors[idx].lastState != portProcessors[idx].state)
  {
    // set current value to register
    bitWrite(portStateRegister, idx, portProcessors[idx].state);
    portProcessors[idx].lastState = portProcessors[idx].state;

    prepareStateMessage(idx);
  }

  // if an output is defined, set its state. Could be a LED.
  if (portProcessors[idx].output > 0)
  {
    showInputPortState(idx);
  }
}

/**
   Depding of the port selection switch this will show the selected
   port state with the setup LED
*/
void showInputPortState(byte idx)
{
  bool ledState = false;
  byte selectedPin = getSelectedPortNumber();

  if (selectedPin > 0
      && selectedPin < 9
      && idx == selectedPin - 1) // wg. Nullbasierten Arrayindexen 1 weniger!
  {
    ledState = portProcessors[idx].state;
  }

  digitalWrite(portProcessors[idx].output, ledState);

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

/*
   Is sending a CAN message depending on the send mode ACC or S88
*/
void prepareStateMessage(int idx)
{
  if (isCanModeS88)
  {
    sendS88State(getAddressByTrackFormat(addressList[idx]), bitRead(portStateRegister, idx));
  }
  else
  {
    // Nachricht senden
    sendAccState(addressList[idx], bitRead(portStateRegister, idx), false, false);  // Request
    sendAccState(addressList[idx], bitRead(portStateRegister, idx), true, false);   // Response

    // CDB
    sendAccState(addressList[idx], bitRead(portStateRegister, idx), true, true);   // Sending cdb

    // maerklin
    ctrl.setTurnout(addressList[idx], bitRead(portStateRegister, idx));           // send explizit turnout message

    // s88 for those who want it
    sendS88State(getAddressByTrackFormat(addressList[idx]), bitRead(portStateRegister, idx));  // S88
  }
}

/*
   Sending a s88 CAN message
*/
void sendS88State(word addr, byte state)
{
  TrackMessage message;
  TrackMessage messagein;
  message.command = CAN_CMD_S88EVENT;
  message.length = 8;
  message.response = true;

  message.data[LOCID_BYTE_1] = 0x0;
  message.data[LOCID_BYTE_2] = 0x0;
  message.data[LOCID_BYTE_3] = highByte(addr);
  message.data[LOCID_BYTE_4] = lowByte(addr);


  message.data[CAN_S88_LAST]    = !state;  // Last State
  message.data[CAN_S88_CURRENT] = state;   // Current State

  message.data[DATA_BYTE_7] = 0x0;
  message.data[DATA_BYTE_8] = 0x0;

  ctrl.sendMessage(message);
}

/**
   Sending turnout position Message into CAN-Bus
*/
void sendAccState(word addr, byte state, bool response, bool cdb)
{
  TrackMessage message;
  message.command = CAN_CMD_ACC;
  message.length = 6;
  message.data[LOCID_BYTE_1] = 0x0;
  message.data[LOCID_BYTE_2] = 0x0;
  message.data[LOCID_BYTE_3] = highByte(addr);  // implizit mit DCC oder MM2
  message.data[LOCID_BYTE_4] = lowByte(addr);

  byte stateValue = state == true ? CAN_POSITION_CURVE : CAN_POSITION_STRAIGHT;
  message.data[CAN_CMD_ACC_POSITION] = stateValue;           // current state of port
  message.data[CAN_CMD_ACC_POWER] = 0x00;                     // Power off, only state

  if (cdb)
  {
    /* Meldung für CdB-Module und Rocrail Feldereignisse. Aus MäCan geklaut */
    message.data[CAN_CMD_ACC_POSITION] = CAN_CDB_POSITION_VALUE - stateValue;
  }
  else
  {
    // RocRail does send it
    //message.length = 8;
    //message.data[6] = 0;
    //message.data[7] = 0x19;
  }

  message.response = response;
  ctrl.sendMessage(message);

#if DEBUG
  Serial.print(F("Message: ")); Serial.println(message);
#endif

}


// Ported from maecan project
void sendPingFrame(CanDevice device, bool response)
{
  TrackMessage message;

  message.command = CAN_CMD_PING;
  message.hash = device.hash;
  message.response = response;
  message.length = 8;
  message.data[0] = device.uid >> 24;
  message.data[1] = device.uid >> 16;
  message.data[2] = device.uid >> 8;
  message.data[3] = device.uid;
  message.data[4] = device.versHigh;
  message.data[5] = device.versLow;
  message.data[6] = device.type >> 8;
  message.data[7] = device.type;

  ctrl.sendMessage(message);

}

void pingResponse(CanDevice device) {
  sendPingFrame(device, true);
}


/*************************************************************
   Misc
 *************************************************************/
/**
   Reads the ports of the BCD-Switch and
   calculates the selected value
*/
byte getSelectedPortNumber()
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
    delay(250);
  }

  digitalWrite(LED_SETUP, isSetup); // depends on setup state
}


/*
   Setup for the port processor structure
*/
void initPortProcessorData(byte idx, byte input, byte output, unsigned long lastMillis)
{
  portProcessors[idx].state = bitRead(portStateRegister, idx) ? true : false;
  portProcessors[idx].input = input;
  portProcessors[idx].output = output;
  portProcessors[idx].lastState = false;
  portProcessors[idx].lastMillis = lastMillis;
  portProcessors[idx].stateDebounceCounter = 0;

  pinMode(portProcessors[idx].input, INPUT);

  if (portProcessors[idx].output > 0)
  {
    pinMode(portProcessors[idx].output, OUTPUT);
  }
}



/*************************************************************
   Callbacks for buttons
 *************************************************************/
void progBtn_pressedDurationCallback(unsigned long duration)
{
  if (duration > 5000 && isSetup == false)
  {
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
    digitalWrite(LED_SETUP, false);
  }
  else if (duration > 5000 && isSetup == true)
  {
    // disable setup and write the setup
    saveCurrentSetupAndFinish();
  }
}

/*
   Saves the current setup params and finishes the setup mode
*/
void saveCurrentSetupAndFinish()
{
  confirmCommandLED(1);
  eepromSetupState = 1;
  saveConfiguration();
  isSetup = false;
  confirmCommandLED(3);
  delay(20);
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

  // handles the selector for S88 vs. ACC message mode
  pinMode(SELECT_MODE_S88_SWITCH, INPUT_PULLUP);
  if (digitalRead(SELECT_MODE_S88_SWITCH) == false)
  {
    isCanModeS88 = true;
  }

  // Setting inputs
  long lastMillis = millis();
  for (int idx = 0; idx < MAX_FEEDBACK_OUTPUT_COUNT; idx++)
  {
    initPortProcessorData(idx, portList[idx], outList[idx], lastMillis);
  }

  // Berechne interval aus geg. Frequenz, je hoeher, desto kleiner das interval
  millisIntervall = 1000 / FREQUENZ;

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
  // Geräteinformationen:
  device.versHigh = VERS_HIGH;
  device.versLow = VERS_LOW;
  device.hash = generateHash(UID);
  device.uid = UID;
  device.artNum = DEVICE_ARTNUM;
  device.name = DEVICE_NAME;
  device.boardNum = BOARD_NUM;
  device.type = DEVICE_TYPE;

  // Start controller
  ctrl = TrackController(device.hash, false);
  ctrl.begin();
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
    setupCurrentSelectedPortNr = getSelectedPortNumber();
#if DEBUG
    if (setupCurrentSelectedPortNr != setupLastSelectedPortNr)
    {
      Serial.println(F("--------------------------------------------------"));
      Serial.print(  F("New selected port: ")); Serial.println( setupCurrentSelectedPortNr);
      setupLastSelectedPortNr = setupCurrentSelectedPortNr;
    }
#endif
  }

  // Read the current port states
  for (int idx = 0; idx < MAX_FEEDBACK_OUTPUT_COUNT; idx++)
  {
    readCurrentPortState(idx);
  }

  // poll button state
  progBtn.process(millis()); // callbacks called in context of this function

  // Delay
  delay(20);
}

