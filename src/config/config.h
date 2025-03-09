#ifndef CONFIG
#define CONFIG

/**
 * Language
 */
// #define LANG_FR true
// #define LANG_EN false
//#define DEBUGLEVEL1 false

#define CLEAN false
#define NTP false

/**
 * WiFi credentials
 */
#define WEBSSERVER true
#define WIFI_ACTIVE true

/**
 * MQTT credentials
 */
#define MQTT_USER ""
#define MQTT_PASSWORD ""
constexpr size_t MQTT_INTERVAL = 60;

/**
 * Switch Screnn button and time on
 *
 */
#define SWITCH 35
#define BUTTON_LEFT 0 // bouton droit ttgo
#define NB_PAGES 1    // nombre de pages d'affichages codées

/**
 * Time between 2 mesure of dimmer temp
 */
constexpr size_t GETTEMPREFRESH = 5;

// #define configMAX_PRIORITIES 1024

/**
 * The name of this device (as defined in the AWS IOT console).
 * Also used to set the hostname on the network
 */
#define DEVICE_NAME "Pv router ESP32"

/**
 * ADC input pin that is used to read out the CT sensor
 */

#ifdef ESP32D1MINI_FIRMWARE
  #define ADC_INPUT 34    // linky
  #define ADC_PORTEUSE 35 // porteuse
#endif

#ifdef TTGO
  #define ADC_INPUT 32    // linky
  #define ADC_PORTEUSE 33 // porteuse
#endif

#define ADC_MIDDLE 1893 /// en dessous laquelle on considère une valeur négative

int dimmer_getState_interval = 0; // On requête la puissance du dimmer régulièrement, mais pas trop souvent

#define ADC_BITS 12
#define ADC_COUNTS (1 << ADC_BITS)
float VrmsOLD = 225; // Valeur de référence, s'ajuste avec la tension mesurée en fonction du coef PHASECAL
float PHASECAL = 0.5;

// Valeurs théoriques pour PHASECAL.
// En modifiant le logiciel pour signaler le temps qu'il faut pour terminer la boucle de mesure interne
// et le nombre d'échantillons enregistrés, le temps entre les échantillons a été mesuré à 377 μs.
// Cela équivaut à 6,79° (à 50 Hz, un cycle complet, soit 360°, prend 20 ms)
// Par conséquent, une valeur de 1 n'applique aucune correction,
// Zéro et 2 appliquent environ 7° de correction dans des directions opposées.
// Une valeur de 1,28 corrigera l'erreur de 2° causée par le retard entre la tension d'échantillonnage et le courant.
float PVA;         // Power in VA
double PW;         // Power in Watt
float PowerFactor; //

#define AUTO_OFF 5 // temps de coupure automatique en minutes

/**
 *  Dimmer
 */
#define DIMMER true
// #define TRIGGER 10   ///

///// PVROUTER false dans le cas d'un enphase en pilote full
#define PVROUTER true

#define outputPin 26 // PSM on board
#define zerocross 27 // for boards with CHANGEBLE input pins // ZC on board
#define COOLER 12    // Pin for COOLER. (switch on dimmer)
#define outputPin2 37 // pin 37 TTGO
#define outputPin3 38 // pin 38 TTGO

#define ONE_WIRE_BUS 25
#define TEMPERATURE_PRECISION 10
// #define TRIGGER 10   /// Trigger % for max temp protection. max temp configuration is in config.json
bool discovery_temp = false;

/**
 * Timeout for the WiFi connection. When this is reached,
 * the ESP goes into deep sleep for 30seconds to try and
 * recover.
 */
constexpr size_t WIFI_TIMEOUT = 30000; // 20 seconds

/**
 * How long should we wait after a failed WiFi connection
 * before trying to set one up again.
 */
constexpr size_t WIFI_RECOVER_TIME_MS = 30000; // 20 seconds

/**
 * Dimensions of the OLED display attached to the ESP
 */
#define OLED_ON true
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/// flip screen  set true for 3D print box
#define FLIP true

/**
 * Force Emonlib to assume a 3.3V supply to the CT sensor
 */
#define emonTxV3 1

/**
 * Local measurements
 */
constexpr size_t LOCAL_MEASUREMENTS = 30;
#define FACTEURPUISSANCE 10.50
constexpr size_t COMPENSATION = 100; ///  % d'asservissement pour l'envoie de puissance au dimmer.

/**
 * Syncing time with an NTP server
 */
#define NTP_TIME_SYNC_ENABLED true
#define NTP_SERVER "europe.pool.ntp.org"
constexpr size_t NTP_OFFSET_SECONDS = 3600;
constexpr size_t NTP_UPDATE_INTERVAL_MS = 3600000; /// synch de l'heure toute les heures

// Check which core Arduino is running on. This is done because updating the
// display only works from the Arduino core.
#if CONFIG_FREERTOS_UNICORE
  #define ARDUINO_RUNNING_CORE 0
#else
  //  #define ARDUINO_RUNNING_CORE 1
#endif

#define RELEASE "Version 20250309"
constexpr const int FS_RELEASE = 20250302;

#ifdef LIGHT_FIRMWARE
  #define VERSION "Light " RELEASE
#else
  #define VERSION RELEASE
#endif

#define EnvoyJ "/auth/check_jwt"
#define EnvoyR "/api/v1/production"
#define EnvoyS "/production.json"

bool AP = true;

/// Log web
#define TEMPOLOG 15

#define RELAY1 13
#define RELAY2 15
#define TIME_BOOST 3600 // temps de boost

/// taille max des logs stockée
constexpr size_t LOG_MAX_STRING_LENGTH = 1500;

#ifdef S3
  #define ADC_INPUT 10
  #define INPUT 3
  #define RELAY1 43
  #define RELAY2 44
  #define SWITCH 14
  #define BUTTON_LEFT 0 // bouton droit ttgo
  #define outputPin 1   // PSM on board
  #define zerocross 2   // for boards with CHANGEBLE input pins // ZC on board
  #define COOLER 18     // Pin for COOLER. (switch on dimmer)
#endif

/**
 * Set this to false to disable Serial logging
 */
#define DEBUG true
// #define DEBUGLEVEL1 false
#ifdef DEBUGLEVEL1
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINTLN(x)
#endif

/// Configuration pour ESP32D1MINI sur carte dimmer + récupération Shelly
#ifdef ESP32D1MINI_FIRMWARE
  #undef outputPin
  #define outputPin 18 // PSM on board
  #undef zerocross
  #define zerocross 19 // for boards with CHANGEBLE input pins // ZC on board
  #undef COOLER
  #define COOLER 5      // Pin for COOLER. (switch on dimmer)
  #undef outputPin2
  #define outputPin2 22 // use JOTTA/SSR2 (SSR2) output for 2nd Robotdyn/Random SSR
  #undef outputPin3
  #define outputPin3 21 // (SSR3) use RELAY2/SSR3 output for 3rd Robotdyn/Random SSR ( for old boards )

  #undef ONE_WIRE_BUS
  #define ONE_WIRE_BUS 23
  #define TEMPERATURE_PRECISION 10
  // #define TRIGGER 5   /// Trigger % for max temp protection. max temp configuration is in config.json

  // #undef OLED_ON
  // #define OLED_ON TRUE
  #undef RELAY1
  #define RELAY1 17
  #undef RELAY2
  #define RELAY2 26
#endif

#endif
