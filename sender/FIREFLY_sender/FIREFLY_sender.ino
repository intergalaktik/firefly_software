#include <PololuLedStrip.h>

// Create an ledStrip object and specify the pin it will use.
PololuLedStrip<3> ledStrip;

// Create a buffer for holding the colors (3 bytes per color).
#define LED_COUNT 100

rgb_color colors[LED_COUNT];

uint8_t MODULE_ADDRESS = 0;
#define SW1 22
#define SW2 23
#define SW3 5
#define SW4 6
#define SW5 7
#define SW6 8
#define SW7 9
#define SW8 10

#define MIN 0x00
#define MAX 0xFD
#define HEADER 0xFF
#define FOOTER 0xFE

// uncompiled, untested
byte customMsg1[] = { HEADER, 0x01, MIN, MAX, MIN, 0x63, 0x00, MIN, FOOTER };
byte customMsg2[] = { HEADER, 0x02, MIN, MAX, MIN, 0x63, 0x00, MIN, MIN, MIN, FOOTER };
byte msgAllOff[] = { HEADER, 0x01, MIN, MAX, MIN, 0x63, 0x00, MIN, FOOTER };
byte setAllLeds[306];

byte doSpecialMsg[] = { HEADER, 0xF0, MIN, MAX, 0x01, FOOTER };


bool special = false;

void setup() {
  // put your setup code here, to run once:
  

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW6, INPUT_PULLUP);
  pinMode(SW7, INPUT_PULLUP);
  pinMode(SW8, INPUT_PULLUP);
  pinMode(21, INPUT);      
  getModuleAddress();
  Serial.begin(115200);
  Serial1.begin(115200);
  initBlink(); 
}

void loop() {

getModuleAddress();

if (MODULE_ADDRESS == 0) {
  
  // put your main code here, to run repeatedly:
  if (Serial1.available()) {      // If anything comes in Serial (USB),
    Serial.write(Serial1.read());   // read it and send it out Serial1 (pins 0 & 1)
  }
}
else if (MODULE_ADDRESS == 1) {
      special=false;
    sendWhiteRider(5);
  }
else if (MODULE_ADDRESS == 2) {
      special=false;
    sendBlinkColors();
  }
else if (MODULE_ADDRESS == 3) {
      special=false;
    sendRandomColorRider(5);
  }
else if (MODULE_ADDRESS == 4) {
      special=false;
    sendWhiteRider(5);
  }
else if (MODULE_ADDRESS == 8) {
      special=false;
    sendRandomColorRider(100);
  } 
else if (MODULE_ADDRESS == 16) {
    special=false;
    sendWhiteRider(500);
  } 
else if (MODULE_ADDRESS == 32) {
          special=false;
    doSpecial(1);
  } 
else if (MODULE_ADDRESS == 64) {

      special=false;
    //sendWhiteRider(5);
    sendAllRandom(5);
  } 
}

void sendAllRandom(uint8_t timeout){
  
  setAllLeds[0] = HEADER;
  setAllLeds[1] = 0xE0;  
  setAllLeds[2] = MIN;  
  setAllLeds[3] = MAX;  
  setAllLeds[305] = FOOTER; 
  
  for (int i=0;i<LED_COUNT*3;i++){
    setAllLeds[i+4] = random(MIN,10);
  }
  Serial.write(setAllLeds, sizeof(setAllLeds));
  delay(timeout);
}

void sendWhiteRider(uint8_t timeout){
    for (int i=0;i<100;i++){
        customMsg1[4] = i;
        customMsg1[5] = i;
        customMsg1[7] = MAX;
        Serial.write(customMsg1, sizeof(customMsg1));
        delay(timeout);
        Serial.write(msgAllOff, sizeof(msgAllOff));
        delay(timeout);
    }
}

void sendBlinkColors(){
    customMsg2[7] = MIN;
    customMsg2[8] = MIN;
    customMsg2[9] = MIN;
    customMsg2[4] = MIN;
    customMsg2[5] = 0x63;
    for (int i=0;i<3;i++){
        // Set all panels and all LEDS to highest brightness
        customMsg2[7+i] = MAX;
        Serial.write(customMsg2, sizeof(customMsg2));
        customMsg2[7+i] = MIN;
        delay(500);      
        Serial.write(msgAllOff, sizeof(msgAllOff));
        delay(500);
    }
  }

void sendRandomColorRider(uint8_t timeout){
    for (int i=0;i<100;i++){
        customMsg2[4] = i;
        customMsg2[5] = i;
        for (int j=0;j<3;j++){
            customMsg2[7+j] = random(MIN,MAX);
        }
        Serial.write(customMsg2, sizeof(customMsg2));
        delay(timeout);
        Serial.write(msgAllOff, sizeof(msgAllOff));
        delay(timeout);
    }
  }

void doSpecial(uint8_t ID){
  if(!special){
    doSpecialMsg[4] = ID;
    Serial.write(doSpecialMsg, sizeof(doSpecialMsg));
  }
  special=true;
  }

void getModuleAddress(void){
    MODULE_ADDRESS = digitalRead(SW8)<<7 | digitalRead(SW7)<<6 | digitalRead(SW6)<<5 | digitalRead(SW5)<<4 | digitalRead(SW4)<<3 | digitalRead(SW3)<<2 | digitalRead(SW2)<<1 | digitalRead(SW1);
    MODULE_ADDRESS = ~MODULE_ADDRESS;
  }

void initBlink(){
  for (uint16_t i = 0; i < LED_COUNT; i++){
    colors[i] = rgb_color(255, 255, 255);
  }
  ledStrip.write(colors, LED_COUNT);
  delay(200);
  for (uint16_t i = 0; i < LED_COUNT; i++){
    colors[i] = rgb_color(0, 0, 0);
  }
  ledStrip.write(colors, LED_COUNT);
}
