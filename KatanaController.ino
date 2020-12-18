//#define DEBUG
//#define DEBUG_VERBOSE

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Adafruit_SSD1306.h>
#include <USBHost_t36.h>
#include "defines.h"


LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_WIDTH,LCD_HEIGHT);

Adafruit_SSD1306 oled[] = {
  Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire2, -1),
  Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire1, -1),
  Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire2, -1),
  Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire1, -1),
  Adafruit_SSD1306(OLED_WIDTH, OLED_HEIGHT, &Wire, -1)};

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
MIDIDevice_BigBuffer midi1(myusb);

typedef enum {effectsOnOff = 0, presetSelect = 1, ampTypeSelect = 2} controlMode_t;
typedef enum {GREEN = 0, RED = 1, YELLOW = 2} GRY_t;
typedef enum {ON = 1, OFF = 0} on_off_t;
typedef enum {Pannel = 0, Ch1 = 1, Ch2 = 2, Ch3 = 5, Ch4 = 6} preset_t;
typedef enum {Acoustic = 0, Clean = 1, Crunch = 2, Lead = 3, Brown = 4} amp_type_t;
typedef enum {MidBST = 0, CleanBST = 1, TrebleBST = 2, CrunchOD = 3, NaturalOD = 4, WarmOD = 5, FatDS = 6, MetalDS = 8, OctFuzz = 9, BluesDrive = 10, Overdrive = 11, TScream = 12, TurboOD = 13, Distortion = 14, Rat = 15, GuvDS = 16, DSTp = 17, MetalZone = 18, SixtiesFuzz = 19, MuffFuzz = 20} booster_type_t;

controlMode_t controlMode = effectsOnOff;
GRY_t booster_gry, mod_gry, fx_gry, delay_gry, reverb_gry;
on_off_t booster_en, mod_en, fx_en, delay_en, reverb_en;
preset_t preset;
amp_type_t amp_type;
booster_type_t booster_type;
uint8_t vol_level;
char pannel_name[16], ch1_name[16], ch2_name[16], ch3_name[16], ch4_name[16] = {0};
on_off_t vol_mute = OFF;
uint8_t madeEdits = 0;

uint8_t bp_event[NB_BP] = {0};
uint8_t bp_menu_event = 0;
uint8_t bp_mute_event = 0;


void led_off(uint8_t io) {
  pinMode(io, INPUT);
}

void led_on(uint8_t io) {
  pinMode(io, OUTPUT);
  digitalWrite(io, LOW);
}

uint8_t checksum(const uint8_t *data, uint8_t dataLength) {
    uint8_t sum = 0, i;

    for (i = 8; i < 12 + dataLength; i++) {
        sum = (sum + data[i]) & 0x7F;
    }

    return (128 - sum) & 0x7F;
}

void send_sysex(const unsigned long address, uint8_t *data, uint8_t dataLength, uint8_t action, uint8_t nbPacketExpectedInReturn = 0) {
    uint8_t sysex[14 + dataLength] = {0};

    sysex[0] = SYSEX_START;
    memcpy(sysex + 1, SYSEX_HEADER, 7);
    sysex[7] = action;
    sysex[8] = (uint8_t)(address >> 24);
    sysex[9] = (uint8_t)(address >> 16);
    sysex[10] = (uint8_t)(address >> 8);
    sysex[11] = (uint8_t)(address);
    memcpy(sysex + 12, data, dataLength);
    sysex[12 + dataLength] = checksum(sysex, dataLength);
    sysex[13 + dataLength] = SYSEX_END;

    do {
      midi1.sendSysEx(14 + dataLength, sysex, true);
      midi1.send_now();
    } while(expectMidiPackets(nbPacketExpectedInReturn) != 0);
}

void setEditorMode(uint8_t active) {
    uint8_t data[1];
    data[0] = (active) ? 1 : 0;
    send_sysex(P_EDIT, data, 1, SYSEX_WRITE);    
}

void isr_bp(uint8_t bp) {
  static uint32_t last_event[NB_BP] = {0};

  if (millis() - last_event[bp] > BP_DEBOUNCE_MS) {
    bp_event[bp] = 1;
    last_event[bp] = millis();
  }
}

