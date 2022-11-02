#include <Arduino.h>

#include "WiFi.h"

  #include <driver/adc.h>
  #include "config/config.h"
  #include "config/enums.h"
  #include "config/traduction.h"


  #include <NTPClient.h>


#include <AsyncElegantOTA.h>

// File System
#include <FS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

  #include "tasks/updateDisplay.h"
  #include "tasks/switchDisplay.h"
  #include "tasks/fetch-time-from-ntp.h"
  //#include "tasks/mqtt-aws.h"
  #include "tasks/wifi-connection.h"
  //#include "tasks/wifi-update-signalstrength.h"
  #include "tasks/measure-electricity.h"
  //#include "tasks/mqtt-home-assistant.h"
  #include "tasks/Dimmer.h"
  #include "tasks/gettemp.h"

  //#include "functions/otaFunctions.h"
  #include "functions/spiffsFunctions.h"
  #include "functions/Mqtt_http_Functions.h"
  #include "functions/webFunctions.h"

  #include "functions/froniusFunction.h"
  #include "functions/enphaseFunction.h"
  #include "functions/WifiFunctions.h"


#if DALLAS
// Dallas 18b20
#include <OneWire.h>
#include <DallasTemperature.h>
#include "tasks/dallas.h"
#include "functions/dallasfunction.h"
#endif

#if DIMMERLOCAL 
#include <RBDdimmer.h>   /// the corrected librairy  in RBDDimmer-master-corrected.rar , the original has a bug
#include "functions/dimmerFunction.h"
#endif



//***********************************
//************* Afficheur Oled
//***********************************
#ifdef  DEVKIT1
// Oled
#include "SSD1306Wire.h" /// Oled ( https://github.com/ThingPulse/esp8266-oled-ssd1306 ) 
const int I2C_DISPLAY_ADDRESS = 0x3c;
SSD1306Wire  display(0x3c, SDA, SCL); // pin 21 SDA - 22 SCL
#endif

#ifdef  TTGO
#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI display = TFT_eSPI();   // Invoke library
#endif


DisplayValues gDisplayValues;
//EnergyMonitor emon1;
Config config; 
Configwifi configwifi; 
Configmodule configmodule; 

/// declare logs 
Logs logging;
/// declare MQTT 
Mqtt configmqtt;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);


// Place to store local measurements before sending them off to AWS
unsigned short measurements[LOCAL_MEASUREMENTS];
unsigned char measureIndex = 0;

//***********************************
//************* Dallas
//***********************************
#if DALLAS
Dallas dallas; 
#endif



HA device_dimmer; 
HA device_routeur; 
HA device_grid;
HA device_inject;

/***************************
 *  Dimmer init
 **************************/
