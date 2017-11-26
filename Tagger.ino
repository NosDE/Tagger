/*
  Tagger.ino - Tagger Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/
// Basic Libs
#include <SPI.h>
#include <Wire.h>

// Infinitag Libs
#include <sensor_dhcp_server.h>
#include <Infinitag_Core.h>
#include <Infinitag_SH1106.h>
#include <Infinitag_GFX.h>

// Vendor Libs
#include <Adafruit_NeoPixel.h>
#include <IRremote.h>

// Settings
#include "Settings.h"

// Infinitag Inits
//SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);
Infinitag_Core infinitagCore;
sh1106_spi display = create_display(displayResetPin, displayDcPin, displayCsPin);
Framebuffer framebuffer;

// Vendor Inits
IRsend irSend;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, muzzleLedPin, NEO_GRBW + NEO_KHZ800);

#include "Game.h"
Game game(framebuffer, display, irSend, infinitagCore, strip);

void setup() {
  Serial.begin(57600);
  
  //SensorServer.initialize();

  Wire.begin();
  //Wire.onReceive(receiveEvent);

  pinMode(fireBtnPin, INPUT);
  pinMode(muzzleLedPin, OUTPUT);
  
  SPI.begin();
  initialize_display(&display);
  
  strip.begin();
  colorWipe(strip.Color(0,0,0,0));
  
  game.initButtons(rightBtnPin, leftBtnPin, downBtnPin, upBtnPin, specialBtnPin, infoBtnPin, reloadBtnPin, fireBtnPin, enterBtnPin, resetBtnPin);
  game.updateSensorConfig();
}

/*
 * Loops
 */
void loop() {
  getButtonStates();
  pollSensors();

  switch(currentScreen) {
    case 2:
      if (enterBtnState == HIGH) {
        startGame();
        return;
      }
      game.loopStats();
      break;
    case 1:
      if (game.isRunning()) {
        game.loop();
      } else {
        currentScreen = 2;
      }
      break;
    case 0:
    default:
      loopHomescreen();
      break;
  }
}

void loopHomescreen() {
  framebuffer.clear(BLACK);
  
  String text = "Homescreen";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 0, WHITE);
  framebuffer.drawHorizontalLine(0, 14, 128, WHITE);
  
  text = "Press [Enter] to play";
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 30, WHITE);

  display_buffer(&display, framebuffer.getData());
  
  if (enterBtnState == HIGH) {
    startGame();
    return;
  }
  
  delay(100);
}

/*
 * Events
 */
void pollSensors() {
  Serial.print("poll ");
  Serial.println(millis());
  int byteCounter = 0;
  byte data[4] = {
    B0,
    B0,
    B0,
    B0,
  };
  
  Wire.requestFrom(0x22, 4);

  while (Wire.available()) {
    data[byteCounter] = Wire.read();
    byteCounter++;
  }
  Serial.println(data[0]);

  switch(data[0]) {
    case 0x06:
      Serial.println("drin");
      game.receiveShot(data, byteCounter);
      break;
  }
}

/*
 * Helpers
 */
void startGame() {
  game.start();
  currentScreen = 1;
}

void colorWipe(uint32_t c) {
  for (uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void getButtonStates() {
  rightBtnState = digitalRead(rightBtnPin);
  leftBtnState = digitalRead(leftBtnPin);
  downBtnState = digitalRead(downBtnPin);
  upBtnState = digitalRead(upBtnPin);
  specialBtnState = digitalRead(specialBtnPin);
  infoBtnState = digitalRead(infoBtnPin);
  reloadBtnState = digitalRead(reloadBtnPin);
  fireBtnState = digitalRead(fireBtnPin);
  enterBtnState = digitalRead(enterBtnPin);
  resetBtnState = digitalRead(resetBtnPin);
}
