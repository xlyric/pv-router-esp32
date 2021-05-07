#include <Arduino.h>
#include "EmonLib.h"
#include "WiFi.h"
#include <driver/adc.h>
#include "config/config.h"
#include "config/enums.h"
#include <NTPClient.h>


// File System
#include <FS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
// Oled
#include "SSD1306Wire.h" /// Oled ( https://github.com/ThingPulse/esp8266-oled-ssd1306 ) 

#include "tasks/updateDisplay.h"
#include "tasks/fetch-time-from-ntp.h"
//#include "tasks/mqtt-aws.h"
#include "tasks/wifi-connection.h"
//#include "tasks/wifi-update-signalstrength.h"
#include "tasks/measure-electricity.h"
//#include "tasks/mqtt-home-assistant.h"
#include "tasks/Dimmer.h"

#include "functions/otaFunctions.h"
#include "functions/spiffsFunctions.h"
#include "functions/Mqtt_http_Functions.h"
#include "functions/webFunctions.h"

#if DIMMERLOCAL 
#include "functions/dimmerFunction.h"
#endif



//***********************************
//************* Afficheur Oled
//***********************************
const int I2C_DISPLAY_ADDRESS = 0x3c;
SSD1306Wire  display(0x3c, SDA, SCL); // pin 21 SDA - 22 SCL

DisplayValues gDisplayValues;
EnergyMonitor emon1;
Config config; 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);

// Place to store local measurements before sending them off to AWS
unsigned short measurements[LOCAL_MEASUREMENTS];
unsigned char measureIndex = 0;

