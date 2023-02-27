#ifndef CONFIG
#define CONFIG

/**
 * Modification matérielle du PV Routeur --> Très bon résultats, a tester dans le but de, pourquoi pas, lancer une V2?  
 */
#ifdef HARDWARE_MOD // si modification hardware du PV Router (bypass diode, ADC Middle sur GPIO 39, SCT centrée sur ADC MIDDLE )
// ADC MIDDLE = 3.3V /2

    float VrmsOLD = 225; // Valeur de référence, s'ajuste avec la tension mesurée en fonction du coef PHASECAL
    float PHASECAL = 0.2; // Coefficient permettant le lissage des valeurs de tension.  Vrms = VrmsOLD + PHASECAL * (Vrms - VrmsOLD); 
    int timeout = 22000; // 20ms , 1 * 50Hz + 2ms marge?
    int value0=1840; // Valeur de référence, s'ajuste avec la tension mesurée en fonction du coef PHASECAL


    float PVA;  //Puissance active en  VA
    double PW;   //Puissance en Watt
    float PowerFactor; // Facteur de puissance
    int startV; // Mesure ADC AC en début de cycle
    double Vrms; // Tension secteur RMS
    double Irms; // Intensité secteur RMS
#endif

/**
 * Language 
 */
#define LANG_FR true
#define LANG_EN false

#define CLEAN false
#define NTP false

/**
 * WiFi credentials
 */
#define WEBSSERVER true
#define WIFI_ACTIVE  true
//#define MQTT_CLIENT true --> option in the web menu

#define WIFI_NETWORK "xxx"
#define WIFI_PASSWORD "xxx"

/**
 * WiFi credentials
 */
#define MQTT_USER ""
#define MQTT_PASSWORD ""

/**
 * Switch Screnn button and time on
 * 
 */
#define SWITCH 35
//#define SWITCHTIMER 0   // 0 : always ON  / other : time in sec 
#define BUTTON_LEFT 0 // bouton droit ttgo
#define NB_PAGES 1 // nombre de pages d'affichages codées

/**
 * Time between 2 mesure of dimmer temp
 */
#define GETTEMPREFRESH 5
/**
 * Set this to false to disable Serial logging
 */
#define DEBUG true
//#define configMAX_PRIORITIES 1024


/**
 * The name of this device (as defined in the AWS IOT console).
 * Also used to set the hostname on the network
 */
#define DEVICE_NAME "Pv router ESP32"

/**
 * ADC input pin that is used to read out the CT sensor
 */

#ifdef  DEVKIT1
#define ADC_INPUT 34 // linky
#define ADC_PORTEUSE 35 // porteuse
#endif

#ifdef  TTGO
#define ADC_INPUT 32 // linky
#define ADC_PORTEUSE 33 // porteuse
#define ADC_MIDDLE 39 // 3.3V / 2
#endif


// #define ADC_MIDDLE 1893  /// en dessous laquelle on considère une valeur négative



#define ADC_BITS    12
#define ADC_COUNTS  (1<<ADC_BITS)
int sigma_read;
int half;




/**
 * The voltage of your home, used to calculate the wattage.
 * Try setting this as accurately as possible.
 */
// #define HOME_VOLTAGE 225.0

/**
 *  Dimmer 
 */

#define DIMMER true
#define DIMMERLOCAL true
#define DALLAS true
#define TRIGGER 10   /// 

///// PVROUTER false dans le cas d'un enphase en pilote full 
#define PVROUTER true

#if DIMMERLOCAL 
    #define outputPin  26 // PSM on board
    #define zerocross  27 // for boards with CHANGEBLE input pins // ZC on board
    #define cooler 12 // Pin for cooler. (switch on dimmer)

#endif

#if DALLAS
    #define ONE_WIRE_BUS  25
    #define TEMPERATURE_PRECISION 10
    #define TRIGGER 10   /// Trigger % for max temp protection. max temp configuration is in config.json 
 #endif
bool discovery_temp = false;



/**
 * Timeout for the WiFi connection. When this is reached,
 * the ESP goes into deep sleep for 30seconds to try and
 * recover.
 */
#define WIFI_TIMEOUT 30000 // 30 seconds

/**
 * How long should we wait after a failed WiFi connection
 * before trying to set one up again.
 */
#define WIFI_RECOVER_TIME_MS 30000 // 30 seconds

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
// #define emonTxV3 1


/**
 * Local measurements
 */
#define LOCAL_MEASUREMENTS 30
#define COMPENSATION 60 ///  % d'asservissement pour l'envoie de puissance au dimmer.

/**
 * Syncing time with an NTP server
 */
#define NTP_TIME_SYNC_ENABLED true
#define NTP_SERVER "europe.pool.ntp.org"
#define NTP_OFFSET_SECONDS 3600
#define NTP_UPDATE_INTERVAL_MS 3600000 /// synch de l'heure toute les heures


// Check which core Arduino is running on. This is done because updating the 
// display only works from the Arduino core.
#if CONFIG_FREERTOS_UNICORE
    #define ARDUINO_RUNNING_CORE 0
#else
    #define ARDUINO_RUNNING_CORE 1
#endif

#define VERSION "version 3.20230209"

#define EnvoyR "/api/v1/production"
#define EnvoyS "/production.json"

bool AP=true; 

/// Log web
#define TEMPOLOG 15 


#define RELAY1 13
#define RELAY2 15


#endif


///