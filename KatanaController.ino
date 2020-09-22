#define DEBUG
//#define DEBUG_VERBOSE

#include <USBHost_t36.h>
#include "defines.h"


bool bp_event[6] = {false};

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
MIDIDevice_BigBuffer midi1(myusb);

enum controlMode_t {effectsOnOff, presetSelect, menu3};
enum controlMode_t controlMode = effectsOnOff;

enum GRY_t {GREEN=0, RED=1, YELLOW=2};
enum GRY_t booster_gry, mod_gry, fx_gry, delay_gry, reverb_gry;

enum on_off_t {ON=1, OFF=0};
enum on_off_t booster_en, mod_en, fx_en, delay_en, reverb_en;

enum preset_t {Pannel = 0, Ch1 = 1, Ch2 = 2, Ch3 = 5, Ch4 = 6};
enum preset_t preset;

uint8_t vol_level;
enum on_off_t vol_mute = OFF;

char pannel_name[16], ch1_name[16], ch2_name[16], ch3_name[16], ch4_name[16];

byte checksum(byte const *data, byte dataLength) {
    byte sum = 0, i;

    for (i = 8; i < 12 + dataLength; i++) {
        sum = (sum + data[i]) & 0x7F;
    }

    return (128 - sum) & 0x7F;
}

void send_sysex(const unsigned long address, byte *data, byte dataLength, byte action) {
    byte sysex[14 + dataLength] = {0};

    sysex[0] = SYSEX_START;
    memcpy(sysex + 1, SYSEX_HEADER, 7);
    sysex[7] = action;
    sysex[8] = (byte)(address >> 24);
    sysex[9] = (byte)(address >> 16);
    sysex[10] = (byte)(address >> 8);
    sysex[11] = (byte)(address);
    memcpy(sysex + 12, data, dataLength);
    sysex[12 + dataLength] = checksum(sysex, dataLength);
    sysex[13 + dataLength] = SYSEX_END;

    midi1.sendSysEx(14 + dataLength, sysex, true);
    midi1.send_now();
    delay(100);
}

void setEditorMode(bool active = true) {
    byte data[1] = {(active) ? (byte) 0x01 : (byte) 0x00};
    send_sysex(P_EDIT, data, 1, SYSEX_WRITE);    
}

void isr_bp(uint8_t bp) {
  static unsigned long last_event[6] = {0};

  if (millis() - last_event[bp] > 1000) {
    bp_event[bp-1] = true;
    last_event[bp] = millis();
  }
}

void isr_bp1() {
  isr_bp(1);
}

void isr_bp2() {
  isr_bp(2);
}

void isr_bp3() {
  isr_bp(3);
}

void isr_bp4() {
  isr_bp(4);
}

void isr_bp5() {
  isr_bp(5);
}

void isr_bp6() {
  isr_bp(6);
}

void println(enum on_off_t s) { 
  if(s == ON)
    println("ON");
  else
    println("OFF");
}

void println(enum GRY_t s, int b = DEC) { 
  if (s == GREEN)
    println("GREEN");
  else if (s == RED)
    println("RED");
  else
    println("YELLOW");
}

void println(enum preset_t s, int b = DEC) { 
  if (s == Pannel)
    println("Pannel");
  else if (s == Ch1)
    println("Ch1");
  else if (s == Ch2)
    println("Ch2");
  else if (s == Ch3)
    println("Ch3");
  else if (s == Ch4)
    println("Ch4");
  else
    println((int)s, DEC);
}

