//Transmitter code for Feather M0 RFM69 and DHT11 temp/humidity sensor


#include <DHT.h>
#include <DHT_U.h>
static uint8_t firmware_version =  1; //Version of packet (data structure) sent
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
#include <Adafruit_SleepyDog.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  // The same on all nodes that talk to each other
#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

//Match frequency to the hardware version of the radio on your Feather
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

#define DHTPIN 12
#define DHTTYPE DHT11

/* for Feather M0*/
  #define RFM69_CS      8
  #define RFM69_IRQ     3
  #define RFM69_IRQN    3  // Pin 3 is IRQ 3!
  #define RFM69_RST     4


unsigned int reading_number = 0;  // The reading number is incremented when a transmission happens.

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

DHT dht(DHTPIN,DHTTYPE);

//..........................
// Define a struct to hold the values
//..........................
struct valuesStruct_t
{
  uint8_t  firmware_version;
  unsigned int reading_number;
  float temp;
  unsigned int sleepMS;
};
//..........................
// Define a union type to map from the struct to a byte buffer
//..........................
union txUnion_t
{
  valuesStruct_t values;
  uint8_t b[sizeof(valuesStruct_t)];
};
//..........................
// finally, the code for transmitting the values.
//..........................

void transmitReadings(unsigned int sleepMS) {
  txUnion_t txData;
  reading_number += 1;
  txData.values.reading_number = reading_number;
  txData.values.temp = getTempReading();
  txData.values.sleepMS = sleepMS;
  txData.values.firmware_version = firmware_version;
  
  DEBUG_PRINTF("Version: ");
  DEBUG_PRINT(txData.values.firmware_version);
  DEBUG_PRINTF("| Reading number: ");
  DEBUG_PRINT(txData.values.reading_number);
  DEBUG_PRINTF(" | Temperature: ");
  DEBUG_PRINT(txData.values.temp);
  DEBUG_PRINTF(" | Time since last reading: ");
  DEBUG_PRINT(txData.values.sleepMS);
  DEBUG_PRINTLNF(" ms");
  if (radio.sendWithRetry(RECEIVER, txData.b, sizeof(txUnion_t))) { //target node Id, message as  byte array, message length
   
    radio.sleep();
  }
}
//***********************************************************

float getTempReading(){
  float tempSense = dht.readTemperature(true);
}

//***********************************************************
// SETUP
//***********************************************************
void setup() {

  dht.begin();
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLNF("Feather RFM69HCW Transmitter");

  //Hard Reset the RFM module
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
  reading_number = 0;
  DEBUG_PRINT(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUG_PRINTLNF(" MHz");
}
// The watchdog timer's max amount of sleep time = 8 seconds.  If you set the time between readings to be
// >8000 and <= 32767 (the maximum positive value for an int), the watchdog timer is set to 8 seconds.
// A negative signed int will set the watchdog timer to 15ms.
void loop() {
  //if using the serial monitor, use the delay() function for time between readings.  If taking readings in the field,
  //use the watchdog timer to save on battery life.
  int time_Between_Readings_in_ms = 1000; //set for Debug.  Will change if using the watchdog timer.
  if (DEBUG) {
    // Debugging, so set a few seconds between sending.
    delay(time_Between_Readings_in_ms);
  } else {
    // Since the watchdog timer maxes out at 8 seconds....
    int number_of_sleeper_loops = 4; //i.e.: time between taking a moisture reading is 4 * 8 seconds = 32 seconds.
    for (int i = 0; i < number_of_sleeper_loops; i++) {
      time_Between_Readings_in_ms = Watchdog.sleep(8000);
    }
    time_Between_Readings_in_ms = time_Between_Readings_in_ms * number_of_sleeper_loops;
  }
  transmitReadings(time_Between_Readings_in_ms);
}
