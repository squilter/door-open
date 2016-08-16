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
#include <stdbool.h>
#include "FastLED.h"

#define DATA_PIN    11
#define CLOCK_PIN   13
#define CHIPSET     WS2801
#define COLOR_ORDER RGB
CRGB leds[1];

#define ACTUATOR_PIN  5
#define DOOR_SENS_PIN 7

// Use this for I2C
/**
#include <Wire.h>
#include <Adafruit_PN532.h>
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
*/

// Use this for UART
#include <PN532_HSU.h>
#include <PN532.h>
PN532_HSU pn532hsu(Serial);
PN532 nfc(pn532hsu);

#define NORMAL_OPEN 1
#define NORMAL_FAIL 2
#define PURPLE_OPEN 3

const uint32_t keys[] = { 1, 2, 3, 4 }; //put key here. Usually 10 digits printed on back of card
const uint8_t  key_size = 4; // the number of keys


void setup() {
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, 1);
  FastLED.setBrightness(50);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    //Didn't find PN53x board. halt.
    blinkPurple(999999);
  }
  // configure board to read RFID tags
  nfc.SAMConfig();
  pinMode(DOOR_SENS_PIN, INPUT_PULLUP);
  pinMode(ACTUATOR_PIN, OUTPUT);
}

void loop() {
  if(!digitalRead(DOOR_SENS_PIN)){//if the door is open, just blink "open" and don't do anything else
    blinkOpen(1);
    return;
  }
  leds[0] = CRGB::Red;
  FastLED.show();

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.). When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength); // note - this statement blocks until a card is read. optionally, a timeout can be specified.
    if (success)
    {
      // We probably have a Mifare Classic card ... 
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];  
      cardid <<= 8;
      cardid |= uid[3];
      if(isvalueinarray(cardid, keys, key_size)){
        digitalWrite(ACTUATOR_PIN, HIGH);
        blinkOpen(6);
        digitalWrite(ACTUATOR_PIN, LOW);
        blinkOpen(6);
      }else{
        blinkFail();  
      }
    }
}

void blinkFail(){
  leds[0] = CRGB(150,255,0); FastLED.show();
  delay(2000); 
}

void blinkOpen(int reps){
  for(int i=0;i<reps;i++){
     leds[0] = CRGB::Red; FastLED.show(); delay(250);
     leds[0] = CRGB(150,255,0); FastLED.show(); delay(250);
   }
}

void blinkPurple(int reps){
  for(int i=0;i<100;i++){
   leds[0] = CRGB(255,0,255); FastLED.show(); delay(250);//purple
   leds[0] = CRGB(255,255,0); FastLED.show(); delay(250);//yellow
 }
}

bool isvalueinarray(int val, const uint32_t *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}