void isr_bp1() {
  isr_bp(0);
}

void isr_bp2() {
  isr_bp(1);
}

void isr_bp3() {
  isr_bp(2);
}

void isr_bp4() {
  isr_bp(3);
}

void isr_bp5() {
  isr_bp(4);
}

void isr_bp_menu() {
  static uint32_t last_event = 0;

  if (millis() - last_event > BP_DEBOUNCE_MS) {
    bp_menu_event = 1;
    last_event = millis();
  }
}

void isr_bp_mute() {
  static uint32_t last_event = 0;

  if (millis() - last_event > BP_DEBOUNCE_MS) {
    bp_mute_event = 1;
    last_event = millis();
  }
}

const char * toString(on_off_t s) { 
  if(s == ON)
    return "ON";
  else
    return "OFF";
}

const char * toString(GRY_t s) { 
  if (s == GREEN)
    return "Green";
  else if (s == RED)
    return "Red";
  else if (s == YELLOW)
    return "Yellow";
  else
    return NULL;
}

const char * toString(preset_t preset) {
  if(preset == Pannel)
    return "Pannel";
  else if(preset == Ch1)
    return "A-Ch1 ";
  else if(preset == Ch2)
    return "A-Ch2 ";
  else if(preset == Ch3)
    return "B-Ch1 ";
  else if(preset == Ch4)
    return "B-Ch2 ";
  else
    return NULL;
}

const char * toString(amp_type_t s) { 
  if (s == Acoustic)
    return "Acoustic";
  else if (s == Clean)
    return "Clean   ";
  else if (s == Crunch)
    return "Crunch  ";
  else if (s == Lead)
    return "Lead    ";
  else if (s == Brown)
    return "Brown   ";
  else
    return NULL;
}

const char * toString(booster_type_t s) {
  if (s == MidBST)
    return "MidBST";
  else if (s == CleanBST)
    return "CleanBST";
  else if (s == TrebleBST)
    return "TrebleBST";
  else if (s == CrunchOD)
    return "CrunchOD";
  else if (s == NaturalOD)
    return "NaturalOD";
  else if (s == WarmOD)
    return "WarmOD";
  else if (s == FatDS)
    return "FatDS";
  else if (s == MetalDS)
    return "MetalDS";
  else if (s == OctFuzz)
    return "OctFuzz";
  else if (s == BluesDrive)
    return "BluesDrive";
  else if (s == Overdrive)
    return "Overdrive";
  else if (s == TScream)
    return "TScream";
  else if (s == TurboOD)
    return "TurboOD";
  else if (s == Distortion)
    return "Distortion";
  else if (s == Rat)
    return "Rat";
  else if (s == GuvDS)
    return "GuvDS";
  else if (s == DSTp)
    return "DST+";
  else if (s == MetalZone)
    return "MetalZone";
  else if (s == SixtiesFuzz)
    return "60sFuzz";
  else if (s == MuffFuzz)
    return "MuffFuzz";
  else
    return NULL;
}

