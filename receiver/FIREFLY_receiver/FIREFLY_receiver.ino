/* shifts left the '1' over pos times to create a single HIGH bit at location pos. */
#define BIT(pos) ( 1<<(pos) )
/* Set single bit at pos to '1' by generating a mask
in the proper bit location and ORing x with the mask. */
#define SET_BIT(x, pos) ( (x) |= (BIT(pos)) )
/* Set single bit at pos to '0' by generating a mask
in the proper bit location and ORing x with the mask. */
#define UNSET_BIT(x, pos) ( (x) &= ~(BIT(pos)) )
/* Set single bit at pos to opposite of what is currently is by generating a mask
in the proper bit location and ORing x with the mask. */
#define FLIP_BIT(x, pos) ( (x) ^= (BIT(pos)) )
/* Return '1' if the bit value at position pos within y is '1' and '0' if it's 0 by
ANDing x with a bit mask where the bit in pos's position is '1' and '0' elsewhere and
comparing it to all 0's.  Returns '1' in least significant bit position if the value
of the bit is '1', '0' if it was '0'. */
#define CHECK_BIT(var,pos)((var) & (1<<(pos)))

#define FASTLED true

#define LED_PIN     3
#define NUM_LEDS    101

#include <FastLED.h>
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];


// Create a buffer for holding the colors (3 bytes per color).
#define MAX_RX_BYTES 310
#define HEADER 0xFF
#define FOOTER 0xFE
#define MIN 0x00
#define MAX 0xFD
#define BRIGHTNESS          96
#define FRAMES_PER_SECOND  120
#define SET_PIXELS_WHITE_VALUE 0x01
#define SET_PIXELS_RBG_VALUE 0x02
#define SET_GROUPS 0x03
#define SET_PIXEL_WHITE_BY_GROUPS 0x04
#define SET_PIXEL_RBG_BY_GROUPS 0x05
#define SET_SPECIAL 0xF0
#define SET_ALL_LEDS 0xE0

int incomingByte = 0; // for incoming serial data
bool dataReady = false;
bool receivingData = false;

byte RX[MAX_RX_BYTES+1];
int rxIndex;
int rxCount;
long serialWDT = 0;
#include <avr/wdt.h> // for watchdog timer

volatile int counter;      // Count number of times ISR is called.
volatile int countmax = 1; // Arbitrarily selected 3 for this example.
                          // Timer expires after about 24 secs if
                          // 8 sec interval is selected below.

uint16_t ledGroups[NUM_LEDS];
// First two bits could be used for something else, as we do not need to store them SET_ALL/SET_INDIVIDUAL | VALUE_LOAD_TYPE
// SET_ALL/SET_INDIVIDUAL | VALUE_LOAD_TYPE | PERSISTENT/BLINK_INTERVAL | RANDOM_INTERVAL/WAVE_INTERVAL | INTERVAL 4 bits
uint8_t ledValueType[NUM_LEDS];

// Default module address
uint8_t MODULE_ADDRESS = 0;
#define SW1 22
#define SW2 23
#define SW3 5
#define SW4 6
#define SW5 7
#define SW6 8
#define SW7 9
#define SW8 10
void setup() {
   delay(500); // 3 second delay for recovery
  // Set serial speed
  Serial.begin(115200);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW6, INPUT_PULLUP);
  pinMode(SW7, INPUT_PULLUP);
  pinMode(SW8, INPUT_PULLUP);
  pinMode(21, INPUT);  // On 1.3 firefly we have two pins coonnected to LED addr - set one pin to input
  // PinMode za SWITCH
  getModuleAddress();

 // As default set all leds to group 0 binary BIN = 0000000000000001 >> DEC = 1
 setAllLedsToGroups(0);
 rxIndex = 0;
 rxCount = 0;
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(255);
  initBlink();
  watchdogEnable();
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns
uint8_t patternEnabled = 0;

