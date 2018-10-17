// ---------------------------------
// CAN defines
// Parts are from jmri.jmrix.marklin.MarklinConstants.java
// ---------------------------------


// CAN Hash
// #define HASH 0xdf24  // Standard Railuino
// #define HASH 0x4d48

// Locations within the UDP packet of the address bytes
#define MODULE_BYTE_HIGH 0x00
#define MODULE_BYTE_LOW 0x01
#define CONTACT_BYTE_HIGH 0x02
#define CONTACT_BYTE_LOW 0x03

#define LOCID_BYTE_1 0
#define LOCID_BYTE_2 1
#define LOCID_BYTE_3 2
#define LOCID_BYTE_4 3


#define DATA_BYTE_1 0
#define DATA_BYTE_2 1
#define DATA_BYTE_3 2
#define DATA_BYTE_4 3
#define DATA_BYTE_5 4
#define DATA_BYTE_6 5
#define DATA_BYTE_7 6
#define DATA_BYTE_8 7

#define CAN_S88_LAST 4
#define CAN_S88_CURRENT 5


// CAN commands
#define CAN_CMD_S88EVENT 0x11
#define CAN_CMD_ACC 0x0b          // Used for switches or signals

#define CAN_CMD_PING 0x18
#define CAN_CMD_ACC_POSITION 0x04
#define CAN_CMD_ACC_POWER 0x05
#define CAN_CMD_ACC_FUNCTIONVALUE_H 0x06
#define CAN_CMD_ACC_FUNCTIONVALUE_L 0x07

#define CAN_POSITION_STRAIGHT 0x01
#define CAN_POSITION_CURVE 0x00

#define CAN_CDB_POSITION_VALUE 0xfe


//MM1 2 accessories article decoder (40 kHz, 320 & 1024 addresses)
#define MM1ACCSTART 0x3000
#define MM1ACCEND 0x33FF

// DCC accessories article decoder
#define DCCACCSTART 0x3800
#define DCCACCEND 0x3FFF

// MM1 Locomotive
#define MM1START 0x0000
#define MM1END 0x03FF

// MM2 Locomotives
#define MM2START 0x2000
#define MM2END 0x23FF

// MFX Locomotives
#define MFXSTART 0x4000
#define MFXEND 0x7FFF

// SX2 Locomotives
#define SX2START 0x8000
#define SX2END 0xBFFF

// DCC Locomotives
#define DCCSTART 0xC000
#define DCCEND 0xFFFF


// MäCAN
/*
 *  Gerätetypen (von MäCAN):
 *  Dienen nur zur Unterscheidung beim Ping, hat keine Auswirkungen auf den Betrieb.
 */
#define CORAX_MAGNET    0x0060
#define CORAX_SERVO     0x0061
#define CORAX_RELAIS    0x0062
#define CORAX_STELLPULT 0x0063
#define CORAX_S88_GBS   0x0064
#define CORAX_S88_RM    0x0065

typedef struct {
  uint8_t versHigh;   //Versionsnummer vor dem Punkt
  uint8_t versLow;    //Versionsnummer nach dem Punkt
  String name;        //Name des Geräts
  String artNum;      //Artikelnummer des Geräts
  int boardNum;       //Nummer des Geräts
  uint16_t hash;      //Hash des Geräts (muss vor her mit generateHash() berechnet werden)
  uint32_t uid;       //UID des Geräts
  uint16_t type;      //Typ des Geräts (z.B. MäCAN Magnetartikeldecoder: 0x0050)
} CanDevice;


// From MäCAN
uint16_t generateHash(uint32_t uid)
{

  uint16_t highbyte = uid >> 16;
  uint16_t lowbyte = uid;
  uint16_t hash = highbyte ^ lowbyte;
  bitWrite(hash, 7, 0);
  bitWrite(hash, 8, 1);
  bitWrite(hash, 9, 1);

  return hash;
}


/**
   Returns the Trackformat
   Value   Protokoll
    0     MM2
    1     MFX
    2     DCC
    255   UNKNOWN
*/
byte getTrackFormat(word data)
{
  if (data >= DCCACCSTART && data <= DCCACCEND)
  {
    return 2;
  }
  else if (data >= MM1ACCSTART && data <= MM1ACCEND)
  {
    return 0;
  }
  else if (data >= MFXSTART && data <= MFXEND)
  {
    return 1;
  }
  else if (data >= DCCSTART && data <= DCCEND)
  {
    return 2;
  }
  else if (data >= MM2START && data <= MM2END)
  {
    return 0;
  }

  return 255;
}

/**
   Returns the address undependend from Trackformat
*/
word getAddressByTrackFormat(word addr)
{
  int minus = 0;

  if (addr >= DCCACCSTART && addr <= DCCACCEND)
  {
    minus = DCCACCSTART - 1;
  }
  else if (addr >= MM1ACCSTART && addr <= MM1ACCEND)
  {
    minus = MM1ACCSTART - 1;
  }

  return addr - minus;
}

