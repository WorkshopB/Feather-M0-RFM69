//Transmitter code for Feather M0 RFM69 and Apds color sensor

#include <SparkFun_APDS9960.h>

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
#include <RFM69.h> 
#include <SPI.h>
#include <Adafruit_SleepyDog.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/ONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NETWORKID     100  // The same on all nodes that talk to each other
#define NODEID        2    // The unique identifier of this node
#define RECEIVER      1    // The recipient of packets

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module
#define LED 13
//*********************************************************************************************
#define SERIAL_BAUD   115200

/* for Feather M0*/
  #define RFM69_CS      8
  #define RFM69_IRQ     3
  #define RFM69_IRQN    3  // Pin 3 is IRQ 3!
  #define RFM69_RST     4


unsigned int reading_number = 0;  // The reading number is incremented when a transmission happens.

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);


SparkFun_APDS9960 apds = SparkFun_APDS9960();
//***********************************************************
 
char  alstr[10]; 
char  rlstr[10]; 
char  glstr[10]; 
char  blstr[10];

char sendBuf[61] = {0};
byte sendLen = 0;

uint16_t  ambient_light; 
uint16_t  red_light; 
uint16_t  green_light; 
uint16_t  blue_light; 
//***********************************************************
// SETUP
//***********************************************************
void setup() {
  
  DEBUG_PRINT_BEGIN(SERIAL_BAUD);
  DEBUG_PRINTLNF("Transmitter Started.");
  
  apds.init(); //color sensor initialize
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
  
  apds.enableLightSensor(false);//begin reading

  
  
}

void loop() {
  
  delay(50);
 if (  !apds.readAmbientLight(ambient_light) ||
        !apds.readRedLight(red_light) ||
        !apds.readGreenLight(green_light) ||
        !apds.readBlueLight(blue_light) ) {
    Serial.println("Error reading light values");
  } else {
    Serial.print("Ambient: ");
    Serial.print(ambient_light);
    Serial.print(" Red: ");
    Serial.print(red_light);
    Serial.print(" Green: ");
    Serial.print(green_light);
    Serial.print(" Blue: ");
    Serial.println(blue_light);
  }delay(1000);
  
  
  dtostrf(ambient_light,3,0,alstr);
  dtostrf(red_light,3,0,rlstr);
  dtostrf(green_light,3,0,glstr);
  dtostrf(blue_light,3,0,blstr);
   sprintf(sendBuf,"Ambient:%s Red:%s Green:%s Blue:%s ",alstr,rlstr,glstr,blstr);
    sendLen = strlen(sendBuf);
    Serial.println(sendBuf);

  radio.send(RECEIVER,&sendBuf,sendLen); //RECEIVER, message as  byte array, message length
  delay(2000);
  
  Blink(LED, 50, 3);
  radio.sleep();
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }

}

char *dtostrf (double val, signed char width, unsigned char prec, char *sout) {
  char fmt[20];
  sprintf(fmt, "%%%d.%df", width, prec);
  sprintf(sout, fmt, val);
  return sout;
}
