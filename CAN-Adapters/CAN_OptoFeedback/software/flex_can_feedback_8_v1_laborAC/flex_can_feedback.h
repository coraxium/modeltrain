
// Device description data -- MäCAN
#define VERS_HIGH     0       // Versionsnummer vor dem Punkt
#define VERS_LOW      1        // Versionsnummer nach dem Punkt
#define BOARD_NUM     1       // Identifikationsnummer des Boards (Anzeige in der CS2)
#define DEVICE_TYPE   CORAX_S88_RM
#define DEVICE_ARTNUM "S88"
#define DEVICE_NAME   "Coraxium Rückmelder"

/*
 * UID - vor dem Brennen anpassen - die 
 * letzen vier Ziffern hochzaehlen
 */
#define UID 0x10016700


// currently used frequenz on input ports, 
// when using AC on opto coupler
//#define FREQUENZ 50 // Geklappte Wellen bei AC-Optokoppler => 100 Hz
#define FREQUENZ 20   // Empirisch bei positiven Halbwellen =>   25 Hz

#define EVENT_WAIT_COUNT 3  // 3 times it has to be true, before the state will be set to true.

// Setup led (is on, when in program mode, and is blinking for confirm)
#define LED_SETUP A4


// Switch for selection of CAN mode
// If port is true, an s88 event will be sent
// If false, a turnout state is coming up
#define SELECT_MODE_S88_SWITCH 8  // PB0 D8

// Pogramming button
// -----------------
// Start of program mode by select address 0 on adress selector
// and pressing progbtn for 5s. When LED is blinking, programming
// mode is enabled.
// Stop programm mode by select address 0 again and pressing progbtn again for 5s.
#define SW_PROGBTN A5
#define BUTTON_DEBOUNCE_DELAY   20   // [ms]

#define MAX_PING_COUNT 5

// Address selector
// ----------------
// #define SYMBOL    Ard // Pin# // Bank
#define PORTNR_PIN_1  0  //  2   // PD0
#define PORTNR_PIN_2  4  //  6   // PD4
#define PORTNR_PIN_4  1  //  3   // PD1
#define PORTNR_PIN_8  3  //  5   // PD3

 
// Input ports
// -----------
#define PORT_1  7
#define PORT_2  6
#define PORT_3  5
#define PORT_4  9
#define PORT_5  A0
#define PORT_6  A1
#define PORT_7  A2
#define PORT_8  A3


// Count of ports aka contacts
// ---------------------------
#define MAX_FEEDBACK_OUTPUT_COUNT 8


volatile byte pingCounter = 0;

bool isCanModeS88 = false;  // sends turnout position messages, if TRUE it will send s88-responses

unsigned int millisIntervall = 20;            // Default on 50 Hz

word addressList[MAX_FEEDBACK_OUTPUT_COUNT];  // int is 2 bytes long, so word is ok

byte portStateRegister = 0x0;                 // stores the portdata as bitfield

struct PortStates {                           // Struct for the port datas
  byte input = 0;
  byte output = 0;
  bool state = false;
  bool lastState = false;
  long lastMillis = 0;
  bool stateDebounceCounter = 0;
};

PortStates portProcessors[MAX_FEEDBACK_OUTPUT_COUNT]; // Array with port data

// Holds the data of all ports
byte portList[MAX_FEEDBACK_OUTPUT_COUNT] = {          // list of input ports accessable by index of this array
  PORT_1,
  PORT_2,
  PORT_3,
  PORT_4,
  PORT_5,
  PORT_6,
  PORT_7,
  PORT_8
};

// Die Setup-LED is HIER der einzige Ausgang
byte outList[MAX_FEEDBACK_OUTPUT_COUNT] = {         // list of output ports accessable by index of this array
  LED_SETUP,
  LED_SETUP,
  LED_SETUP,
  LED_SETUP,
  LED_SETUP,
  LED_SETUP,
  LED_SETUP,
  LED_SETUP
};



