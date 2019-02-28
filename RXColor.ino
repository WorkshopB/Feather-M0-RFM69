//Receiver code for Feather M0 and Apds 9960 RGB Color Sensor
//
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
#include <RFM69.h>    
#include <SPI.h>
#include <SPIFlash.h> 

#define NODEID        1    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define SERIAL_BAUD   115200
#define LED           13 // Feather M0 onboard LED
#define IS_RFM69HCW   true
/* for Feather M0*/
  #define RFM69_CS      8
  #define RFM69_IRQ     3
  #define RFM69_IRQN    3  
  #define RFM69_RST     4


RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

void setup() {
  while ( ! Serial ); 
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);  
  DEBUG_PRINTLNF("Recv Starting...");

  //Hard Reset the RFM module
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  radio.setPowerLevel(31);
  radio.encrypt(ENCRYPTKEY);
  DEBUG_PRINT("...Listening at ");
  DEBUG_PRINT(FREQUENCY == RF69_433MHZ ? 433 : FREQUENCY == RF69_868MHZ ? 868 : 915);
  DEBUG_PRINTLNF(" MHz");
  pinMode(LED, OUTPUT);
  
}
uint32_t packetCount = 0;
void loop() {

  if (radio.receiveDone())
  {
    
   String payload = "";
     for (byte i = 0; i < radio.DATALEN; i++){
     char color = radio.DATA[i];
     payload += color;
     }
     Serial.println(payload); 
   
    Blink(LED,3);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

