//Receiver code for Feather M0 rfm69 an DHT11 temp/humidity sensor
//******************************************************************/
//...........................
// Identify the version of this software
//...........................
const uint8_t firmware_version = 1;
#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT_BEGIN(x) Serial.begin(x)
#define DEBUG_PRINT(x)  Serial.print (x)
#define DEBUG_PRINTHEX(x) Serial.print(x,HEX)
#define DEBUG_PRINTF(x) Serial.print(F(x))
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTLNF(x) Serial.println(F(x))
#else
#define DEBUG_PRINT_BEGIN(x)
#define DEBUG_PRINT(x)
#define DEBUG_PRINTF(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTLNF(x)
#define DEBUG_PRINTHEX(x)
#endif
#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  //the same on all nodes that talk to each other
#define NODEID        1    // the unique identifier for this node.

#define LED           13  // onboard LED

//Match frequency to the hardware version of the radio on your Feather
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY      RF69_915MHZ
#define ENCRYPTKEY     "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW    true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

 /*for Feather M0*/
  #define RFM69_CS      8
  #define RFM69_IRQ     3
  #define RFM69_IRQN    3  // Pin 3 is IRQ 3!
  #define RFM69_RST     4


RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
//***********************************************************
// BLINK on board LED
//***********************************************************
void Blink(byte PIN, byte DELAY_MS, byte loops)
{
  for (byte i=0; i<loops; i++)
  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
//***********************************************************
// SETUP
//***********************************************************
void setup() {
  // The ATmega32u4 has to shut down the bootloader's USB connection and register a new one, and that takes a while.
  // This line will make the code wait until the USB registration has finished before trying to print anything.
  while ( ! Serial ) ;
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLNF("Feather RFM69HCW Receiver");
  // Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);

  // Initialize radio
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)

  radio.encrypt(ENCRYPTKEY);

  DEBUG_PRINT("...Listening at ");
  DEBUG_PRINT(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUG_PRINTLNF(" MHz");
  pinMode(LED, OUTPUT);
}
//..........................
// Define a struct to hold the values
//..........................
struct valuesStruct_t
{
  uint8_t  firmware_version;
  unsigned int reading_number;
  //float battery_level;
  //int moisture_reading;
  float temp;
  unsigned int sleepMS;
};
//..........................
// Define a union type to map from the struct to a byte buffer
//..........................
union rxUnion_t
{
  valuesStruct_t values;
  uint8_t b[sizeof(valuesStruct_t)];
};
//***********************************************************
// CONTROL LOOP
//***********************************************************
void loop() {
  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    if (strlen((char *)radio.DATA)) {
      Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
      if (radio.ACKRequested())
      {
        radio.sendACK();
      }
    }
    rxUnion_t rxData;
    memcpy(rxData.b, (char *)radio.DATA, sizeof(rxUnion_t));
    DEBUG_PRINTF("   [RX_RSSI:");DEBUG_PRINT(radio.RSSI);DEBUG_PRINTF("]");
    DEBUG_PRINTF("Version: ");
    DEBUG_PRINT(rxData.values.firmware_version);
    if (rxData.values.firmware_version == firmware_version) {
      DEBUG_PRINTF(" (firmware match)");
    }
    DEBUG_PRINTF("| Zone: ");
    DEBUG_PRINT(radio.SENDERID);
    DEBUG_PRINTF("| Reading number: ");
    DEBUG_PRINT(rxData.values.reading_number);
    DEBUG_PRINTF("| Temperature: ");
    DEBUG_PRINT(rxData.values.temp);
    DEBUG_PRINTF(" | Time since last reading: ");
    DEBUG_PRINT(rxData.values.sleepMS);
    DEBUG_PRINTLNF(" ms");
  }
}