void handleSysEx(const uint8_t* sysExData, uint16_t sysExSize, bool complete) {
  //simple test to see if the complete message is available
  if(complete){
    uint32_t addr = sysExData[8]<<24 | sysExData[9]<<16 | sysExData[10]<<8 | sysExData[11];

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

    if(preset != Pannel)
      madeEdits = 1;
    
    switch(addr){
     case EN_BOOSTER:
     case R_BOOSTER:
       booster_en = sysExData[12] ? ON : OFF;
       print("EN_BOOSTER : ");
       println(toString(booster_en));
       break;
     case EN_MOD:
     case R_MOD:
       mod_en = sysExData[12] ? ON : OFF;
       print("EN_MOD : ");
       println(toString(mod_en));
       break;
     case EN_FX:
     case R_FX:
       fx_en = sysExData[12] ? ON : OFF;
       print("EN_FX : ");
       println(toString(fx_en));
       break;
     case EN_DELAY:
     case R_DELAY:
       delay_en = sysExData[12] ? ON : OFF;
       print("EN_DELAY : ");
       println(toString(delay_en));
       break;
     case EN_REVERB:
     case R_REVERB:
       reverb_en = sysExData[12] ? ON : OFF;
       print("EN_REVERB : ");
       println(toString(reverb_en));
       break;
       
     case GRY_BOOSTER:
       booster_gry = (GRY_t)sysExData[12];
       print("GRY_BOOSTER : ");
       println(toString(booster_gry));
       break;
     case GRY_MOD:
       mod_gry = (GRY_t)sysExData[12];
       print("GRY_MOD : ");
       println(toString(mod_gry));
       break;
     case GRY_FX:
       fx_gry = (GRY_t)sysExData[12];
       print("GRY_FX : ");
       println(toString(fx_gry));
       break;
     case GRY_DELAY:
       delay_gry = (GRY_t)sysExData[12];
       print("GRY_DELAY : ");
       println(toString(delay_gry));
       break;
     case GRY_REVERB:
       reverb_gry = (GRY_t)sysExData[12];
       print("GRY_REVERB : ");
       println(toString(reverb_gry));
       break;
       
     case PRESET:
       preset = (preset_t)sysExData[13];
       print("PRESET : ");
       println(toString(preset));

       if(preset == Pannel)
         expectMidiPackets(11);
       else
         expectMidiPackets(6);
            
       vol_mute = OFF;
       readEffectStatus();
      
       madeEdits = 0;
       refreshScreen();
       break;
     
     case PANNEL_NAME:
       memcpy(pannel_name, sysExData+12, 15);
       print("Pannel name : ");
       println(pannel_name);
       break;
     case CH1_NAME:
       memcpy(ch1_name, sysExData+12, 15);
       print("Ch1 name : ");
       println(ch1_name);
       break;
     case CH2_NAME:
       memcpy(ch2_name, sysExData+12, 15);
       print("Ch2 name : ");
       println(ch2_name);
       break;
     case CH3_NAME:
       memcpy(ch3_name, sysExData+12, 15);
       print("Ch3 name : ");
       println(ch3_name);
       break;
     case CH4_NAME:
       memcpy(ch4_name, sysExData+12, 15);
       print("Ch4 name : ");
       println(ch4_name);
       break;

     case VOLUME:
       vol_level = sysExData[12];
       print("Volume level : ");
       println(vol_level, HEX);
       break;

     case AMP_TYPE:
       amp_type = (amp_type_t)sysExData[12];
       print("Amp type : ");
       println(toString(amp_type));
       break;

     case BOOSTER_TYPE:
       booster_type = (booster_type_t)sysExData[12];
       print("Booster type : ");
       println(toString(booster_type));
       break;
    }
  }   
}

uint8_t expectMidiPackets(uint8_t expectedPackets) {
  if (expectedPackets == 0)
    return 0;
    
  print_("Expects ");
  print_(expectedPackets);
  println_(" MIDI packets");

  uint32_t timeout = millis()+1000;
  
  while(expectedPackets != 0 && millis() < timeout) {
    myusb.Task();
    if(midi1.read()) {
      expectedPackets--;
    }
  }

  if (expectedPackets == 0)
    println_("All expected packets have now been received");
  else
    println_("Not all expected packets have been received, timeout");
    
  return expectedPackets;
}

void recallPreset(uint8_t preset) {
  uint8_t data[2] = {0};
  data[1] = preset;
  send_sysex(PRESET, data, 2, SYSEX_WRITE, 1);
}

void readEffectStatus() {
  uint8_t data[4] = {0};
  
  data[3] = 1;
  send_sysex(EN_BOOSTER, data, 4, SYSEX_READ, 1);
  send_sysex(EN_MOD, data, 4, SYSEX_READ, 1);
  send_sysex(EN_FX, data, 4, SYSEX_READ, 1);
  send_sysex(EN_DELAY, data, 4, SYSEX_READ, 1);
  send_sysex(EN_REVERB, data, 4, SYSEX_READ, 1);

  send_sysex(GRY_BOOSTER, data, 4, SYSEX_READ, 1);
  send_sysex(GRY_MOD, data, 4, SYSEX_READ, 1);
  send_sysex(GRY_FX, data, 4, SYSEX_READ, 1);
  send_sysex(GRY_DELAY, data, 4, SYSEX_READ, 1);
  send_sysex(GRY_REVERB, data, 4, SYSEX_READ, 1);

  send_sysex(AMP_TYPE, data, 4, SYSEX_READ, 1);

  send_sysex(BOOSTER_TYPE, data, 4, SYSEX_READ, 1);

  if(vol_mute == OFF)
    send_sysex(VOLUME, data, 4, SYSEX_READ, 1);  
}