void setup()
{
  #if DEBUG == true
    Serial.begin(115200);
  #endif 

  // Setup the ADC
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  analogReadResolution(ADC_BITS);
  pinMode(ADC_INPUT, INPUT);

#if WIFI_ACTIVE == true
  WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  serial_println("WiFi connected");
  serial_println("IP address: ");
  serial_println(WiFi.localIP());
  gDisplayValues.currentState = UP;
  gDisplayValues.IP = String(WiFi.localIP().toString());
  btStop();
#endif

#if DIMMERLOCAL 
Dimmer_setup();
#endif

  //démarrage file system
  Serial.println("start SPIFFS");
  SPIFFS.begin();

   // vérification de la présence d'index.html
  if(!SPIFFS.exists("/index.html")){
    Serial.println("Attention fichiers SPIFFS non chargé sur l'ESP, ça ne fonctionnera pas.");  
  }

  if(!SPIFFS.exists(filename_conf)){
    Serial.println("Attention problème avec fichier de conf");  
  }

     //***********************************
    //************* Setup -  récupération du fichier de configuration
    //***********************************
  
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadConfiguration(filename_conf, config);

  // Create configuration file
  //Serial.println(F("Saving configuration..."));
  //saveConfiguration(filename_conf, config);


  #if OLED_ON == true
    Serial.println("start Oled");
    // Initialising OLED
    display.init();
    display.flipScreenVertically();
    display.clear();
    //display.setFont(ArialMT_Plain_16);
    //display.drawString(30,30," Starting..."); 
    //display.display();
    
  #endif

  // Initialize emon library
  emon1.current(ADC_INPUT, 30);

  // Initialize Dimmer State 
  gDisplayValues.dimmer = 0;

#if WIFI_ACTIVE == true
  #if WEBSSERVER == true
  //***********************************
	//************* Setup -  demarrage du webserver et affichage de l'oled
	//***********************************
   Serial.println("start Web server");
   call_pages();
  #endif

  // ----------------------------------------------------------------
  // TASK: Connect to WiFi & keep the connection alive.
  // ----------------------------------------------------------------
  /*
  xTaskCreatePinnedToCore(
    keepWiFiAlive,
    "keepWiFiAlive",  // Task name
    5000,            // Stack size (bytes)
    NULL,             // Parameter
    5,                // Task priority
    NULL    ,      // Task handle
    ARDUINO_RUNNING_CORE
  );*/
  #endif

  // ----------------------------------------------------------------
  // TASK: Connect to AWS & keep the connection alive.
  // ----------------------------------------------------------------
  #if AWS_ENABLED == true
    xTaskCreate(
      keepAWSConnectionAlive,
      "MQTT-AWS",      // Task name
      5000,            // Stack size (bytes)
      NULL,             // Parameter
      5,                // Task priority
      NULL              // Task handle
    );
  #endif

  // ----------------------------------------------------------------
  // TASK: Update the display every second
  //       This is pinned to the same core as Arduino
  //       because it would otherwise corrupt the OLED
  // ----------------------------------------------------------------
  #if OLED_ON == true 
  xTaskCreatePinnedToCore(
    updateDisplay,
    "UpdateDisplay",  // Task name
    10000,            // Stack size (bytes)
    NULL,             // Parameter
    4,                // Task priority
    NULL,             // Task handle
    ARDUINO_RUNNING_CORE
  );


  #endif


  // ----------------------------------------------------------------
  // Task: measure electricity consumption ;)
  // ----------------------------------------------------------------
  xTaskCreate(
    measureElectricity,
    "Measure electricity",  // Task name
    5000,                  // Stack size (bytes)
    NULL,                   // Parameter
    25,                      // Task priority
    NULL                    // Task handle
  
  );

#if WIFI_ACTIVE == true
  #if DIMMER == true
  // ----------------------------------------------------------------
  // Task: Update Dimmer power
  // ----------------------------------------------------------------
  xTaskCreate(
    updateDimmer,
    "Update Dimmer",  // Task name
    5000,                  // Stack size (bytes)
    NULL,                   // Parameter
    4,                      // Task priority
    NULL                    // Task handle
  );
  #endif
#endif


  // ----------------------------------------------------------------
  // TASK: update time from NTP server.
  // ----------------------------------------------------------------
#if WIFI_ACTIVE == true
  #if NTP_TIME_SYNC_ENABLED == true
    xTaskCreate(
      fetchTimeFromNTP,
      "Update NTP time",
      5000,            // Stack size (bytes)
      NULL,             // Parameter
      2,                // Task priority
      NULL              // Task handle
    );
#endif

  // ----------------------------------------------------------------
  // TASK: update WiFi signal strength
  // ----------------------------------------------------------------
 /* xTaskCreate(
    updateWiFiSignalStrength,
    "Update WiFi strength",
    1000,             // Stack size (bytes)
    NULL,             // Parameter
    2,                // Task priority
    NULL              // Task handle
  );
*/
  #if HA_ENABLED == true
    xTaskCreate(
      HADiscovery,
      "MQTT-HA Discovery",  // Task name
      5000,                // Stack size (bytes)
      NULL,                 // Parameter
      5,                    // Task priority
      NULL                  // Task handle
    );

    xTaskCreate(
      keepHAConnectionAlive,
      "MQTT-HA Connect",
      5000,
      NULL,
      4,
      NULL
    );
  #endif
#endif

#if WIFI_ACTIVE == true
  #if OTA == true
    while ( gDisplayValues.currentState != UP )
    {
      delay (500); 
    }
    OTA_init();
    timeClient.begin();
  #endif

  #if WEBSSERVER == true
    server.begin(); 
  #endif

  if ( config.mqtt == true ) {
    Mqtt_init();
  }

  if ( config.autonome == true ) {
    gDisplayValues.dimmer = 0; 
    dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer ) ; 
  }

#endif

  #if OLED_ON == true
    display.clear();
  #endif

}

void loop()
{
//serial_println(F("loop")); 

#if WIFI_ACTIVE == true
    #if OTA == true
    ArduinoOTA.handle();
    #endif

    #if MQTT_CLIENT == true
    if (!client.connected()) {
    reconnect();
    }
    #endif
#endif

  vTaskDelay(10000 / portTICK_PERIOD_MS);
}