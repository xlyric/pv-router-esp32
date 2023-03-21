#include <Arduino.h>

#include "WiFi.h"

  #include <driver/adc.h>
  #include "config/config.h"
  #include "config/enums.h"
  #include "config/traduction.h"

  #if  NTP
  #include <NTPClient.h>
  #include "tasks/fetch-time-from-ntp.h"
  #endif

#include <AsyncElegantOTA.h>

// File System
#include <FS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

  #include "tasks/updateDisplay.h"
  #include "tasks/switchDisplay.h"
 
  //#include "tasks/mqtt-aws.h"
  #include "tasks/wifi-connection.h"
  //#include "tasks/wifi-update-signalstrength.h"
  #include "tasks/measure-electricity.h"
  //#include "tasks/mqtt-home-assistant.h"
  #include "tasks/Dimmer.h"
  #include "tasks/gettemp.h"

  #include "tasks/Serial_task.h"

  //#include "functions/otaFunctions.h"
  #include "functions/spiffsFunctions.h"
  #include "functions/Mqtt_http_Functions.h"
  #include "functions/webFunctions.h"

  #include "functions/froniusFunction.h"
  #include "functions/enphaseFunction.h"
  #include "functions/WifiFunctions.h"
  #include "functions/homeassistant.h"

  #include "uptime.h"

#if DALLAS
// Dallas 18b20
#include <OneWire.h>
#include <DallasTemperature.h>
#include "tasks/dallas.h"
#include "functions/dallasFunction.h"
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


int retry_wifi = 0;
void connect_to_wifi();
#if  NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);
#endif

// Place to store local measurements before sending them off to AWS
unsigned short measurements[LOCAL_MEASUREMENTS];
unsigned char measureIndex = 0;

//***********************************
//************* Dallas
//***********************************
#if DALLAS
Dallas dallas; 
#endif

String loguptime();
#ifndef LIGHT_FIRMWARE
    HA device_dimmer; 
    HA device_routeur; 
    HA device_grid;
    HA device_inject;
    HA compteur_inject;
    HA compteur_grid;
    HA switch_1;
    HA temperature_HA;
    HA power_factor;
    HA power_vrms;
    HA power_irms;
    HA power_apparent;
#endif

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
  logging.init += loguptime();
  logging.init += "Start Filesystem\r\n";
  SPIFFS.begin();

    //***********************************
    //************* Setup -  récupération du fichier de configuration
    //***********************************
  
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  loadConfiguration(filename_conf, config);

  ///define if AP mode or load configuration
  /*if (loadwifi(wifi_conf, configwifi)) {
    AP=false; 
  }*/
  if (configwifi.recup_wifi()){
     logging.init += loguptime();
     logging.init += "Wifi config \r\n"; 
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
  dallas.detect = dallaspresent();
  #endif

  // Setup the ADC
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

  //analogReadResolution(ADC_BITS);
  pinMode(ADC_INPUT, INPUT);

  // déclaration switch
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);




///  WIFI INIT
 
  connect_to_wifi();

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
        
        if (config.flip) {
        display.setRotation(3);
        }
        
        //display.begin();               // Initialise the display
        display.fillScreen(TFT_BLACK); // Black screen fill
        display.setCursor(0, 0, 2);
        display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextSize(1);
        display.println(BOOTING);
    #endif
#endif



#if DIMMERLOCAL 
Dimmer_setup();
#endif


   // vérification de la présence d'index.html
  if(!SPIFFS.exists("/index.html.gz")){
    Serial.println(SPIFFSNO);  
    logging.init += loguptime();
    logging.init += SPIFFSNO ;
  }

  if(!SPIFFS.exists(filename_conf)){
    Serial.println(CONFNO);  
    logging.init += loguptime();
    logging.init += CONFNO;
  }

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
    xTaskCreate(
      serial_read_task,
      "Serial Read",      // Task name
      2000,            // Stack size (bytes)
      NULL,             // Parameter
      0,                // Task priority
      NULL              // Task handle
    );


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
    1,                // Task priority
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
    #if NTP  
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
    #endif
  }
    
#endif

#if WIFI_ACTIVE == true


      #if WEBSSERVER == true
        AsyncElegantOTA.begin(&server);
        server.begin(); 
      #endif
#ifndef LIGHT_FIRMWARE
    if (!AP) {
        if (config.mqtt) {
          Mqtt_init();

        // HA autoconf
        if (configmqtt.HA) init_HA_sensor();
          
        }
    }
