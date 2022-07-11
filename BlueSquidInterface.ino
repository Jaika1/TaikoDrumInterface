/*
 * BlueSquid Interface v1.0.0 made by Jaika 2022. Designed and prototyped using a Leonardo.
 * 
 * DEVICE CONFIG
 * 
 * Rotary Encoder:
 * CLK      - 3
 * DT       - 2
 * SW       - 4 (Button index 0)
 * 
 * OLED Display:
 * CLK      - 13
 * DATA/MOS - 11
 * DC       - 9
 * CS       - 10
 * RESET    - 12
 * 
 * Taiko Sensors:
 * LEFTKA   - A0
 * LEFTDON  - A1
 * RIGHTDON - A2
 * RIGHTKA  - A3
 * 
 * Extra buttons:
 * pins 5-8, configure buttonCount definiton accordingly (indicies 1-4).
 * 
 * See https://github.com/Jaika1/TaikoDrumInterface.git for more info.
 */

 /* TAIKO FORCE PINOUT (As of dec 2021)
  *         ___
  *  |LDO|RKA|RDO|LKA|
  *  |Gnd|Gnd|Gnd|Gnd|
  */

#include <Keyboard.h>
//#include <Arduino.h>
#include <EEPROM.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// Device setup
U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R2, /*clock*/ 13, /*data*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 12);

#define enc_dt 2
#define enc_clk 3
#define buttonStart 4
#define buttonCount 1

//EEPROM ADDRESSES
#define timeOutAdr 0
#define senseAdr 1
#define multiHitAdr 9
#define readDelayAdr 10

volatile int encoderValue = 0;

volatile int startMs = 0;
volatile bool hideDisplay = false;
bool subMenu = false;
byte menuMode = 0;
byte menuOption = 0;
int lastInput = 0;
bool buttonValues[buttonCount];
bool buttonValuesLast[buttonCount];

byte timeOut = 0;
bool multiHit = false;
byte readDelay = 10;
int sens[4];

int cSens[4];
int sensors[4];
int lastSensors[4];

char keys[4] = {'h', 'j', 'k', 'l'};

bool doMenuUpdate = true;

void setup() {
  loadData();
  
  pinMode(enc_dt, INPUT_PULLUP);
  pinMode(enc_clk, INPUT_PULLUP);
  for (int i = buttonStart; i < buttonStart + buttonCount; i++){
    pinMode(i, INPUT);
  }
  attachInterrupt(digitalPinToInterrupt(enc_clk), doEncoder, FALLING);
  
  u8g2.begin();
  u8g2.setFontPosTop();
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  
  Keyboard.begin();
  lastInput = millis();
}

void loop() {
  doButtonUpdate();
  doSensorUpdate();
  startMs = millis();
  
  if (hideDisplay == false && (startMs - lastInput) / 1000 >= timeOut){
    hideDisplay = true;
    u8g2.clearBuffer();
    u8g2.sendBuffer();
  }
  
  if (!hideDisplay)
    doDisplay();
}

void doSensorUpdate(){
  int vals[4];
  byte indicies[4] = {0,1,2,3};
  
  for (byte i = 0; i < 4; i++){
    sensors[i] = analogRead(i);
    vals[i] = sensors[i];
  }

  for(byte i = 0; i < 4; i++){
    byte x = 0;
    for (byte y = 0; y < 4; y++){
      if (indicies[y] == i){
        x = y;
        break;
      }
    }
    
    while(x < 3 && vals[x] < vals[x + 1])
    {
      int vx = vals[x];
      int vxo = vals[x + 1];
      int ix = indicies[x];
      int ixo = indicies[x + 1];

      vals[x] = vxo;
      vals[x + 1] = vx;
      indicies[x] = ixo;
      indicies[x + 1] = ix;
      
      x++;
    }
  }
  
  for (byte x = 0; x < 4; x++){
    byte i = indicies[x];
    if (sensors[i] >= sens[i]){
      if (multiHit){
        if (sensors[i] <= lastSensors[i]){
          lastSensors[i] = sensors[i];
          continue;
        }
      }
      Keyboard.press(keys[i]);
      delay(readDelay);
      Keyboard.releaseAll();
    }
    lastSensors[i] = sensors[i];
  }
}

void loadData(){
  // TIMEOUT = 0
  timeOut = EEPROM.read(timeOutAdr);
  if (timeOut == 255){
    timeOut = 5;
    EEPROM.write(timeOutAdr, timeOut);
  }

  // SENSE = 1-8
  for (int i = 0; i < 4; i++){
    int b1 = EEPROM.read(senseAdr + (2 * i));
    int b2 = EEPROM.read(senseAdr + (2 * i) + 1);
    sens[i] = (b1 << 8) + b2;
    if (b1 == 255){
      sens[i] = (i == 0 || i == 3) ? 252 : 102;
      EEPROM.write(senseAdr + (2 * i), sens[i] >> 8);
      EEPROM.write(senseAdr + (2 * i) + 1, sens[i]);
    }
  }
  
  // MULTIHIT = 9
  int multiHitI = EEPROM.read(multiHitAdr);
  if (multiHitI == 255){
    multiHit = false;
    EEPROM.write(multiHitAdr, multiHit);
  }
  else {
    multiHit = multiHitI;
  }
  
  // READDELAY = 10
  readDelay = EEPROM.read(readDelayAdr);
  if (readDelay == 255){
    readDelay = 10;
    EEPROM.write(readDelayAdr, readDelay);
  }
}

void doButtonUpdate(){
  for (int i = 0; i < buttonCount; i++){
    buttonValuesLast[i] = buttonValues[i];
    buttonValues[i] = digitalRead(buttonStart + i);
  }

  if (buttonValuesLast[0] != buttonValues[0])
    lastInput = startMs;
}

bool buttonPressed(int i){
  return (buttonValues[i] == false && buttonValuesLast[i] == true);
}

void doEncoder() {
  if (digitalRead(enc_clk) == digitalRead(enc_dt)) {
    encoderValue--;
  }
  else {
    encoderValue++;
  }
  lastInput = startMs;
  hideDisplay = false;
  doMenuUpdate = true;
}

void doDisplay() {
  switch(menuMode){
    case 0:
      mainMenu();
      break;
    case 1:
      // Sensitivity
      sensitivityMenu();
      break;
    case 2:
      // Options
      optionsMenu();
      break;
    case 3:
      // Diagnostics
      diagnosticsMenu();
      break;
    case 4:
      // About
      aboutMenu();
      break;
    
    default:
      badMenu();
      break;
  }
}

void doOption(int mn, int mx) {
  if (encoderValue < 0)
    encoderValue = mx - mn - 1;
    
  int old = menuOption;
  menuOption = mn + (encoderValue % (mx - mn));
  if (old != menuOption)
    doMenuUpdate = true;
}

void badMenu(){
  if (doMenuUpdate){
    u8g2.clearBuffer();
                         //////////////////////
    u8g2.drawStr(0, 0,  "GG's, you broke the");
    u8g2.drawStr(0, 12, "menu lmao. Let me");
    u8g2.drawStr(0, 24, "know how plz! Press");
    u8g2.drawStr(0, 36, "the button to return.");
    u8g2.sendBuffer();
    doMenuUpdate = false;
  }
  
  if (buttonPressed(0)){
    encoderValue = 0;
    menuMode = 0;
    doMenuUpdate = true;
  }
}

void mainMenu(){
  if (buttonPressed(0)){
    doMenuUpdate = true;
    encoderValue = 0;
    menuMode = menuOption + 1;
    switch(menuMode){
      case 1:
        cSens[0] = sens[0];
        cSens[1] = sens[1];
        cSens[2] = sens[2];
        cSens[3] = sens[3];
        break;
      case 5:
        menuMode = 0;
        hideDisplay = true;
        u8g2.clearBuffer();
        u8g2.sendBuffer();
        break;
    }
    return;
  }
  
  doOption(0, 5);

  if (doMenuUpdate){
    u8g2.clearBuffer();
    u8g2.drawStr(12, 0, "SENSITIVITY");
    u8g2.drawStr(12, 12, "OPTIONS");
    u8g2.drawStr(12, 24, "DIAGNOSTICS");
    u8g2.drawStr(12, 36, "ABOUT");
    u8g2.drawStr(12, 48, "HIDE DISPLAY");
    
    u8g2.drawStr(0, 12 * menuOption, ">");
    u8g2.sendBuffer();
    doMenuUpdate = false;
  }
}

void optionsMenu(){
  if (buttonPressed(0)){
    doMenuUpdate = true;
    switch(menuOption){
      case 0:
        if (!subMenu){
          encoderValue = timeOut;
          subMenu = true;
        } else {
          encoderValue = 0;
          subMenu = false;
        }
        break;
      case 1:
        multiHit = !multiHit;
        break;
      case 2:
        if (!subMenu){
          encoderValue = readDelay;
          subMenu = true;
        } else {
          encoderValue = 0;
          subMenu = false;
        }
        break;
      case 3:
        encoderValue = 1;
        menuMode = 0;
        EEPROM.update(timeOutAdr, timeOut);
        EEPROM.update(multiHitAdr, multiHit);
        EEPROM.update(readDelayAdr, readDelay);
        return;
    }
  }

  if (!subMenu){
    doOption(0, 4);
  } else {
    switch(menuOption){
      case 0: {
          encoderValue = constrain(encoderValue, 5, 30);
          byte curt = timeOut;
          timeOut = encoderValue;
          if (curt != timeOut)
            doMenuUpdate = true;
        }
        break;
      case 2: {
          encoderValue = constrain(encoderValue, 1, 255);
          int curd = readDelay;
          readDelay = encoderValue;
          if (curd != readDelay)
            doMenuUpdate = true;
        }
        break;
    }
  }

  if (doMenuUpdate){
    u8g2.clearBuffer();
    u8g2.drawStr(12, 0, ("DSP TIMEOUT: " + String(timeOut) + "S").c_str());
    u8g2.drawStr(12, 12, ("MULTI HIT: " + String(multiHit ? "ON" : "OFF")).c_str());
    u8g2.drawStr(12, 24, ("HIT DELAY: " + String(readDelay) + "MS").c_str());
    u8g2.drawStr(12, 36, "BACK");

    if (!subMenu){
      u8g2.drawStr(0, 12 * menuOption, ">");
    }else{
      u8g2.drawStr(0, 12 * menuOption, "=");
    }
    u8g2.sendBuffer();
    doMenuUpdate = false;
  }
}

void sensitivityMenu(){
  if (!subMenu){
    doOption(0, 3);
    if (buttonPressed(0)){
      encoderValue = 0;
      doMenuUpdate = true;
      switch (menuOption){
        case 0:
          encoderValue = sens[0] / 3;
          subMenu = true;
          break;
        case 1:
          // SAVE LOGIC
          for (int i = 0; i < 4; i++){
            EEPROM.update(senseAdr + (2 * i), sens[i] >> 8);
            EEPROM.update(senseAdr + (2 * i) + 1, sens[i]);
          }
          menuMode = 0;
          break;
        case 2:
          // EXIT NO SAVE LOGIC
          for (int i = 0; i < 4; i++){
            sens[i] = cSens[i];
          }
          menuMode = 0;
          break;
      }
    }
  }
  else {
    encoderValue = constrain(encoderValue, 0, 341);
    int curs = sens[menuOption];
    sens[menuOption] = encoderValue * 3;
    if (curs != sens[menuOption])
      doMenuUpdate = true;
      
    if (buttonPressed(0)){
      doMenuUpdate = true;
      menuOption++;
      if (menuOption > 3){
        encoderValue = 0;
        subMenu = false;
      }
      else {
        encoderValue = sens[menuOption] / 3;
      }
    }
  }

  if (doMenuUpdate){
    u8g2.clearBuffer();
    u8g2.drawBox(12, 0, (sens[0] / 1023.0) * 116, 12);
    u8g2.drawBox(12, 13, (sens[1] / 1023.0) * 116, 12);
    u8g2.drawBox(12, 26, (sens[2] / 1023.0) * 116, 12);
    u8g2.drawBox(12, 39, (sens[3] / 1023.0) * 116, 12);
  
    u8g2.setDrawColor(2);
    u8g2.setFontMode(1);
    u8g2.drawStr(14, 1, ("LKA: " + String(sens[0], DEC)).c_str());
    u8g2.drawStr(14, 14, ("LDO: " + String(sens[1], DEC)).c_str());
    u8g2.drawStr(14, 27, ("RDO: " + String(sens[2], DEC)).c_str());
    u8g2.drawStr(14, 40, ("RKA: " + String(sens[3], DEC)).c_str());
    u8g2.setDrawColor(1);
    u8g2.setFontMode(0);
    
    u8g2.drawStr(10, 55, "EDIT");
    u8g2.drawStr(48, 55, "SAVE");
    u8g2.drawStr(89, 55, "EXIT");

    if (!subMenu){
      u8g2.drawStr(40 * menuOption, 55, ">");
      u8g2.drawStr(40 + (40 * menuOption), 55, "<");
    } else {
      u8g2.drawStr(0, 1 + (13 * menuOption), ">");
    }
    
    u8g2.sendBuffer();
    doMenuUpdate = false;
  }
}

void diagnosticsMenu() {
  lastInput = startMs;
  
  int lka = sensors[0];
  int ldon = sensors[1];
  int rdon = sensors[2];
  int rka = sensors[3];
  
  if (lka >= sens[0] || ldon >= sens[1] || rdon >= sens[2] || rka >= sens[3])
    doMenuUpdate = true;
  
  if (doMenuUpdate){
    u8g2.clearBuffer();
    u8g2.drawStr(0, 0, ("LKA: " + String(lka)).c_str());
    u8g2.drawStr(0, 12, ("LDO: " + String(ldon)).c_str());
    u8g2.drawStr(0, 24, ("RDO: " + String(rdon)).c_str());
    u8g2.drawStr(0, 36, ("RKA: " + String(rka)).c_str());
    u8g2.sendBuffer();
    doMenuUpdate = false;
  }
  if (buttonPressed(0)){
    doMenuUpdate = true;
    encoderValue = 2;
    menuMode = 0;
  }
}

void aboutMenu(){
  if (doMenuUpdate){
    u8g2.clearBuffer();
                         //////////////////////
    u8g2.drawStr(0, 0,  "BlueSquid Interface,");
    u8g2.drawStr(0, 12, "design + software");
    u8g2.drawStr(0, 24, "Jaika★, 2022.");
    u8g2.drawStr(0, 36, "VER. 1.0.0");
    u8g2.sendBuffer();
    doMenuUpdate = false;
  }
  if (buttonPressed(0)){
    doMenuUpdate = true;
    encoderValue = 3;
    menuMode = 0;
  }
}
