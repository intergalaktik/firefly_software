/*                                                                   
 * GPL3 License                                                         
 *                                                                      
 * Author(s):                                                              
 *      Intergalaktik d.o.o. <warp@intergalaktik.eu>                  
 *                                                                      
 *                                                                      
 * Copyright (c) 2021 Intergalaktik d.o.o.                              
 *                                                                      
 * This file is part of FireFly software.                                          
 *                                                                      
 * FireFly software is free software: you can redistribute it and/or modify        
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, either version 3 of the License, or    
 * (at your option) any later version.                                  
 *                                                                      
 * aPlus is distributed in the hope that it will be useful,             
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        
 * GNU General Public License for more details.                         
 *                                                                      
 * You should have received a copy of the GNU General Public License    
 * along with aPlus.  If not, see <http://www.gnu.org/licenses/>.                                                    
 * 
   Modul 8.3 ---  ID1
   ulaz: simulacija tastature na USB-u s tipkama "A", "B" i "C" (kontakti k1, k2 i k3).
   logika:
   ako je prekinut kontakt k1, onda osvijetli podskup lampica p1 na 100% bijele, ostalo isključi.
   ako je prekinut kontakt k2, onda osvijetli podskup lampica p2 na 100% bijele, ostalo isključi.
   ako je prekinut kontakt k3, onda osvijetli podskup lampica p3 na 100% bijele, ostalo isključi.
   Ukupno će biti spojeno 39 LED-ica. Ovo je definicija grupa P1, P2 i P3 (1-based indexing):
   P1= 13-15, 34-36;
   P2= 1-15, 19-21, 25-39;
   P3= 1-3, 10-18, 22-39

   Modul 5.6 -- ID3
   ulaz: kontakt 3 kanala (jumperi)
   logika:
   svaki put kad se promijeni bilo koji kontakt onda se provjerava sljedeće:
   zbroji kanale k1, k2 i k3 da dobiješ K (konačno 0-3)
   ako je K=0, onda je osvijetljenje podskup lampica P1 s treperenjem (tamno crvena)
   ili ako je K=1, onda je osvijetljenje podskup lampica P1 s treperenjem (crvena) i P2 s treperenjem (tamno crvena)
   ili ako je K=2, onda je osvijetljenje podskup lampica P1 s treperenjem (narančasta) i P2 s treperenjem (crvena) i P3 s treperenjem (tamno crvena)
   ili ako je K=3, onda je osvijetljenje podskup lampica P1 s treperenjem (žuta) P2 s treperenjem (narančasta) i P3 s treperenjem (crvena) i P4 s treperenjem (crvena)
   P1: 1-25
   P2: 26-50
   P3: 51-75
   P4: 76-100
   (1-based indexing)
   tamno crvena (128,0,0)
   crvena    (255,0,0)
   narančasta  (255, 51, 0)
   žuta    (255, 255, 0)

   Modul 8.6  -- ID2
   ulaz: USB iz kompjutera
   logika:
   preko USB serijskog se emitira pet binarnih brojeva (a, b, c, d, e)
   ako je a = 1, onda upali podskup lampica p1
   ako je b = 1, onda upali podskup lampica p2
   ako je c = 1, onda upali podskup lampica p3
   ako je d = 1, onda upali podskup lampica p4
   ako je e = 1, onda upali podskup lampica p5

   http://fastled.io/docs/3.1/struct_c_r_g_b.html

*/

#include <FastLED.h>

#define NUM_LEDS 100
#define CHIPSET     WS2811
#define COLOR_ORDER RGB
#define LED_PIN 3
CRGB leds[NUM_LEDS];
#define BRIGHTNESS  255

#define flickerON 30
#define flickerOFF 1

int counter = 0;
int maxCount = flickerON + flickerOFF;

long resetTimer = 0;
// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI


int subSet_8_3_1[] = { 13, 14, 15, 34, 35, 36 };
int subSet_8_3_2[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 19, 20, 21, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };
int subSet_8_3_3[] = { 1, 2, 3, 10, 11, 12, 13, 14, 15, 16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39 };