void readFullStatus() {
  uint8_t data[4] = {0};

  data[3] = 16;
  send_sysex(PANNEL_NAME, data, 4, SYSEX_READ, 1);
  send_sysex(CH1_NAME, data, 4, SYSEX_READ, 1);
  send_sysex(CH2_NAME, data, 4, SYSEX_READ, 1);
  send_sysex(CH3_NAME, data, 4, SYSEX_READ, 1);
  send_sysex(CH4_NAME, data, 4, SYSEX_READ, 1);

  data[3] = 2;
  send_sysex(PRESET, data, 4, SYSEX_READ, 1);
  
  madeEdits = 0;
}

void refreshScreen() {
  lcd.clear();
  lcd.setCursor(0,0);

  for(int i = 0; i<5; i++)
    oled[i].clearDisplay();
    
  if (!midi1) {
    lcd.setCursor(0,1);
    lcd.print(" Waiting for Katana ");
    lcd.setCursor(0,2);
    lcd.print("     to connect     ");
  
  } else {
  
    if(madeEdits)
      lcd.print("*");
    else
      lcd.print(" ");
  
    if(preset == Pannel)
      lcd.print(pannel_name);
    else if(preset == Ch1)
      lcd.print(ch1_name);
    else if(preset == Ch2)
      lcd.print(ch2_name);
    else if(preset == Ch3)
      lcd.print(ch3_name);
    else if(preset == Ch4)
      lcd.print(ch4_name);
    
    if(vol_mute == ON)
      lcd.print("MUTE");
    else
      lcd.print("    ");
  
    lcd.setCursor(0,1);
    if(controlMode == presetSelect)
      lcd.print("> ");
    else
      lcd.print("  ");
    lcd.print(toString(preset));
    lcd.print("  ");
    if(controlMode == ampTypeSelect)
      lcd.print("> ");
    else
      lcd.print("  ");
    lcd.print(toString(amp_type));

    if (controlMode == effectsOnOff) {
      for(int i = 0; i<5; i++) {
        oled[i].setCursor(0,0);
        oled[i].setTextSize(4);
      }
      oled[0].println("Boost");
      oled[1].println("Mod");
      oled[2].println("FX");
      oled[3].println("Delay");
      oled[4].println("Rev");

      for(int i = 0; i<5; i++) {
        oled[i].setTextSize(2);
      }
      oled[0].println(toString(booster_type));
      oled[1].println("?");
      oled[2].println("?");
      oled[3].println("?");
      oled[4].println("?");

      if (booster_en == ON)
        oled[0].println(toString(booster_gry));
      if (mod_en == ON)
        oled[1].println(toString(mod_gry));
      if (fx_en == ON)
        oled[2].println(toString(fx_gry));
      if (delay_en == ON)
        oled[3].println(toString(delay_gry));
      if (reverb_en == ON)
        oled[4].println(toString(reverb_gry));
        
    } else if (controlMode == presetSelect) {
      for(int i = 0; i<5; i++) {
        oled[i].setCursor(0,0);
        oled[i].setTextSize(2);
      }
      oled[0].setTextSize(3);
      oled[0].println("Pannel");
      oled[1].println(ch1_name);
      oled[2].println(ch2_name);
      oled[3].println(ch3_name);
      oled[4].println(ch4_name);
      if (preset <= 2) {
        oled[preset].setTextSize(3);
        oled[preset].println("   *");
      } else {
        oled[preset-2].setTextSize(3);
        oled[preset-2].println("   *");
      }
      
    } else if (controlMode == ampTypeSelect) {
      for(int i = 0; i<5; i++) {
        oled[i].setCursor(0,0);
        oled[i].setTextSize(3);
      }
      oled[0].println("Acousti");
      oled[1].println("Clean");
      oled[2].println("Crunch");
      oled[3].println("Leed");
      oled[4].println("Brown");    
      oled[amp_type].println("   *");
    }
  }

  for(int i = 0; i<5; i++)
    oled[i].display();
}