#endif
  //if ( config.autonome == true ) {
    gDisplayValues.dimmer = 0; 
    dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer ) ; 
  //}

#endif

  #if OLED_ON == true
    #ifdef  DEVKIT1
      display.clear();
    #endif
  #endif


logging.power=true; logging.sct=true; logging.sinus=true; 
}

void loop()
{

/// redémarrage sur demande
  if (config.restart) {
    delay(5000);
    Serial.print(PV_RESTART);
    ESP.restart();
  }

/// vérification du buffer log 
  if (logging.start.length() > LOG_MAX_STRING_LENGTH ) { 
    logging.start = "";
  }

  

//serial_println(F("loop")); 

  if ( WiFi.status() != WL_CONNECTED ) {
      connect_to_wifi();
      }
    
  if (AP) {
    int number_client = WiFi.softAPgetStationNum(); // Nombre de stations connectées à ESP8266 soft-AP
    if (number_client == 0 ) {
      if (retry_wifi == 10 ) {
        retry_wifi = 0;
        connect_to_wifi();
      }
      if (retry_wifi < 10 ) {
        retry_wifi ++;
      }

    }
  }
#ifndef LIGHT_FIRMWARE
    if (!AP) {
      #if WIFI_ACTIVE == true
          if (config.mqtt) {
            if (!client.connected()) { reconnect(); }
          client.loop();
          }
          // TODO : désactivation MQTT en décochant la case, sans redémarrer
          // else {
          //     if (client.connected()) { client.disconnect(); }
          // }  

      #endif
    }
#endif
  vTaskDelay(10000 / portTICK_PERIOD_MS);
}

void connect_to_wifi() {
  ///// AP WIFI INIT 
  if (AP || strcmp(configwifi.SID,"AP") == 0 ) {
      AP=true; 
      APConnect(); 
      gDisplayValues.currentState = UP;
      gDisplayValues.IP = String(WiFi.softAPIP().toString());
      btStop();
      return; 
  }
  else {
      #if WIFI_ACTIVE == true
      WiFi.mode(WIFI_STA);
      //Esp_wifi_set_ps (WIFI_PS_NONE);
      WiFi.setSleep(false);
      WiFi.begin(configwifi.SID, configwifi.passwd); 
      int timeoutwifi=0;
      logging.init += loguptime();
      logging.init += "Start Wifi Network " + String(configwifi.SID) +  "\r\n";
      while ( WiFi.status() != WL_CONNECTED ) {
        delay(500);
        Serial.print(".");
        timeoutwifi++; 

        if (timeoutwifi > 20 ) {
              logging.init += loguptime();
              logging.init += "timeout, go to AP mode \r\n" ;  
              logging.init += loguptime();
              logging.init += "Wifi State :";
              logging.init += loguptime();
              
              switch (WiFi.status()) {
                  case 1:

                      logging.init += "SSID is not available" ; 
                      break;
                  case 4:

                      logging.init += "The connection fails for all the attempts"  ;
                      break;
                  case 5:
                      logging.init +="The connection is lost" ; 
                      break;
                  case 6:
                      logging.init +="Disconnected from the network" ; 
                      break;
                  default:
                      logging.init +="I have no idea ?! " ; 
                      break;
          
              
              logging.init += "\r\n";
              break;}
      }


        //// timeout --> AP MODE 
        if ( timeoutwifi > 20 ) {
              WiFi.disconnect(); 
              AP=true; 
              serial_println("timeout, go to AP mode ");
              
              gDisplayValues.currentState = UP;
             // gDisplayValues.IP = String(WiFi.softAPIP().toString());
              APConnect(); 
        }
    }

      serial_println("WiFi connected");
      logging.init += loguptime();
      logging.init += "Wifi connected\r\n";
      serial_println("IP address: ");
      serial_println(WiFi.localIP());
        serial_print("force du signal:");
        serial_print(WiFi.RSSI());
        serial_print("dBm");
      gDisplayValues.currentState = UP;
      gDisplayValues.IP = String(WiFi.localIP().toString());
      btStop();
      #endif
  }
}

String loguptime() {
  String uptime_stamp;
  uptime::calculateUptime();
  uptime_stamp = String(uptime::getDays())+":"+String(uptime::getHours())+":"+String(uptime::getMinutes())+":"+String(uptime::getSeconds())+ "\t";
  return uptime_stamp;
}