void loop() {
  // on each loop get device address - that way we can simplycheck device change
  getModuleAddress();
  checkPattern();
  if(dataReady){
    patternEnabled=false;
    checkData();
  }
  else{
    getSerialData();
  }
}

void checkPattern(){
  if(patternEnabled){
      // Call the current pattern function once, updating the 'leds' array
      gPatterns[gCurrentPatternNumber]();
      // send the 'leds' array out to the actual LED strip
      FastLED.show();
      // insert a delay to keep the framerate modest
      FastLED.delay(1000/FRAMES_PER_SECOND); 
    
      // do some periodic updates
      EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
      EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically
    }
  }

void checkData(void){
      // Switch commands
      switch (RX[1]){
      case SET_PIXELS_WHITE_VALUE:
        // Check if module is in range
        if (RX[2] <= MODULE_ADDRESS && MODULE_ADDRESS <= RX[3]){
            // Set all LEDs in range
           for(int i=RX[4];i<=RX[5];i++){
              // save all value types
              ledValueType[i] = RX[6];
              leds[i].setRGB( RX[7], RX[7], RX[7]);
            }
          FastLED.show();
          dataReady = false;
          }
          else{
              // If we message is not for this pannel -- ignore it and wait for next
              dataReady = false;
            }
        break;
      case SET_PIXELS_RBG_VALUE:
        // statements
        // Check if module is in range
        if (RX[2] <= MODULE_ADDRESS && MODULE_ADDRESS <= RX[3]){
            // Set all LEDs in range
           for(int i=RX[4];i<=RX[5];i++){
              ledValueType[i] = RX[6];
              leds[i].setRGB( RX[7], RX[8], RX[9]);
            }
          FastLED.show();
          dataReady = false;
          }
          else{
              // If we message is not for this pannel -- ignore it and wait for next
              dataReady = false;
            }        
        break;
      case SET_GROUPS:
        // statements
        // Check if module is in range
        if (RX[2] <= MODULE_ADDRESS && MODULE_ADDRESS <= RX[3]){
            // Set all LEDs in range
          for(int i=RX[4];i<=RX[5];i++){
              setLedToGroup(i,(RX[6]<<8 | RX[7]));
            }
          dataReady = false;
          }
          else{
              // If we message is not for this pannel -- ignore it and wait for next
              dataReady = false;
             // memset(RX, 0, sizeof(RX));
            }
        break;
      case SET_PIXEL_WHITE_BY_GROUPS:
        // statements
        for(int i=0;i<NUM_LEDS;i++){
            // If LED is in the group
            if(checkGroupBits(i,(RX[2]<<8 | RX[3]))){
                ledValueType[i]= RX[4];
                leds[i].setRGB( RX[7], RX[7], RX[7]);
              }
          }
          FastLED.show();
          dataReady = false;
          // Clear RX buffer
          //memset(RX, 0, sizeof(RX));
        break;
      case SET_PIXEL_RBG_BY_GROUPS:
        // statements
        for(int i=0;i<NUM_LEDS;i++){
            // If LED is in the group
            if(checkGroupBits(i,(RX[2]<<8 | RX[3]))){
                ledValueType[i]= RX[4];
                leds[i].setRGB( RX[5], RX[6], RX[7]);
              }
          }
          FastLED.show();
          dataReady = false;
        break;
        // statements
          dataReady = false;
        break;
      case SET_SPECIAL:
        // Check if module is in range
        if (RX[2] <= MODULE_ADDRESS && MODULE_ADDRESS <= RX[3]){
            // Set all LEDs in range
          patternEnabled=true;
          //FastLED.show();
          dataReady = false;
          }
          else{
              // If we message is not for this pannel -- ignore it and wait for next
              dataReady = false;
            }
        break;
        case SET_ALL_LEDS:
        // Check if module is in range
        if (RX[2] <= MODULE_ADDRESS && MODULE_ADDRESS <= RX[3]){
          // Set all LEDs in range
          for(int i=0;i<100;i++){
              leds[i].setRGB( RX[4+i*3], RX[5+i*3], RX[6+i*3]);
            }
          FastLED.show();
          dataReady = false;
          }
          else{
              // If we message is not for this pannel -- ignore it and wait for next
              dataReady = false;
             // memset(RX, 0, sizeof(RX));
            }
        break;
        default:
        dataReady = false;
        receivingData = false;
    }
  }

