

// Setup led (is blinking, when in program mode)
#define LED_SETUP A4

// Pogramming button
// -----------------
// Start of program mode by select address 0 on adress selector
// and pressing progbtn for 5s. When LED is blinking, programming
// mode is enabled.
// Stop programm mode by select address 0 again and pressing progbtn again for 5s.
#define SW_PROGBTN A5
#define BUTTON_DEBOUNCE_DELAY   20   // [ms]


// Address selector
//#define PORTNR_PIN_1  0
//#define PORTNR_PIN_2  4
//#define PORTNR_PIN_4  1
//#define PORTNR_PIN_8  3
#define PORTNR_PIN_1  5
#define PORTNR_PIN_2  4
#define PORTNR_PIN_4  6
#define PORTNR_PIN_8  3


// Inputs
#define PORT_1  A3
#define PORT_2  A2
#define PORT_3  A1
#define PORT_4  A0
//#define PORT_1  17
//#define PORT_2  16
//#define PORT_3  15
//#define PORT_4  14
//#define PORT_5  9
//#define PORT_6  5
//#define PORT_7  6
//#define PORT_8  7



// Count of Contacts
//#define MAX_FEEDBACK_OUTPUT_COUNT 8
#define MAX_FEEDBACK_OUTPUT_COUNT 4

word addressList[MAX_FEEDBACK_OUTPUT_COUNT];  // int is 2 bytes long, so it is ok

byte portStateRegister = 0x0;

// TODO: StateMachine portProcessors[] ???
StateMachine portProcessors[MAX_FEEDBACK_OUTPUT_COUNT] = {
  StateMachine(PORT_1),
  StateMachine(PORT_2),
  StateMachine(PORT_3),
  StateMachine(PORT_4), 
  //  PORT_5,
  //  PORT_6,
  //  PORT_7,
  //  PORT_8
};


