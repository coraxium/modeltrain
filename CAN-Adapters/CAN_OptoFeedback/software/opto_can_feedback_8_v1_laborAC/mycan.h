// ---------------------------------
// CAN defines
// Parts from jmri.jmrix.marklin.MarklinConstants.java
// ---------------------------------

// Location within the UDP packet of the address bytes
#define MODULE_BYTE_HIGH 0x00
#define MODULE_BYTE_LOW 0x01
#define CONTACT_BYTE_HIGH 0x02
#define CONTACT_BYTE_LOW 0x03

#define LOCID_BYTE_1 0
#define LOCID_BYTE_2 1
#define LOCID_BYTE_3 2
#define LOCID_BYTE_4 3

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

// CAN Hash
#define HASH 0xdf24
//#define HASH 0x4d48


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