void getSerialData(void){

    // If data on serial is waiting
    if (Serial.available() > 0) {

    RX[rxIndex] = Serial.read();
    
    if(receivingData){
      // Store data in buffer
      // If FOOTER received
      if(RX[rxIndex] == FOOTER){
        serialWDT = millis();
        dataReady = true;
        receivingData = false;
        // Add one as index start from 0
        rxCount = rxIndex+1;
        // Clear index to 0
        rxIndex=0;
        wdt_reset(); // start timer again (still in interrupt-only mode)
      }
      else{
        // If FOOTER is not received add one to index
        rxIndex++;
      }
    }
    else if(RX[rxIndex] == HEADER){
      receivingData = true;
      rxIndex++;
    }
    }
    // Clear before buffer overflow
    if (Serial.available() >= MAX_RX_BYTES || rxIndex >= MAX_RX_BYTES) {
      //Serial.flush();
      rxIndex=0;
    }
}

boolean checkGroupBits(uint8_t ledID, uint16_t groups){
    for(int i=0;i<16;i++){
        if((CHECK_BIT(ledGroups[ledID],i)) && CHECK_BIT(groups,i)){
            return 1;
          }
      }
    return 0;
  }
  
void setAllLedsToGroups(uint16_t groups){
    for(int i=0;i<NUM_LEDS;i++){
      setLedToGroup(i, groups);
    }
}
void setLedToGroup(uint8_t ledID, uint16_t groups){
    ledGroups[ledID] = groups;
}

/*
uint16_t getLedGroup(uint8_t ledID){
    uint16_t group;
    group = ledGroups[ledID];
    return group;
}
*/
void getModuleAddress(void){
    MODULE_ADDRESS = digitalRead(SW8)<<7 | digitalRead(SW7)<<6 | digitalRead(SW6)<<5 | digitalRead(SW5)<<4 | digitalRead(SW4)<<3 | digitalRead(SW3)<<2 | digitalRead(SW2)<<1 | digitalRead(SW1);
    MODULE_ADDRESS = ~MODULE_ADDRESS;
  }

void initBlink(){
      blinkFastLed(100);
}

void blinkFastLed(uint16_t timer){
  for (uint16_t i = 0; i < NUM_LEDS; i++){
      leds[i].setRGB( 5, 5, 5);
      //leds[i].maximizeBrightness(MAX);
  }
  FastLED.show();
  delay(timer);
  for (uint16_t i = 0; i < NUM_LEDS; i++){
      leds[i].setRGB( MIN, MIN, MIN);
      //leds[i].maximizeBrightness(MIN);
  }
  FastLED.show();
  delay(timer);
}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void watchdogEnable()
{
 cli();                              // disable interrupts

 MCUSR = 0;                          // reset status register flags
  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
                                     // Put timer in interrupt-only mode:                                        
 //WDTCSR |= 0b00011000;               // Set WDCE (5th from left) and WDE (4th from left) to enter config mode,
                                     // using bitwise OR assignment (leaves other bits unchanged).
 WDTCSR =  0b01000000 | 0b000110;    // set WDIE (interrupt enable...7th from left, on left side of bar)
                                     // clr WDE (reset enable...4th from left)
                                     // and set delay interval (right side of bar) to 8 seconds,
                                     // using bitwise OR operator.

 sei();                              // re-enable interrupts
 wdt_reset();                      // this is not needed...timer starts without it

 // delay interval patterns:
 //  16 ms:     0b000000
 //  500 ms:    0b000101
 //  1 second:  0b000110
 //  2 seconds: 0b000111
 //  4 seconds: 0b100000
 //  8 seconds: 0b100001

}