#if DIMMERLOCAL
dimmerLamp dimmer_hard(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
#endif

void setup()
{
  #if DEBUG == true
    Serial.begin(115200);
  #endif 
  logging.init="197}11}1";
  logging.init += "#################  Starting System  ###############\r\n";
  //démarrage file system
  Serial.println("start SPIFFS");
  logging.init += "Start Filesystem\r\n";
  SPIFFS.begin();


  ///define if AP mode or load configuration
  if (loadwifi(wifi_conf, configwifi)) {
    AP=false; 
  }

   loadmqtt(mqtt_conf ,configmqtt);
  // test if Fronius is present ( and load conf )
  configmodule.Fronius_present = loadfronius(fronius_conf, configmodule);

  // test if Enphase is present ( and load conf )
  configmodule.enphase_present = loadenphase(enphase_conf, configmodule);

  /// recherche d'une sonde dallas
  #if DALLAS
  Serial.println("start 18b20");
  sensors.begin();
  /// recherche d'une sonde dallas
  dallaspresent();
  #endif

  // Setup the ADC
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  //analogReadResolution(ADC_BITS);
  pinMode(ADC_INPUT, INPUT);


///  WIFI INIT
 


///// AP WIFI INIT 
if (AP) {
  APConnect(); 
    gDisplayValues.currentState = UP;
    gDisplayValues.IP = String(WiFi.softAPIP().toString());
    btStop();
}
else {
    #if WIFI_ACTIVE == true
      if ( strcmp(WIFI_PASSWORD,"xxx") == 0 ) { WiFi.begin(configwifi.SID, configwifi.passwd); }
      else { WiFi.begin(WIFI_NETWORK, WIFI_PASSWORD); }
      
      int timeoutwifi=0;
      while ( WiFi.status() != WL_CONNECTED ) {
        delay(500);
        Serial.print(".");
        timeoutwifi++; 
        if (timeoutwifi > 10 ) {break;}
      }

        //// timeout --> AP MODE 
        if ( timeoutwifi > 10 ) {
              WiFi.disconnect(); 
              AP=true; 
              serial_println("timeout, go to AP mode ");
              gDisplayValues.currentState = UP;
              gDisplayValues.IP = String(WiFi.softAPIP().toString());
              APConnect(); 
              btStop();
        }

      serial_println("WiFi connected");
      logging.init += "Wifi connected\r\n";
      serial_println("IP address: ");
      serial_println(WiFi.localIP());
      
      gDisplayValues.currentState = UP;
      gDisplayValues.IP = String(WiFi.localIP().toString());
      btStop();
    #endif
}

#if OLED_ON == true
    Serial.println(OLEDSTART);
    // Initialising OLED
    #ifdef  DEVKIT1
      display.init();
      display.flipScreenVertically();
    
      display.clear();
    #endif
    
    #ifdef TTGO

        pinMode(SWITCH,INPUT);

        display.init();
        //digitalWrite(TFT_BL, HIGH);
        display.setRotation(1);
        
        if (FLIP) {
        display.setRotation(3);
        }
        
        //display.begin();               // Initialise the display
        display.fillScreen(TFT_BLACK); // Black screen fill
        display.setCursor(0, 0, 2);
        display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextSize(1);
        display.println(BOOTING);
          if  (strcmp(WIFI_PASSWORD,"xxx") == 0) { 
            if  (strcmp(configwifi.SID,"xxx") == 0) {
            display.println(WIFINO); 
            }
          else { 
            display.println(WIFICONNECT + String(configwifi.SID));
          }
        } 
        else display.println(WIFICONNECT WIFI_NETWORK);
    #endif
#endif



#if DIMMERLOCAL 
Dimmer_setup();
#endif


   // vérification de la présence d'index.html
  if(!SPIFFS.exists("/index.html")){
    Serial.println(SPIFFSNO);  
    logging.init += "SPIFFS not uploaded!!\r\n";
  }

  if(!SPIFFS.exists(filename_conf)){
    Serial.println(CONFNO);  
    logging.init += "config file not exist, taking default value\r\n";
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

  




  // Initialize emon library
  //emon1.current(ADC_INPUT, 30);

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
  if (!AP){
    xTaskCreate(
      keepWiFiAlive,
      "keepWiFiAlive",  // Task name
      5000,            // Stack size (bytes)
      NULL,             // Parameter
      5,                // Task priority
      NULL          // Task handle
      
    );
    }
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

#if DALLAS
  // ----------------------------------------------------------------
  // Task: Read Dallas Temp
  // ----------------------------------------------------------------
  xTaskCreate(
    dallasread,
    "Dallas temp",  // Task name
    1000,                  // Stack size (bytes)
    NULL,                   // Parameter
    2,                      // Task priority
    NULL                    // Task handle
  );
#endif

#ifdef  TTGO
  // ----------------------------------------------------------------
  // Task: Update Dimmer power
  // ----------------------------------------------------------------
  xTaskCreate(
    switchDisplay,
    "Swith Oled",  // Task name
    1000,                  // Stack size (bytes)
    NULL,                   // Parameter
    2,                      // Task priority
    NULL                    // Task handle
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
  
  // ----------------------------------------------------------------
  // Task: Get Dimmer temp
  // ----------------------------------------------------------------

    xTaskCreate(
    GetDImmerTemp,
    "Update temp",  // Task name
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
  if (!AP) {
    #if NTP_TIME_SYNC_ENABLED == true
      xTaskCreate(
        fetchTimeFromNTP,
        "Update NTP time",
        5000,            // Stack size (bytes)
        NULL,             // Parameter
        2,                // Task priority
        NULL              // Task handle
      );
    }
  #endif

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


      #if WEBSSERVER == true
        AsyncElegantOTA.begin(&server);
        server.begin(); 
      #endif

if (!AP) {
    if (config.mqtt) {
      Mqtt_init();

    // HA autoconf

      device_dimmer.Set_name("dimmer");
      device_dimmer.Set_unit_of_meas("%");
      device_dimmer.Set_stat_cla("measurement");
      device_dimmer.Set_dev_cla("power");
      
      device_routeur.Set_name("power");
      device_routeur.Set_unit_of_meas("W");
      device_routeur.Set_stat_cla("measurement");
      device_routeur.Set_dev_cla("power");

      device_grid.Set_name("grid");
      device_grid.Set_unit_of_meas("W");
      device_grid.Set_stat_cla("measurement");
      device_grid.Set_dev_cla("power");

      device_inject.Set_name("inject");
      device_inject.Set_unit_of_meas("W");
      device_inject.Set_stat_cla("measurement");
      device_inject.Set_dev_cla("power");

      client.setBufferSize(1024);
      device_routeur.discovery();
      device_dimmer.discovery();
      device_grid.discovery();
      device_inject.discovery();

    }
}

  if ( config.autonome == true ) {
    gDisplayValues.dimmer = 0; 
    dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer ) ; 
  }

#endif

  #if OLED_ON == true
    #ifdef  DEVKIT1
      display.clear();
    #endif
  #endif



}

void loop()
{
//serial_println(F("loop")); 

  if (!AP) {
    #if WIFI_ACTIVE == true
        if (config.mqtt) {
        if (!client.connected()) {
          reconnect();
        }
        client.loop();
        }
    #endif
  }



  vTaskDelay(10000 / portTICK_PERIOD_MS);
}