int subSet56_DIV_NUMBER = 4;
int subSet56_P_NUMBER = NUM_LEDS / subSet56_DIV_NUMBER;
int subSet86_DIV_NUMBER = 5;
int subSet86_P_NUMBER = NUM_LEDS / subSet86_DIV_NUMBER;

boolean SW1_status = 0;
boolean SW2_status = 0;
boolean SW3_status = 0;
boolean SW_changed = 0;
int SW_SUM_OLD, SW_SUM = 0;

#define SW1 22
#define SW2 23
#define SW3 5
#define SW4 6
#define SW5 7
#define SW6 8
#define SW7 9
#define SW8 10
uint8_t MODULE_ADDRESS = 0;

void setup() {

  // SW1 - SW4 are used for button inputs
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);

  // SW5 - SW8 are used for module ID
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW6, INPUT_PULLUP);
  pinMode(SW7, INPUT_PULLUP);
  pinMode(SW8, INPUT_PULLUP);
  getModuleAddress();
  Serial1.begin(115200);
  delay( 1000 ); // power-up safety delay
  // It's important to set the color correction for your LED strip here,
  // so that colors can be more accurately rendered through the 'temperature' profiles
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalSMD5050 );
  //FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.setBrightness( BRIGHTNESS );
  //MODULE_ADDRESS = 3;
  resetTimer = millis();

}

void loop() {
  if (counter < maxCount) {
    counter++;
  }
  else {
    counter = 0;
  }
  moduleMain();
}

void moduleMain() {
  readAllButtons();
  switch (MODULE_ADDRESS) {
    case 0:
      break;
    case 1:
      runModule1();
      break;
    case 2:
      runModule2();
      break;
    case 3:
      runModule3();
      break;
    default:
      break;
  }
}

void readAllButtons() {
  // Inverting buttons readings so closed button will be 1
  SW1_status = !digitalRead(SW1);
  SW2_status = !digitalRead(SW2);
  SW3_status = !digitalRead(SW3);
  SW_SUM = SW1_status + SW2_status + SW3_status;
  if (SW_SUM != SW_SUM_OLD) {
    SW_changed = 1;
  }
  else {
    SW_changed = 0;
  }
  SW_SUM_OLD = SW_SUM;
}

void runModule1() {
  if (SW_SUM != 0) {
    allOff();
    if (SW1_status) {
      subsetToWhite(BRIGHTNESS, subSet_8_3_1, sizeof(subSet_8_3_1));
    }
    if (SW2_status) {
      subsetToWhite(BRIGHTNESS, subSet_8_3_2, sizeof(subSet_8_3_2));
    }
    if (SW3_status) {
      subsetToWhite(BRIGHTNESS, subSet_8_3_3, sizeof(subSet_8_3_3));
    }
  }
  else {
    allOff();
  }
}
void runModule2() {
  if (resetTimer < (millis() - 10000)) {
    allOff();
    resetTimer = millis();
  }
  // check micros
  // if more then 10 sec
  // turn off leds
  // reset micros
  if (Serial1.available() > 0) {
    // reset micros
    resetTimer = millis();
    int myChar = Serial1.read();
    switch (myChar) {
      case 'a':
        allOff();
        setRangeToWhite(BRIGHTNESS, 0, subSet86_P_NUMBER);
        break;
      case 'b':
        allOff();
        setRangeToWhite(BRIGHTNESS, subSet86_P_NUMBER, subSet86_P_NUMBER * 2);
        break;
      case 'c':
        allOff();
        setRangeToWhite(BRIGHTNESS, subSet86_P_NUMBER * 2, subSet86_P_NUMBER * 3);
        break;
      case 'd':
        allOff();
        setRangeToWhite(BRIGHTNESS, subSet86_P_NUMBER * 3, subSet86_P_NUMBER * 4);
        break;
      case 'e':
        allOff();
        setRangeToWhite(BRIGHTNESS, subSet86_P_NUMBER * 4, subSet86_P_NUMBER * 5);
        break;
      case 'f':
        allOff();
        break;
      default:
        break;
    }
    // Get byte
    // Switch byte
  }
}
void runModule3() {
  if (counter <= flickerOFF) {
    allOff();
  }
  else {
    switch (SW_SUM) {
      case 0:
        K0_56();
        break;
      case 1:
        K1_56();
        break;
      case 2:
        K2_56();
        break;
      case 3:
        K3_56();
        break;
      default:
        allOff();
        break;
    }
  }
}