void handleSysEx(const byte* sysExData, uint16_t sysExSize, bool complete) {
  //simple test to see if the complete message is available
  if(complete){
    unsigned long addr = sysExData[8]<<24 | sysExData[9]<<16 | sysExData[10]<<8 | sysExData[11];

    print_("Received ");
    print_(sysExSize);
    println_(" bytes");
    print_("Addr : 0x");
    println_(addr, HEX);
    print_("Data : ");

    for (uint16_t i=12;i<sysExSize-2;i++){
      print_(sysExData[i],HEX);
      print_(" ");
    }
    println_("");

    switch(addr){
     case EN_BOOSTER:
       booster_en = sysExData[12] ? ON : OFF;
       print("EN_BOOSTER : ");
       println(booster_en);
       break;
     case EN_MOD:
       mod_en = sysExData[12] ? ON : OFF;
       print("EN_MOD : ");
       println(mod_en);
       break;
     case EN_FX:
       fx_en = sysExData[12] ? ON : OFF;
       print("EN_FX : ");
       println(fx_en);
       break;
     case EN_DELAY:
       delay_en = sysExData[12] ? ON : OFF;
       print("EN_DELAY : ");
       println(delay_en);
       break;
     case EN_REVERB:
       reverb_en = sysExData[12] ? ON : OFF;
       print("EN_REVERB : ");
       println(reverb_en);
       break;
       
     case GRY_BOOSTER:
       booster_gry = (GRY_t)sysExData[12];
       print("GRY_BOOSTER : ");
       println(booster_gry);
       break;
     case GRY_MOD:
       mod_gry = (GRY_t)sysExData[12];
       print("GRY_MOD : ");
       println(mod_gry);
       break;
     case GRY_FX:
       fx_gry = (GRY_t)sysExData[12];
       print("GRY_FX : ");
       println(fx_gry);
       break;
     case GRY_DELAY:
       delay_gry = (GRY_t)sysExData[12];
       print("GRY_DELAY : ");
       println(delay_gry);
       break;
     case GRY_REVERB:
       reverb_gry = (GRY_t)sysExData[12];
       print("GRY_REVERB : ");
       println(reverb_gry);
       break;
       
     case PRESET:
       preset = (preset_t)sysExData[13];
       print("PRESET : ");
       println(preset);
       break;
     
     case PANNEL_NAME:
       memcpy(pannel_name, sysExData+12, 16);
       print("Pannel name : ");
       println(pannel_name);
       break;
     case CH1_NAME:
       memcpy(ch1_name, sysExData+12, 16);
       print("Ch1 name : ");
       println(ch1_name);
       break;
     case CH2_NAME:
       memcpy(ch2_name, sysExData+12, 16);
       print("Ch2 name : ");
       println(ch2_name);
       break;
     case CH3_NAME:
       memcpy(ch3_name, sysExData+12, 16);
       print("Ch3 name : ");
       println(ch3_name);
       break;
     case CH4_NAME:
       memcpy(ch4_name, sysExData+12, 16);
       print("Ch4 name : ");
       println(ch4_name);
       break;

     case VOLUME:
       vol_level = sysExData[12];
       print("Volume level : ");
       println(vol_level, HEX);
       break;
    }
  }   
}

void recallPreset(uint8_t preset) {
  byte data[2] = {0};
  data[1] = preset;
  send_sysex(PRESET, data, 2, SYSEX_WRITE);
  vol_mute = OFF;
  delay(200);

  readEffectStatus();
}

void readEffectStatus() {
  byte data[4] = {0};
  
  data[3] = 1;
  send_sysex(EN_BOOSTER, data, 4, SYSEX_READ);
  send_sysex(EN_MOD, data, 4, SYSEX_READ);
  send_sysex(EN_FX, data, 4, SYSEX_READ);
  send_sysex(EN_DELAY, data, 4, SYSEX_READ);
  send_sysex(EN_REVERB, data, 4, SYSEX_READ);

  send_sysex(GRY_BOOSTER, data, 4, SYSEX_READ);
  send_sysex(GRY_MOD, data, 4, SYSEX_READ);
  send_sysex(GRY_FX, data, 4, SYSEX_READ);
  send_sysex(GRY_DELAY, data, 4, SYSEX_READ);
  send_sysex(GRY_REVERB, data, 4, SYSEX_READ);

  if(vol_mute == OFF)
    send_sysex(VOLUME, data, 4, SYSEX_READ);  
}

void readFullStatus() {
  byte data[4] = {0};

  readEffectStatus();

  data[3] = 2;
  send_sysex(PRESET, data, 4, SYSEX_READ);

  data[3] = 16;
  send_sysex(PANNEL_NAME, data, 4, SYSEX_READ);
  send_sysex(CH1_NAME, data, 4, SYSEX_READ);
  send_sysex(CH2_NAME, data, 4, SYSEX_READ);
  send_sysex(CH3_NAME, data, 4, SYSEX_READ);
  send_sysex(CH4_NAME, data, 4, SYSEX_READ);
}

void initKatana() {
  println("Waiting for Katana to connect");
  while(!midi1);

  midi1.setHandleSysEx(handleSysEx);
  
  delay(1000);

  char* manufacturer = (char*)midi1.manufacturer();
  char* product = (char*)midi1.product();

  print("Vendor : ");
  print(manufacturer);
  print(" (0x");
  print(midi1.idVendor(), HEX);
  println(")");

  print("Product : ");
  print(product);
  print(" (0x");
  print(midi1.idProduct(), HEX);
  println(")");
  
  setEditorMode(true);
  setEditorMode(true);
  setEditorMode(true);

  readFullStatus();

  println("Ready");
}

