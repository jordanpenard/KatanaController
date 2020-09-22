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

#define BP_1 34
#define BP_2 35
#define BP_3 36
#define BP_4 37
#define BP_5 38
#define BP_6 39

const byte SYSEX_WRITE = 0x12;
const byte SYSEX_READ = 0x11;
const byte SYSEX_START = 0xF0;
const byte SYSEX_HEADER[6] = {0x41, 0x00, 0x00, 0x00, 0x00, 0x33}; //Katana header
const byte SYSEX_END = 0xF7;

const unsigned long P_EDIT =      0x7F000001;
const unsigned long PRESET =      0x00010000;
const unsigned long VOLUME =      0x60000652;

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

// Preset names
const unsigned long PANNEL_NAME = 0x10000000;
const unsigned long CH1_NAME =    0x10010000;
const unsigned long CH2_NAME =    0x10020000;
const unsigned long CH3_NAME =    0x10050000;
const unsigned long CH4_NAME =    0x10060000;
