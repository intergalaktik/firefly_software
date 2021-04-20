// DMX Receiver/Fixture Example for WS2812B RGB LEDs
// Target Hardware:  Arduino Uno
//
// Receives individual RGB levels for 60 WS2812B LEDs (60*3 = 180 DMX channels total)
// over an RS485 link in DMX packet format, to be decoded and sent out to a 60 LED strip.
// The test pattern is an LED chaser where two different color bars run end to end
// along the strip at different speeds, crossing over each other when they meet.
//
// The first DMX channel is 1 so the 60 LED RGB values are sent sequentially in
// channels 1 to 180, eg:
// Ch   1: LED  0 RED   value [0-255]
// Ch   2: LED  0 GREEN value [0-255]
// Ch   3: LED  0 BLUE  value [0-255]
// Ch 178: LED 59 RED   value [0-255]
// Ch 179: LED 59 GREEN value [0-255]
// Ch 180: LED 59 BLUE  value [0-255]
//
// Required library:
//    DMXSerial        install from library manager or https://github.com/mathertel/DMXSerial
//
// Gadget Reboot
// https://www.youtube.com/gadgetreboot

#include <DMXSerial.h>
#include "ws2812.h"                // a specific LED controller that disables interrupts to work better

#define NUM_LEDS 144                // number of RGB LEDs on strip
#define DMXSTART 5                 // first DMX channel
#define DMXLENGTH (NUM_LEDS*3)     // number of DMX channels used (3*60 LEDs)

uint8_t *DMX_buffer;

#define SW1 22
#define SW2 23
#define SW3 5
#define SW4 6
#define SW5 7
#define SW6 8
#define SW7 9
#define SW8 10
uint8_t MODULE_ADDRESS = 0;

void setup () {

  DMXSerial.init(DMXProbe);        // initialize DMX bus in manual access mode
  DMXSerial.maxChannel(DMXLENGTH); // "onUpdate" will be called when all new ch data has arrived

  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW6, INPUT_PULLUP);
  pinMode(SW7, INPUT_PULLUP);
  pinMode(SW8, INPUT_PULLUP);

  setupNeopixel();                 // setup the LED output hardcoded to pin 12 in ws2812.h
  getModuleAddress();
}

void loop() {
  // wait for an incomming DMX packet and write
  // the RGB data for 144 LEDs on the strip
  if (DMXSerial.receive()) {
    getModuleAddress();
    uint8_t panelFunction =  DMXSerial.read(1);
    uint8_t firstPanel = DMXSerial.read(2);
    uint8_t lastPanel = DMXSerial.read(3);
    uint8_t special =  DMXSerial.read(4);
    if (firstPanel <= MODULE_ADDRESS && MODULE_ADDRESS <= lastPanel){
      updateNeopixel(DMXSerial.getBuffer() + DMXSTART, NUM_LEDS);
    }
  }
}

void getModuleAddress(void){
    MODULE_ADDRESS = digitalRead(SW8)<<7 | digitalRead(SW7)<<6 | digitalRead(SW6)<<5 | digitalRead(SW5)<<4 | digitalRead(SW4)<<3 | digitalRead(SW3)<<2 | digitalRead(SW2)<<1 | digitalRead(SW1);
    MODULE_ADDRESS = ~MODULE_ADDRESS;
  }