void setup() {
  pinMode(BP_1, INPUT_PULLUP);
  pinMode(BP_2, INPUT_PULLUP);
  pinMode(BP_3, INPUT_PULLUP);
  pinMode(BP_4, INPUT_PULLUP);
  pinMode(BP_5, INPUT_PULLUP);
  pinMode(BP_6, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BP_1), isr_bp1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_2), isr_bp2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_3), isr_bp3, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_4), isr_bp4, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_5), isr_bp5, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_6), isr_bp6, FALLING);

#ifdef DEBUG
  Serial.begin(115200);
#endif
  delay(1000);
  myusb.begin();
}

void loop() {
  
  if (!midi1)
    initKatana();
    
  myusb.Task();
  midi1.read();

  if (bp_event[0]) {
    println_("BP1");
    bp_event[0] = false;

    if (controlMode == effectsOnOff) {
      booster_en = (booster_en==ON) ? OFF : ON;
      byte data[1] = {(booster_en==ON) ? (byte)1 : (byte)0};
      send_sysex(EN_BOOSTER, data, 1, SYSEX_WRITE);
      print("EN_BOOSTER : ");
      println(booster_en);
    } else if (controlMode == presetSelect) {
      preset = Pannel;
      recallPreset(preset);
    } else if (controlMode == menu3) {
      if (vol_mute == ON) {
        vol_mute = OFF;
        byte data[1] = {(byte)vol_level};
        send_sysex(VOLUME, data, 1, SYSEX_WRITE);
        println("Unmutted");
      } else {
        vol_mute = ON;
        byte data[1] = {(byte)0};
        send_sysex(VOLUME, data, 1, SYSEX_WRITE);
        println("Muted");
      }
    }
  }

  if (bp_event[1]) {
    println_("BP2");
    bp_event[1] = false;

    if (controlMode == effectsOnOff) {
      mod_en = (mod_en==ON) ? OFF : ON;
      byte data[1] = {(mod_en==ON) ? (byte)1 : (byte)0};
      send_sysex(EN_MOD, data, 1, SYSEX_WRITE);
      print("EN_MOD : ");
      println(mod_en);
    } else if (controlMode == presetSelect) {
      preset = Ch1;
      recallPreset(preset);
    }
  }

  if (bp_event[2]) {
    println_("BP3");
    bp_event[2] = false;

    if (controlMode == effectsOnOff) {
      fx_en = (fx_en==ON) ? OFF : ON;
      byte data[1] = {(fx_en==ON) ? (byte)1 : (byte)0};
      send_sysex(EN_FX, data, 1, SYSEX_WRITE);
      print("EN_FX : ");
      println(fx_en);
    } else if (controlMode == presetSelect) {
      preset = Ch2;
      recallPreset(preset);
    }
  }

  if (bp_event[3]) {
    println_("BP4");
    bp_event[3] = false;
    
    if (controlMode == effectsOnOff) {
      delay_en = (delay_en==ON) ? OFF : ON;
      byte data[1] = {(delay_en==ON) ? (byte)1 : (byte)0};
      send_sysex(EN_DELAY, data, 1, SYSEX_WRITE);
      print("EN_DELAY : ");
      println(delay_en);
    } else if (controlMode == presetSelect) {
      preset = Ch3;
      recallPreset(preset);
    }
  }

  if (bp_event[4]) {
    println_("BP5");
    bp_event[4] = false;

    if (controlMode == effectsOnOff) {
      reverb_en = (reverb_en==ON) ? OFF : ON;
      byte data[1] = {(reverb_en==ON) ? (byte)1 : (byte)0};
      send_sysex(EN_REVERB, data, 1, SYSEX_WRITE);
      print("EN_REVERB : ");
      println(reverb_en);
    } else if (controlMode == presetSelect) {
      preset = Ch4;
      recallPreset(preset);
    }
  }

  if (bp_event[5]) {
    println_("BP6");
    bp_event[5] = false;

    if(controlMode == effectsOnOff) {
      controlMode = presetSelect;
      println("controlMode : presetSelect");
    } else if(controlMode == presetSelect) {
      controlMode = menu3;      
      println("controlMode : menu3");
    } else if(controlMode == menu3) {
      controlMode = effectsOnOff;      
      println("controlMode : effectsOnOff");
    }
  }

}
