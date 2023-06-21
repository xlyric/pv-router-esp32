#ifndef CONFIG
#define CONFIG

/**
 * Language 
 */
#define LANG_FR true
#define LANG_EN false

#define CLEAN false
#ifdef  DEVKIT1
#define NTP false
#else
#undef NTP
#endif
/**
 * WiFi credentials
 */
#define WEBSSERVER true
#define WIFI_ACTIVE  true
//#define MQTT_CLIENT true --> option in the web menu

//#define WIFI_NETWORK "xxx"
//#define WIFI_PASSWORD "xxx"

/**
 * WiFi credentials
 */
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_INTERVAL 60

/**
 * Switch Screnn button and time on
 * 
 */
#define SWITCH 0
//#define SWITCHTIMER 0   // 0 : always ON  / other : time in sec 
#define BUTTON_LEFT 35 // bouton droit ttgo
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
#endif


#define ADC_MIDDLE 1893  /// en dessous laquelle on considère une valeur négative



#define ADC_BITS    12
#define ADC_COUNTS  (1<<ADC_BITS)
int sigma_read;
float VrmsOLD = 225; // Valeur de référence, s'ajuste avec la tension mesurée en fonction du coef PHASECAL
float PHASECAL = 0.5;

// Valeurs théoriques pour PHASECAL.
// En modifiant le logiciel pour signaler le temps qu'il faut pour terminer la boucle de mesure interne 
// et le nombre d'échantillons enregistrés, le temps entre les échantillons a été mesuré à 377 μs.
// Cela équivaut à 6,79° (à 50 Hz, un cycle complet, soit 360°, prend 20 ms)
// Par conséquent, une valeur de 1 n'applique aucune correction, 
// Zéro et 2 appliquent environ 7° de correction dans des directions opposées.
// Une valeur de 1,28 corrigera l'erreur de 2° causée par le retard entre la tension d'échantillonnage et le courant.

float PVA;  //Power in VA
double PW;   //Power in Watt
float PowerFactor; // 


/**
 * The voltage of your home, used to calculate the wattage.
 * Try setting this as accurately as possible.
 */
// #define HOME_VOLTAGE 225.0

/**
 *  Dimmer 
 */

#define DIMMER true
//#define DIMMERLOCAL true
#define DALLAS false
#define TRIGGER 10   /// 

///// PVROUTER false dans le cas d'un enphase en pilote full 
#define PVROUTER true

#if 1//DIMMERLOCAL 
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
#define WIFI_TIMEOUT 30000 // 20 seconds

/**
 * How long should we wait after a failed WiFi connection
 * before trying to set one up again.
 */
#define WIFI_RECOVER_TIME_MS 30000 // 20 seconds

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
#define LOCAL_MEASUREMENTS 30
#define FACTEURPUISSANCE 10.50
#define COMPENSATION 100 ///  % d'asservissement pour l'envoie de puissance au dimmer.

/**
 * Syncing time with an NTP server
 */
#define NTP_TIME_SYNC_ENABLED true
#define NTP_SERVER "europe.pool.ntp.org"
#define NTP_OFFSET_SECONDS 3600
#define NTP_UPDATE_INTERVAL_MS 3600000 /// synch de l'heure toute les heures


//*************not implemented /tested **********
/**
 * Wether or not you want to enable Home Assistant integration
 */
//#define HA_ENABLED false
//#define HA_ADDRESS "*** YOUR HOME ASSISTANT IP ADDRESSS ***"
//#define HA_PORT 8883
//#define HA_USER ""
//#define HA_PASSWORD ""
//*************not implemented /tested **********
/**
 * The MQTT endpoint of the service we should connect to and receive messages
 * from.
 */
//#define AWS_ENABLED false
//#define AWS_IOT_ENDPOINT "**** YOUR AWS IOT ENDPOINT ****"
//#define AWS_IOT_TOPIC "**** YOUR AWS IOT RULE ARN ****"

//#define MQTT_CONNECT_DELAY 200
//#define MQTT_CONNECT_TIMEOUT 20000 // 20 seconds
//*************END not implemented /tested **********

// Check which core Arduino is running on. This is done because updating the 
// display only works from the Arduino core.
#if CONFIG_FREERTOS_UNICORE
    #define ARDUINO_RUNNING_CORE 0
#else
  //  #define ARDUINO_RUNNING_CORE 1
#endif

#define VERSION "Version 20230620"
    #ifdef LIGHT_FIRMWARE
        #define VERSION "Light " RELEASE
        #else
        #define VERSION RELEASE
    #endif

bool AP=true; 

/// Log web
#define TEMPOLOG 15 


#define RELAY1 13
#define RELAY2 15

/// taille max des logs stockée
#define LOG_MAX_STRING_LENGTH 254

//// JOTTA 
#define JOTTA  2
#define GRIDFREQ 90 ///PWM frequency

#ifdef S3
    #define ADC_INPUT 10
    #define INPUT 3
    #define RELAY1 43
    #define RELAY2 44
    #define SWITCH 14
    #define BUTTON_LEFT 0 // bouton droit ttgo
    #define outputPin  1 // PSM on board
    #define zerocross  2 // for boards with CHANGEBLE input pins // ZC on board
    #define cooler 18 // Pin for cooler. (switch on dimmer)
#endif


#endif


///