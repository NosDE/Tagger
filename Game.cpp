/*
  Game.cpp - Game Library for the Infinitag System.
  Created by Jani Taxidis & Tobias Stewen & Florian Kleene.
  Info: www.infinitag.io

  All files are published under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0)
  License: https://creativecommons.org/licenses/by-nc-sa/4.0/
*/

#include "Arduino.h"

// Infinitag
#include "Game.h"


Game::Game(IRsend& ir, Infinitag_Core& core, Adafruit_NeoPixel& ledStrip)
{
  irsend = ir;
  infinitagCore = core;
  strip = ledStrip;
  
  timePlayTime = 60000;//0;
}

void Game::loop() {
  getButtonStates();
  
  if (timeToEnd <= 0) {
    end();
    return;
  }

  colorWipe(strip.Color(0, 0, ledIntensity, 0));
  
  calculateTime();
  demoFunktions();
  
  if (fireBtnState == HIGH) {
    unsigned long shotValue = infinitagCore.ir_encode(false, 0, playerTeamId, playerId, 1, 100);
    irsend.sendRC5(shotValue, 24);
    colorWipe(strip.Color(0, ledIntensity, 0, 0));
    statsShots++;
    playerAmmo--;
  }
  
  delay(100);
}

void Game::start() {
  timeStart = millis();
  timeEnd = timeStart + timePlayTime;
  
  statsShots = 0;
  statsDeath = 0;
  playerAmmo = 120;
  playerHealth = 100;
  
  calculateTime();
}

void Game::end() {
}

bool Game::isRunning() {
  return (timeToEnd > 0);
}

void Game::calculateTime() {
  timeDiff = millis() - timeStart;
  if (timeDiff <= timePlayTime) {
    timeToEnd = timePlayTime - timeDiff;
    if (timeToEnd > 0) {
      timeDiffMinutes = (timeToEnd / 60000);
      timeDiffSeconds = (timeToEnd - (timeDiffMinutes * 60000)) / 1000;
      return;
    }
  }
  timeToEnd = 0;
  timeDiffMinutes = 0;
  timeDiffSeconds = 0;
}


void Game::colorWipe(uint32_t c) {
  for (uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

void Game::demoFunktions() {
  if (upBtnState == HIGH) {
    if (playerId == 1) {
      playerId = 2;
    } else {
      playerId = 1;
      if (playerTeamId == 1) {
        playerTeamId = 2;
      } else {
        playerTeamId = 1;
      }
    }
    updateSensorConfig();
    delay(100);
  }
}

void Game::updateSensorConfig() {
  Wire.beginTransmission(0x22);
  Wire.write(playerTeamId);
  Wire.write(playerId);
  Wire.endTransmission();
  Wire.beginTransmission(0x24);
  Wire.write(playerTeamId);
  Wire.write(playerId);
  Wire.endTransmission();
}

void Game::initButtons(int rP, int lP, int dP, int uP, int sP, int iP, int rlP, int fP, int eP, int rsP) {
  rightBtnPin = rP;
  rightBtnState = 0;
  leftBtnPin = lP;
  leftBtnState = 0;
  downBtnPin = dP;
  downBtnState = 0;
  upBtnPin = uP;
  upBtnState = 0;
  specialBtnPin = sP;
  specialBtnState = 0;
  infoBtnPin = iP;
  infoBtnState = 0;
  reloadBtnPin = rlP;
  reloadBtnState = 0;
  fireBtnPin = fP;
  fireBtnState = 0;
  enterBtnPin = eP;
  enterBtnState = 0;
  resetBtnPin = rsP;
  resetBtnState = 0;
}

void Game::getButtonStates() {
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

void Game::loopStats() {
  /*framebuffer.clear(BLACK);
  
  String text = "Game-Stats";
  char textBuf[50];
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 0, WHITE);
  framebuffer.drawHorizontalLine(0, 14, 128, WHITE);

  text = "Shots: ";
  text += statsShots;
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 17, WHITE);

  text = "Death: ";
  text += statsDeath;
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 31, WHITE);
  
  text = "Press [Enter] to restart";
  text.toCharArray(textBuf, 50);
  framebuffer.displayText(textBuf, 0, 49, WHITE);
  framebuffer.drawHorizontalLine(0, 48, 128, WHITE);

  display_buffer(&display, framebuffer.getData());*/
  
  delay(100);
}

void Game::receiveShot(byte *data, int byteCounter) {
  Serial.println("receiveShot");
  switch (data[0]) {
    case 0x06:
      if (byteCounter == 4) {
        infinitagCore.ir_decode(data);
        if (infinitagCore.ir_recv_cmd == 1) {
          setDamage(infinitagCore.ir_recv_cmd_value);
        }
      }
      break;
      
    default:
      Serial.println("No Command found");
      break;
  }
}

void Game::setDamage(int damage) {
  Serial.println("Damage");
  Serial.println(damage);
  statsDeath++;
}

