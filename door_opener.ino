/******************************************************************************************/
/*
 * Based on readMifareClassic.pde from Adafruit Industries (BSD license - see license.txt)
 * 
 * Reads the 4 byte (32 bit) ID of a MiFare Classic card, and compares it to the
 * allowed cards. If the ID matches, the actuator is activated to open the door,
 * and the light blinks.
 * 
 * TODO:
 * - compare card id's to known list
 * - actuator control
*/
/******************************************************************************************/

#include "FastLED.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#define NUM_LEDS    1
#define DATA_PIN    11
#define CLOCK_PIN   13
#define CHIPSET     WS2801
#define COLOR_ORDER RGB

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(150);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    //Didn't find PN53x board. halt.
    while (1)
      blinkOpen();
  }
  // configure board to read RFID tags
  nfc.SAMConfig();
}

void loop() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength); // note - this statement blocks until a card is read. optionally, a timeout can be specified.
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];  
      cardid <<= 8;
      cardid |= uid[3]; 
      //TODO check cardid!
    }
}

void blinkOpen(){
  for(int i=0; i < 8; i++){
    leds[0] = CRGB::Red; FastLED.show(); delay(250);
    leds[0] = CRGB(50,180,0); FastLED.show(); delay(250);
  }
}

