
/*************************************************************
   Configuration code
 *************************************************************/

// EEPROM: spaces for data, Startadresses
#define EEPROM_SETUP_STATE 0
#define EEPROM_MODULE_ID 1
#define EEPROM_DATA_MODE 2
#define EEPROM_STATE_REGISTER 3
#define EEPROM_ADDRESSES 10

byte eepromSetupState = 0;
byte eepromModuleId = 127;
byte eepromDataMode = 1;

volatile bool isSetup = false;


bool isConfigured()
{
  eepromSetupState = EEPROM.read(EEPROM_SETUP_STATE);
  return eepromSetupState == 1;
}

/**
   pre-init the storage array
*/
void initConfiguration()
{
  // inits array with default values
  for (int idx = 0; idx < MAX_FEEDBACK_OUTPUT_COUNT; idx++) {
    addressList[idx] = 0;
  }
}

void resetConfiguration()
{
  initConfiguration();  // pre-init storage array

  // Komplett der ganze beschriebene Bereich
  for (int idx = 0 ; idx < MAX_FEEDBACK_OUTPUT_COUNT + EEPROM_ADDRESSES; idx++) {
    EEPROM.update(idx, 0xff);
  }

  EEPROM.update(EEPROM_SETUP_STATE, 0xff);  // reset

  delay(20); // Waiting for write cycle
}

void showConfiguration()
{

#if DEBUG
  Serial.println(F("\n+++++++++++++\nshowConfiguration()\n+++++++++++++"));
  Serial.println(F("+-------------------------+"));
  Serial.println(F("|   Gespeicherte Daten    |"));
  Serial.println(F("|   Liste                 |"));
  Serial.println(F("+-------------------------+"));
#endif
  byte data[2] = {0, 0};


  // inits array with default values
  for (int index = 0; index < MAX_FEEDBACK_OUTPUT_COUNT; index++) {

    data[0] = highByte(addressList[index]);
    data[1] = lowByte(addressList[index]);

#if DEBUG
    Serial.print(F("Port #"));
    Serial.print(1 + index);
    Serial.print(F(" --> "));
    Serial.print(addressList[index]);
    Serial.print(F("   (0x"));
    Serial.print(data[0], HEX);
    Serial.print(F(" 0x"));
    Serial.print(data[1], HEX);


    Serial.print(F(")   "));
    byte adr = getAddressByTrackFormat(addressList[index]);
    byte form = getTrackFormat(addressList[index]);

    Serial.print(adr);
    if (form == 0) Serial.print(F(" [MM2] "));
    if (form == 1) Serial.print(F(" [MFX] "));
    if (form == 2) Serial.print(F(" [DCC] "));
    if (form > 2) Serial.print(F(" [unknown] "));

    Serial.println(F(""));
#endif
  }
}

void readConfiguration()
{
  byte index = 0;
  byte idx = 0;
  byte data[2] = {0, 0};

  initConfiguration();    // pre-init storage array

#if DEBUG
  Serial.println(F("+-------------------------+"));
  Serial.println(F("|   Gespeicherte Daten    |"));
  Serial.println(F("+-------------------------+"));
#endif

  // Adressliste lesen
  // Berechnung:
  //    EEPROM_ADDRESSES Startadresse
  //    8 Adressen, Int (2Bytes) ((MAX_FEEDBACK_OUTPUT_COUNT * 2) + EEPROM_ADDRESSES)
  for (index = EEPROM_ADDRESSES; index < ((MAX_FEEDBACK_OUTPUT_COUNT * 2) + EEPROM_ADDRESSES); index += 2)
  {
    // 2 Bytes werden gelesen
    data[0] = EEPROM.read(index);     // Highbyte
    data[1] = EEPROM.read(index + 1); // Lowbyte

    addressList[idx] = word(data[0], data[1]);

#if DEBUG
    Serial.print(F("Port #"));
    Serial.print(1 + (index - EEPROM_ADDRESSES ) / 2);
    Serial.print(F(" --> "));
    Serial.print(addressList[idx]);
    Serial.print(F(" ("));
    Serial.print(data[0], HEX);
    Serial.print(F(" "));
    Serial.print(data[1], HEX);
    Serial.println(F(")"));
#endif

    idx++;
  }

  // Jetzt die Bytes aus dem Config-Bereich lesen
  eepromSetupState = EEPROM.read(EEPROM_SETUP_STATE);  // 1 == TRUE => Konfiguration wurde geschrieben, kein Setup notwendig
  eepromModuleId = EEPROM.read(EEPROM_MODULE_ID);  // Modulid (later), default 127
  eepromDataMode = EEPROM.read(EEPROM_DATA_MODE);    // Modus MM oder DCC: MM => 1, DCC => 2

  // letzter Weichenrueckmelderstatus
  portStateRegister = EEPROM.read(EEPROM_STATE_REGISTER);
  

#if DEBUG
  Serial.println("");
  Serial.print(F("Is configured?  "));
  Serial.println(eepromSetupState == 1 ? "true" : "false");
  Serial.print(F("Modulid:        "));
  Serial.println(eepromModuleId);
  Serial.print(F("Datamode:       "));
  Serial.println(eepromDataMode < 3 && eepromDataMode == 1 ? "MM2" : "DCC");
#endif
}


void saveConfiguration()
{
  byte idx = 0;
  byte index = 0;
  byte data[2] = {0, 0};


  // Adressliste schreiben
  for (index = EEPROM_ADDRESSES; index < ((MAX_FEEDBACK_OUTPUT_COUNT * 2) + EEPROM_ADDRESSES); index += 2)
  {
    /*
      Button pressed, Setup DISABLED
      Port #1 --> 60929 (1 238)
      Port #2 --> 479 (223 1)
      Port #3 --> 25 (25 0)
      Port #4 --> 62215 (7 243)

      = 0f44   0b 6 00 00 30 09 00 00
      Adresse 10
    */

    data[0] = highByte(addressList[idx]);
    data[1] = lowByte(addressList[idx]);

    // Die 2 Bytes werden geschrieben
    EEPROM.update(index, data[0]);
    EEPROM.update(index + 1, data[1]);


#if DEBUG
    Serial.print(F("Port #"));
    Serial.print(1 + (index - EEPROM_ADDRESSES ) / 2);
    Serial.print(F(" --> "));
    Serial.print(addressList[idx]);
    Serial.print(F(" ("));
    Serial.print(data[0]);
    Serial.print(F(" "));
    Serial.print(data[1]);
    Serial.println(F(")"));
#endif
    idx++;
  }

  // Jetzt die weitere Konfiguration
  EEPROM.update(EEPROM_SETUP_STATE, eepromSetupState);  // 1 == TRUE => Konfiguration wurde geschrieben, kein Setup notwendig
  EEPROM.update(EEPROM_MODULE_ID, eepromModuleId);  // Modulid (later), default 127
  EEPROM.update(EEPROM_DATA_MODE, eepromDataMode);    // Modus MM oder DCC: MM => 1, DCC => 2
}



void setupConfiguration()
{
  readConfiguration();
  isSetup = true;
}

