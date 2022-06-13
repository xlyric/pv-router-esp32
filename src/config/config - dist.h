#ifndef CONFIG
#define CONFIG

/**
 * WiFi credentials
 */
#define WEBSSERVER true
#define WIFI_ACTIVE  true
#define MQTT_CLIENT true

#define WIFI_NETWORK "xxx"
#define WIFI_PASSWORD "xxx"

/**
 * MQTT credentials
 */
#define MQTT_USER ""
#define MQTT_PASSWORD ""

/**
 * Switch Screnn button and time on
 */
#define SWITCH 35
//#define SWITCHTIMER 0   // 0 : always ON  / other : time in sec 

/**
 * Time between 2 mesure of dimmer temp
 */
#define GETTEMPREFRESH 30


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

/**
 * The voltage of your home, used to calculate the wattage.
 * Try setting this as accurately as possible.
 */
#define HOME_VOLTAGE 225.0

/**
 *  Dimmer 
 */

#define DIMMER true /// Use distant Dimmer 
#define DIMMERLOCAL false /// Use local Dimmer 
#define DALLAS false  //// Use local Dallas ( GPIO 25 )

///// PVROUTER false if you use Enphase Envoy S switch ( default : true )
#define PVROUTER true 

#if DIMMERLOCAL 
#define outputPin  26 
#define zerocross  27 // for boards with CHANGEBLE input pins
#endif

#if DALLAS
#define ONE_WIRE_BUS  25  /// Need to conbect pin 37 to 25 ( pin 37 not working with Dallas )
#define TEMPERATURE_PRECISION 10  
#define TRIGGER 10   /// Trigger % for max temp protection. max temp configuration is in config.json 
#endif



/**
 * Timeout for the WiFi connection. When this is reached,
 * the ESP goes into deep sleep for 30seconds to try and
 * recover.
 */
#define WIFI_TIMEOUT 20000 // 20 seconds

/**
 * How long should we wait after a failed WiFi connection
 * before trying to set one up again.
 */
#define WIFI_RECOVER_TIME_MS 20000 // 20 seconds

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
// #define emonTxV3 1   /// not used 


/**
 * Local measurements
 */
#define LOCAL_MEASUREMENTS 30
#define FACTEURPUISSANCE 10.50

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
#define ARDUINO_RUNNING_CORE 1
#endif

#define VERSION "version 3.5"

#define EnvoyR "/api/v1/production"
#define EnvoyS "/production.json"

bool AP=true; 

#endif