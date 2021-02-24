#ifdef DEBUG_VERBOSE
  #define DEBUG
  void print_(const char s[]) { Serial.print(s); }
  void println_(const char s[]) { Serial.println(s); }
  void print_(int s, int b = DEC) { Serial.print(s, b); }
  void println_(int s, int b = DEC) { Serial.println(s, b); }
#else
  void print_(const char *s) { }
  void println_(const char *s) { }
  void print_(int s, int b = DEC) { }
  void println_(int s, int b = DEC) { }
#endif

#ifdef DEBUG
  void print(const char s[]) { Serial.print(s); }
  void println(const char s[]) { Serial.println(s); }
  void print(int s, int b = DEC) { Serial.print(s, b); }
  void println(int s, int b = DEC) { Serial.println(s, b); }
#else
  void print(const char *s) { }
  void println(const char *s) { }
  void print(int s, int b = DEC) { }
  void println(int s, int b = DEC) { }
#endif

#define MAIN_MENU_SIZE ((int)(sizeof(mainMenu)/sizeof(mainMenu[0])))

#define BP_DEBOUNCE_MS 1000

#define NB_BP 7
#define BP_0 34
#define BP_1 35
#define BP_2 36
#define BP_3 37
#define BP_4 38
#define BP_5 33
#define BP_6 39

#define LED_BP_0 7
#define LED_BP_1 6
#define LED_BP_2 5
#define LED_BP_3 4
#define LED_BP_4 3
#define LED_BP_5 8
#define LED_BP_6 2

#define LCD_ADDRESS 0x27
#define LCD_WIDTH   20 // LCD display width, in char
#define LCD_HEIGHT  4  // LCD display height, in char

#define OLED01_ADDRESS 0x3D
#define OLED234_ADDRESS 0x3C
#define OLED_WIDTH   128 // OLED display width, in pixels
#define OLED_HEIGHT  64  // OLED display height, in pixels

#define OLED_SPI_D0_CLK 27
#define OLED_SPI_D1_DATA 28
#define OLED_SPI_DC 30
#define OLED_SPI_RES 29
#define OLED5_SPI_CS 31
#define OLED6_SPI_CS 32

const byte SYSEX_WRITE = 0x12;
const byte SYSEX_READ = 0x11;
const byte SYSEX_START = 0xF0;
const byte SYSEX_HEADER[6] = {0x41, 0x00, 0x00, 0x00, 0x00, 0x33}; //Katana header
const byte SYSEX_END = 0xF7;

const unsigned long P_EDIT =      0x7F000001;
const unsigned long PRESET =      0x00010000;
const unsigned long VOLUME =      0x60000652;
const unsigned long AMP_TYPE =    0x60000650;

// Turning effects on and off
const unsigned long EN_BOOSTER =  0x60000010;
const unsigned long EN_MOD =      0x60000100;
const unsigned long EN_FX =       0x60000300;
const unsigned long EN_DELAY =    0x60000500;
const unsigned long EN_REVERB =   0x60000540;

// Effect color
const unsigned long GRY_BOOSTER = 0x60000639;
const unsigned long GRY_MOD =     0x6000063A;
const unsigned long GRY_FX =      0x6000063B;
const unsigned long GRY_DELAY =   0x6000063C;
const unsigned long GRY_REVERB =  0x6000063D;

// Read only
const unsigned long R_BOOSTER =   0x6000065D;
const unsigned long R_MOD =       0x6000065E;
const unsigned long R_FX =        0x6000065F;
const unsigned long R_DELAY =     0x60000660;
const unsigned long R_REVERB =    0x60000661;

// Preset names
const unsigned long PANNEL_NAME = 0x10000000;
const unsigned long CH1_NAME =    0x10010000;
const unsigned long CH2_NAME =    0x10020000;
const unsigned long CH3_NAME =    0x10050000;
const unsigned long CH4_NAME =    0x10060000;

// Effect type (read only ?)
const unsigned long BOOSTER_TYPE = 0x60000011;
const unsigned long MOD_TYPE =     0x60000101;
const unsigned long FX_TYPE =      0x60000301;
const unsigned long DELAY_TYPE =   0x60000501;
const unsigned long REVERB_TYPE =  0x60000541;