void initKatana() {
  println("Waiting for Katana to connect");
  refreshScreen();
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
  
  setEditorMode(1);
  setEditorMode(1);
  setEditorMode(1);

  readFullStatus();
  refreshScreen();

  println("Ready");
}

void init_oled() {
  for(int i = 0; i<5; i++) {
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if(!oled[i].begin(SSD1306_SWITCHCAPVCC, (i<=1) ? OLED01_ADDRESS : OLED234_ADDRESS)) { 
      Serial.print("OLED ");
      Serial.print(i);
      Serial.println(" allocation failed");
      for(;;); // Don't proceed, loop forever
    }

    // Clear the buffer
    oled[i].clearDisplay();
    oled[i].setTextColor(WHITE);
  }
}

void setup() {

  led_off(LED_MUTE);
  led_off(LED_MENU);
  led_off(LED_BP_1);
  led_off(LED_BP_2);
  led_off(LED_BP_3);
  led_off(LED_BP_4);
  led_off(LED_BP_5);

  pinMode(BP_1, INPUT_PULLUP);
  pinMode(BP_2, INPUT_PULLUP);
  pinMode(BP_3, INPUT_PULLUP);
  pinMode(BP_4, INPUT_PULLUP);
  pinMode(BP_5, INPUT_PULLUP);
  pinMode(BP_MENU, INPUT_PULLUP);
  pinMode(BP_MUTE, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(BP_1), isr_bp1, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_2), isr_bp2, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_3), isr_bp3, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_4), isr_bp4, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_5), isr_bp5, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_MENU), isr_bp_menu, FALLING);
  attachInterrupt(digitalPinToInterrupt(BP_MUTE), isr_bp_mute, FALLING);

  lcd.init();
  lcd.init();
  lcd.backlight();

  init_oled();

#ifdef DEBUG
  Serial.begin(115200);
#endif
  delay(1000);
  myusb.begin();
}

void muteToggle() {
  uint8_t data[1];
  uint8_t madeEditsBak = madeEdits;
  if (vol_mute == ON) {
    vol_mute = OFF;
    data[0] = {vol_level};
    send_sysex(VOLUME, data, 1, SYSEX_WRITE, 1);
    println("Unmutted");
  } else {
    vol_mute = ON;
    data[0] = {0};
    send_sysex(VOLUME, data, 1, SYSEX_WRITE, 1);
    println("Muted");
  }
  madeEdits = madeEditsBak;
  refreshScreen();  
}

void ampTypeSelectEvent(uint8_t bp) {
  if (bp != amp_type) {
    uint8_t data[1] = {bp};
    send_sysex(AMP_TYPE, data, 1, SYSEX_WRITE, 1);
    amp_type = (amp_type_t)bp;
    madeEdits = 1;
    refreshScreen();
  }
}

void presetSelectEvent(uint8_t bp) {
  if (bp == 0)
    preset = Pannel;
  else if (bp == 1)
    preset = Ch1;
  else if (bp == 2)
    preset = Ch2;
  else if (bp == 3)
    preset = Ch3;
  else if (bp == 4)
    preset = Ch4;
          
  recallPreset(preset);
}

