#ifndef ENUMS
#define ENUMS

#include <Arduino.h>

// The state in which the device can be. This mainly affects what
// is drawn on the display.
enum DEVICE_STATE {
  CONNECTING_WIFI,
  CONNECTING_AWS,
  FETCHING_TIME,
  UP,
};

// Place to store all the variables that need to be displayed.
// All other functions should update these!
struct DisplayValues {
  double watt;
  double amps;
  int8_t wifi_strength;
  DEVICE_STATE currentState;
  String IP;
  String time;
  bool injection; 
  int dimmer;
  int security; 
  int change;
  bool task;
  bool porteuse; 
  bool screenstate; 
  String temperature;
  int Fronius_conso; 
  int Fronius_prod; 
  int Fronius_totalconso;
  float celsius;
  int page=1; 
};

struct Config {
  char hostname[16];
  int port;
  char apiKey[64];
  bool UseDomoticz;
  bool UseJeedom;
  int IDX;
  char otapassword[64];
  int delta;
  int deltaneg;
  int cosphi;
  int readtime;
  int cycle;
  bool sending;
  bool autonome;
  char dimmer[16];
  bool dimmerlocal;
  float facteur;
  int num_fuse;
  bool mqtt;
  char mqttserver[16];
  int mqttport; 
  int IDXdimmer;
  int IDXprod; //IDX pour le device qui comptabilise la production (enphase)
  int tmax;
  int resistance;
  bool polarity; 
  char Publish[100];
  int  ScreenTime;
};

struct Configwifi {
  char SID[32];
  char passwd[64];
};

struct Configmodule {
  char hostname[16];
  char port[5];
  bool enphase_present; 
  bool Fronius_present;
  char envoy[5];
  bool pilote; 
};

#if DEBUG == true
  #define serial_print(x)  Serial.print (x)
  #define serial_println(x)  Serial.println (x)
#else
  #define serial_print(x)
  #define serial_println(x)
#endif

struct Dallas{
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius = 0.00 ;
  byte security = 0;
};

#endif