void getModuleAddress(void) {
  MODULE_ADDRESS = 1 << 7 | 1 << 6 | 1 << 5 | 1 << 4 | digitalRead(SW8) << 3 | digitalRead(SW7) << 2 | digitalRead(SW6) << 1 | digitalRead(SW5);
  MODULE_ADDRESS = ~MODULE_ADDRESS;
}
void allOff() {
  // Now turn the LED off, then pause
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void setRangeToWhite(int brightness, int fromRange, int toRange) {
  FastLED.setBrightness( brightness );
  for (int i = fromRange; i < toRange; i++) {
    leds[i] = CRGB::White;
  }
  FastLED.show();
}

void subsetToWhite(int brightness, int *subset, int arraySize) {
  arraySize = arraySize / sizeof(int);
  FastLED.setBrightness( brightness );
  for (int i = 0; i < arraySize; i++) {
    leds[subset[i] - 1] = CRGB::White;
  }
  FastLED.show();
}

void K0_56(void) {
  for (int i = 0; i < subSet56_P_NUMBER; i++) {
    leds[i] = CRGB::OrangeRed;
  }
  for (int i = subSet56_P_NUMBER; i < subSet56_P_NUMBER * 2; i++) {
    leds[i] = CRGB::Black;
  }
  for (int i = subSet56_P_NUMBER * 2; i < subSet56_P_NUMBER * 3; i++) {
    leds[i] = CRGB::Black;
  }
  for (int i = subSet56_P_NUMBER * 3; i < subSet56_P_NUMBER * 4; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void K1_56(void) {
  for (int i = 0; i < subSet56_P_NUMBER; i++) {
    leds[i] = CRGB::Red;
  }
  for (int i = subSet56_P_NUMBER; i < subSet56_P_NUMBER * 2; i++) {
    leds[i] = CRGB::OrangeRed;
  }
  for (int i = subSet56_P_NUMBER * 2; i < subSet56_P_NUMBER * 3; i++) {
    leds[i] = CRGB::Black;
  }
  for (int i = subSet56_P_NUMBER * 3; i < subSet56_P_NUMBER * 4; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void K2_56(void) {
  for (int i = 0; i < subSet56_P_NUMBER; i++) {
    leds[i] = CRGB::Orange;
  }
  for (int i = subSet56_P_NUMBER; i < subSet56_P_NUMBER * 2; i++) {
    leds[i] = CRGB::Red;
  }
  for (int i = subSet56_P_NUMBER * 2; i < subSet56_P_NUMBER * 3; i++) {
    leds[i] = CRGB::OrangeRed;
  }
  for (int i = subSet56_P_NUMBER * 3; i < subSet56_P_NUMBER * 4; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void K3_56(void) {
  for (int i = 0; i < subSet56_P_NUMBER; i++) {
    leds[i] = CRGB::Yellow;
  }
  for (int i = subSet56_P_NUMBER; i < subSet56_P_NUMBER * 2; i++) {
    leds[i] = CRGB::Orange;
  }
  for (int i = subSet56_P_NUMBER * 2; i < subSet56_P_NUMBER * 3; i++) {
    leds[i] = CRGB::Red;
  }
  for (int i = subSet56_P_NUMBER * 3; i < subSet56_P_NUMBER * 4; i++) {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
}