void effectsOnOffEvent(uint8_t bp) {
  uint8_t data[1];
  if (bp == 0) {
    booster_en = (booster_en==ON) ? OFF : ON;
    data[0] = (booster_en==ON) ? 1 : 0;
    send_sysex(EN_BOOSTER, data, 1, SYSEX_WRITE, 1);
  } else if (bp == 1) {
    mod_en = (mod_en==ON) ? OFF : ON;
    data[0] = (mod_en==ON) ? 1 : 0;
    send_sysex(EN_MOD, data, 1, SYSEX_WRITE, 1);
  } else if (bp == 2) {
    fx_en = (fx_en==ON) ? OFF : ON;
    data[0] = (fx_en==ON) ? 1 : 0;
    send_sysex(EN_FX, data, 1, SYSEX_WRITE, 1);
  } else if (bp == 3) {
    delay_en = (delay_en==ON) ? OFF : ON;
    data[0] = (delay_en==ON) ? 1 : 0;
    send_sysex(EN_DELAY, data, 1, SYSEX_WRITE, 1);
  } else if (bp == 4) {
    reverb_en = (reverb_en==ON) ? OFF : ON;
    data[0] = (reverb_en==ON) ? 1 : 0;
    send_sysex(EN_REVERB, data, 1, SYSEX_WRITE, 1);
  }
  madeEdits = 1;
  refreshScreen();
}

void loop() {
  
  if (!midi1)
    initKatana();

  myusb.Task();
  if(midi1.read()) {
    refreshScreen();

  } else {

    for (uint8_t i = 0; i < NB_BP; i++) {
      if (bp_event[i]) {
        print_("BP");
        println_(i+1);
        bp_event[i] = 0;

        if (controlMode == effectsOnOff)
          effectsOnOffEvent(i);
        
        else if (controlMode == presetSelect)
          presetSelectEvent(i);

        else if (controlMode == ampTypeSelect) 
          ampTypeSelectEvent(i);
      }  
    }

    if (bp_mute_event) {
      println_("BP_MUTE");
      bp_mute_event = 0;
      muteToggle();
    }
    
    if (bp_menu_event) {
      println_("BP_MENU");
      bp_menu_event = 0;
  
      if(controlMode == effectsOnOff) {
        controlMode = presetSelect;
        println("controlMode : presetSelect");
      } else if(controlMode == presetSelect) {
        controlMode = ampTypeSelect;      
        println("controlMode : ampTypeSelect");
      } else if(controlMode == ampTypeSelect) {
        controlMode = effectsOnOff;      
        println("controlMode : effectsOnOff");
      }
      refreshScreen();
    }
  }

  if(vol_mute == ON)
    led_on(LED_MUTE);
  else
    led_off(LED_MUTE);

  if (controlMode == effectsOnOff) {
    if (booster_en)
      led_on(LED_BP_1);
    else
      led_off(LED_BP_1);
    
    if (mod_en)
      led_on(LED_BP_2);
    else
      led_off(LED_BP_2);
    
    if (fx_en)
      led_on(LED_BP_3);
    else
      led_off(LED_BP_3);
    
    if (delay_en)
      led_on(LED_BP_4);
    else
      led_off(LED_BP_4);
        
    if (reverb_en)
      led_on(LED_BP_5);
    else
      led_off(LED_BP_5);
  
  } else if(controlMode == presetSelect) {
    if (preset == Pannel)
      led_on(LED_BP_1);
    else
      led_off(LED_BP_1);
    
    if (preset == Ch1)
      led_on(LED_BP_2);
    else
      led_off(LED_BP_2);
    
    if (preset == Ch2)
      led_on(LED_BP_3);
    else
      led_off(LED_BP_3);
    
    if (preset == Ch3)
      led_on(LED_BP_4);
    else
      led_off(LED_BP_4);
        
    if (preset == Ch4)
      led_on(LED_BP_5);
    else
      led_off(LED_BP_5);
      
  } else if(controlMode == ampTypeSelect) {
    if (amp_type == Acoustic)
      led_on(LED_BP_1);
    else
      led_off(LED_BP_1);
    
    if (amp_type == Clean)
      led_on(LED_BP_2);
    else
      led_off(LED_BP_2);
    
    if (amp_type == Crunch)
      led_on(LED_BP_3);
    else
      led_off(LED_BP_3);
    
    if (amp_type == Lead)
      led_on(LED_BP_4);
    else
      led_off(LED_BP_4);
        
    if (amp_type == Brown)
      led_on(LED_BP_5);
    else
      led_off(LED_BP_5);

  }
      
}
