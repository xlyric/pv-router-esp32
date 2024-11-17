#include <Arduino.h>
#include "driver/ledc.h"


#include "WiFiClientSecure.h"
#include <ESPmDNS.h>

  #include <driver/adc.h>
  #include "config/config.h"
  #include "config/enums.h"
  #include "config/traduction.h"
  #ifdef S3
  #include "pin_config.h"
  #endif

  #if  NTP
  #include <NTPClient.h>
  #include "tasks/fetch-time-from-ntp.h"
  #endif

#include <ElegantOTA.h>

// File System
#include <FS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

  #include "tasks/updateDisplay.h"
  #include "tasks/switchDisplay.h"
 
  
  #include "tasks/wifi-connection.h"
  #include "tasks/measure-electricity.h"
  
  #include "tasks/Dimmer.h"
 
  #include "tasks/gettemp.h"

  #include "tasks/Serial_task.h"
  #include "tasks/send-mqtt.h"
  #include "tasks/watchdog_memory.h"
  #include "tasks/mDNS.h"
  
  #include "functions/spiffsFunctions.h"
  #include "functions/Mqtt_http_Functions.h"
  #include "functions/webFunctions.h"

  #include "functions/froniusFunction.h"
  #include "functions/enphaseFunction.h"
  #include "functions/WifiFunctions.h"
  #include "functions/homeassistant.h"

  #include "functions/minuteur.h"

  #include "uptime.h"
  #include <driver/adc.h>

   

// Dallas 18b20
  #include <OneWire.h>
  #include <DallasTemperature.h>
  #include "tasks/dallas.h"
  #include "functions/dallasFunction.h"


/// déclaration dimmer
#include <RBDdimmer.h>   /// the corrected librairy  in RBDDimmer-master-corrected.rar , the original has a bug
#include "functions/unified_dimmer.h"
#include "functions/dimmerFunction.h"

int pwmChannel = 0; //Choisit le canal 0
int frequence = 1000; //Fréquence PWM de 1 KHz
int resolution = 10; // Résolution de 8 bits, 256 valeurs possibles



//***********************************
//************* Afficheur Oled
//***********************************
#ifdef  DEVKIT1
// Oled
#include "SSD1306Wire.h" /// Oled ( https://github.com/ThingPulse/esp8266-oled-ssd1306 ) 
constexpr const int I2C_DISPLAY_ADDRESS = 0x3c;
SSD1306Wire  display(0x3c, SDA, SCL); // pin 21 SDA - 22 SCL
#endif

#ifdef  TTGO
#include <TFT_eSPI.h>
#include <SPI.h>
TFT_eSPI display = TFT_eSPI();   // Invoke library
#endif


DisplayValues gDisplayValues;

Config config; 
Configwifi configwifi; 
Configmodule configmodule; 

///déclaration des programmateurs 
Programme programme; 
Programme programme_relay1;
Programme programme_relay2;

/// declare logs 
Logs logging;
/// declare MQTT 
Mqtt configmqtt;
/// surveillance mémoire
Memory task_mem; 


int retry_wifi = 0;
void connect_to_wifi();
void handler_before_reset();
void reboot_after_lost_wifi(int timeafterlost);
void IRAM_ATTR function_off_screen();
void IRAM_ATTR function_next_screen();


// Place to store local measurements before sending them off to AWS
unsigned short measurements[LOCAL_MEASUREMENTS]; // NOSONAR
unsigned char measureIndex = 0;

///gestion des tasks
TaskHandle_t serialTaskHandle = NULL;

//***********************************
//************* Dallas
//***********************************

Dallas dallas; 



#ifndef LIGHT_FIRMWARE
    HA device_dimmer; 
    HA device_routeur; 
    HA device_routed; // Ajout envoi MQTT puissance routée totale (local + distants)
    HA device_dimmer_power; // Ajout MQTT envoi puissance du dimmer local
    HA device_grid;
    HA device_inject;
    HA compteur_inject;
    HA compteur_grid;
    HA switch_1;
    HA temperature_HA;
    HA power_factor;
    HA power_apparent;
    HA switch_relay1;
    HA switch_relay2;

#endif

/***************************
 *  Dimmer init
 **************************/
/// Déclaration des dimmers
dimmerLamp dimmer1(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
#ifdef outputPin2
  dimmerLamp dimmer2(outputPin2, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
  dimmerLamp dimmer3(outputPin3, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
#endif
// déclaration de la gestion des dimmers
gestion_puissance unified_dimmer;

void setup()
{
  #if DEBUG == true
    Serial.begin(115200);
  #endif 
  #if CORE_DEBUG_LEVEL > ARDUHAL_LOG_LEVEL_NONE
    Serial.setDebugOutput(true);
  #endif
  Serial.println("\n================== " + String(VERSION) + " ==================");
  logging.Set_log_init("197}11}1");
  logging.Set_log_init("#################  Restart reason  ###############\r\n");
  esp_reset_reason_t reason = esp_reset_reason();
  logging.Set_log_init(String(reason).c_str());
  logging.Set_log_init("\r\n#################  Starting System  ###############\r\n");
  
  //démarrage file system
  Serial.println("start SPIFFS");
  test_fs_version();
  
  logging.Set_log_init("Start Filesystem\r\n",true);
  
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Initialization failed!");
    return;
  }

  /// Program & FS size
    // size of the compiled program
    uint32_t program_size = ESP.getSketchSize();

    // size of the file system
    uint32_t file_system_size = SPIFFS.totalBytes();

    // used size of the file system
    uint32_t file_system_used = SPIFFS.usedBytes();

    // free size in the flash memory
    uint32_t free_size = ESP.getFlashChipSize() - program_size - file_system_size + file_system_used;

    Serial.println("Program size: " + String(program_size) + " bytes");
    Serial.println("File system size: " + String(file_system_size) + " bytes");
    Serial.println ("File system used: " + String(file_system_used) + " bytes");
    Serial.println("Free space: " + String(free_size) + " bytes");

    pinMode(COOLER, OUTPUT);
    digitalWrite(COOLER, HIGH);

    ledcSetup(pwmChannel, frequence, resolution);
    //ledcAttachPin(outputPin, pwmChannel); // NOSONAR

    #ifdef ESP32D1MINI_FIRMWARE
        pinMode(outputPin2, OUTPUT);
        pinMode(outputPin3, OUTPUT);
        //ledcAttachPin(outputPin2, pwmChannel); // NOSONAR 
        //ledcAttachPin(outputPin3, pwmChannel);  // NOSONAR
    #endif
//**********************************    
/// test ACD 

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);
   

    //***********************************
    //************* Setup -  récupération du fichier de configuration
    //***********************************
  
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
  config.loadConfiguration();

  // récup des logs
  loadlogs();

  if (configwifi.recup_wifi()){
     
     logging.Set_log_init("Wifi config \r\n",true);
       AP=false; 
  } 
  else {
    Serial.println(F("mode AP please configure password before Wifi"));
  }

  configmodule.enphase_present=false; 
  configmodule.Fronius_present=false;

  configmqtt.loadmqtt();
  // vérification que le nom du serveur MQTT est différent de none
  if (strcmp(config.mqttserver,"none") == 0 ) {
    config.mqtt = false; 
  }
  else {
    config.mqtt = true; 
  }
  
  // test if Fronius is present ( and load conf )
  configmodule.Fronius_present = loadfronius(fronius_conf);

  // test if Enphase is present ( and load conf )
  loadenphase(enphase_conf);
 
  /// recherche d'une sonde dallas
  
  Serial.println("start 18b20");
  sensors.begin();
  /// recherche d'une sonde dallas
  dallas.detect = dallaspresent();
  

  // Setup the ADC
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);

  pinMode(ADC_INPUT, INPUT);

  // déclaration switch
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

///  WIFI INIT
 
  connect_to_wifi();

  // Initialize mDNS
  //config.say_my_name)
  if (!MDNS.begin(gDisplayValues.pvname)) {   
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  Serial.println(gDisplayValues.pvname);
  logging.Set_log_init("mDNS responder started\r\n",true);
  logging.Set_log_init(gDisplayValues.pvname + ".local\r\n",true);

#if OLED_ON == true
    Serial.println(OLEDSTART);
    // Initialising OLED
    #ifdef  DEVKIT1
      display.init();
      display.flipScreenVertically();
    
      display.clear();
    #endif
    
    #ifdef TTGO

        pinMode(SWITCH,INPUT_PULLUP);
        pinMode(BUTTON_LEFT,INPUT_PULLUP);

        display.init();
        display.setRotation(1);
        
        if (config.flip) {
        display.setRotation(3);
        }

        display.fillScreen(TFT_BLACK); // Black screen fill
        display.setCursor(0, 0, 2);
        display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextSize(1);
        display.println(BOOTING);
    #endif
#endif

/// init du NTP
ntpinit(); 

  Dimmer_setup();


   // vérification de la présence d'index.html
  if(!SPIFFS.exists("/index.html.gz")){
    Serial.println(SPIFFSNO);  
    
    logging.Set_log_init(SPIFFSNO,true);
  }

  if(!SPIFFS.exists(config.filename_conf)){
    Serial.println(CONFNO);  
    logging.Set_log_init(CONFNO,true);

  }

/// chargement des conf de minuteries
  Serial.println("Loading minuterie");
  programme.set_name("/dimmer");
  programme.loadProgramme();
  programme.saveProgramme();

  programme_relay1.set_name("/relay1");
  programme_relay1.loadProgramme();
  programme_relay1.saveProgramme();

  programme_relay2.set_name("/relay2");
  programme_relay2.loadProgramme();
  programme_relay2.saveProgramme();

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
  /*if (!AP){ // NOSONAR
    xTaskCreate( // NOSONAR
      keepWiFiAlive, // NOSONAR
      "keepWiFiAlive",  // Task name // NOSONAR
      8000,            // Stack size (bytes) // NOSONAR
      NULL,             // Parameter // NOSONAR
      5,                // Task priority // NOSONAR
      NULL          // Task handle // NOSONAR
      
    );  //pdMS_TO_TICKS(30000)// NOSONAR
    } */ // NOSONAR

    // task du watchdog de la mémoire
    xTaskCreate(
      watchdog_memory,
      "watchdog_memory",  // Task name
      6000,            // Stack size (bytes)
      NULL,             // Parameter
      5,                // Task priority
      NULL          // Task handle
    );  

        // task de recherche dimmer 
    xTaskCreate(
      mdns_discovery,
      "mdns_discovery",  // Task name
      6000,            // Stack size (bytes)
      NULL,             // Parameter
      5,                // Task priority
      NULL          // Task handle
    );  


     //// task pour remettre le wifi en route en cas de passage en mode AP
    xTaskCreate(
      keepWiFiAlive2,
      "keepWiFiAlive",  // Task name
      6000,            // Stack size (bytes)
      NULL,             // Parameter
      5,                // Task priority
      NULL          // Task handle
      
    );

    
  #endif

  // ----------------------------------------------------------------
  // TASK: envoie d'information série
  // ----------------------------------------------------------------
   //// la task a un timeout et le mode série ne répond plus après 2 minutes ce qui laisse le temps de faire les reglages wifi sur l'OTA
      xTaskCreate(
      serial_read_task,
      "Serial Read",      // Task name
      4000,            // Stack size (bytes)
      NULL,             // Parameter
      1,                // Task priority
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
    5000,            // Stack size (bytes)
    NULL,             // Parameter
    2,                // Task priority
    NULL,             // Task handle
    ARDUINO_RUNNING_CORE
  );  
  #endif



  if (dallas.detect) {
    // ----------------------------------------------------------------
    // Task: Read Dallas Temp
    // ----------------------------------------------------------------
    xTaskCreate(
      dallasread,
      "Dallas local temp",  // Task name
      5000,                  // Stack size (bytes)
      NULL,                   // Parameter
      2,                      // Task priority
      NULL       // Task handle
    ); 
  }


#ifdef  TTGO
  // ----------------------------------------------------------------
  // Task: Update Dimmer power
  // ----------------------------------------------------------------
  attachInterrupt(SWITCH, function_off_screen, FALLING);
  attachInterrupt(BUTTON_LEFT, function_next_screen, FALLING);

  xTaskCreate( 
    switchDisplay,
    "Swith Oled",  // Task name
    5000,                  // Stack size (bytes)
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
    8000,                  // Stack size (bytes)
    NULL,                   // Parameter
    7,                      // Task priority
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
  if (!dallas.detect) {
      xTaskCreate(
        GetDImmerTemp,
        "Update temp",  // Task name
        6000,                  // Stack size (bytes)
        NULL,                   // Parameter
        4,                      // Task priority
        NULL                    // Task handle
      );  
  }

// ----------------------------------------------------------------
  // Task: MQTT send
  // ----------------------------------------------------------------

    xTaskCreate(
    send_to_mqtt,
    "Update MQTT",  // Task name
    8000,                  // Stack size (bytes)
    NULL,                   // Parameter
    5,                      // Task priority
    NULL                    // Task handle
  );  


  #endif

#endif


#if WIFI_ACTIVE == true


      #if WEBSSERVER == true
        ElegantOTA.begin(&server);
        server.begin(); 
      #endif
  #ifndef LIGHT_FIRMWARE
      if (!AP) {
          if (config.mqtt) {
            Mqtt_init();

          // HA autoconf
          if (!client.connected() && (WiFi.status() == WL_CONNECTED )) { reconnect(); delay (1000);}
          if (configmqtt.HA) init_HA_sensor();
            
          }
      }
  #endif

    gDisplayValues.dimmer = 0; 
    dimmer_change( config.dimmer, config.IDXdimmer, gDisplayValues.dimmer,0 ) ; 


#endif

  #if OLED_ON == true
    #ifdef  DEVKIT1
      display.clear();
    #endif
  #endif





esp_register_shutdown_handler( handler_before_reset );

logging.power=true; logging.sct=true; logging.sinus=true; 

/// affichage de l'heure  GMT +1 dans la log
logging.Set_log_init("-- fin du demarrage  \r\n",true);

savelogs(" -- fin du précédent reboot -- ");

/// envoie de l'info de reboot
constexpr const int bufferSize = 150; // Taille du tampon pour stocker le message
char raison[bufferSize];     
snprintf(raison, bufferSize, "restart : %s", logging.loguptime()); 
#ifndef LIGHT_FIRMWARE 
  client.publish("memory/Routeur", raison, true);
#endif


}


/// @brief / Loop function
void loop()
{

  //// si perte du wifi après  6h, reboot
  if (AP) {
    reboot_after_lost_wifi(6);
  }

/// redémarrage sur demande
  if (config.restart) {

    Serial.print(PV_RESTART);
    savelogs("-- reboot demande par l'utilisateur -- ");
    ESP.restart();
  }

   ///  vérification de la taille du buffer log_init ( 600 caractères max ) il est créé à 650 caractères ( enums.h )
   /// pour éviter les buffer overflow et fuite mémoire. 
  logging.clean_log_init();
// affichage en mode serial de la taille de la chaine de caractère logging.log_init

// vérification de la connexion wifi 
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
/// connexion MQTT
#ifndef LIGHT_FIRMWARE
    if (!AP) {
      #if WIFI_ACTIVE == true
          if (config.mqtt) {
            if (!client.connected() && (WiFi.status() == WL_CONNECTED )) { reconnect(); delay (1000); }
          client.loop();
          
          }

      #endif
    }
#endif
//// surveillance des fuites mémoires 
#ifndef LIGHT_FIRMWARE
  client.publish(("memory/"+gDisplayValues.pvname).c_str(), String(esp_get_free_heap_size()).c_str()) ;
  client.publish(("memory/"+gDisplayValues.pvname + " min free").c_str(), String(esp_get_minimum_free_heap_size()).c_str()) ;
  
#endif

 


if (config.dimmerlocal) {
  ///////////////// gestion des activité minuteur 
  //// Dimmer 
    Serial.println(unified_dimmer.get_power());
    if (programme.run) { 
        //  minuteur en cours
        if (programme.stop_progr()) { 
            //dimmer1.setPower(0); // plus forcément utile --> unified dimmer
            unified_dimmer.dimmer_off();
            unified_dimmer.set_power(0);
            dallas.security=true;

          DEBUG_PRINTLN("programme.run");
          Serial.println("stop minuteur dimmer");
          //arret du ventilateur
          digitalWrite(COOLER, LOW);
          /// retrait de la securité dallas
          
          
          
          /// remonté MQTT
          #ifndef LIGHT_FIRMWARE
            Mqtt_send(String(config.IDX), String(unified_dimmer.get_power()),"pourcent"); // remonté MQTT de la commande réelle
            if (configmqtt.HA) {
              int instant_power = unified_dimmer.get_power();
              device_dimmer.send(String(instant_power * config.charge/100));
            } 
          #endif
        } 
    } 
    else { 
      // minuteur à l'arret
      if (programme.start_progr()){ 
        int sysvar_puissance; 
        if ( programme.puissance > config.localfuse ) {     sysvar_puissance=config.localfuse; }
        else { sysvar_puissance = programme.puissance; } 

        unified_dimmer.set_power(sysvar_puissance);
        delay (50);
        Serial.println("start minuteur ");
        //demarrage du ventilateur 
        digitalWrite(COOLER, HIGH);
        
        /// remonté MQTT
        #ifndef LIGHT_FIRMWARE
          Mqtt_send(String(config.IDX), String(unified_dimmer.get_power()),"pourcent"); // remonté MQTT de la commande réelle
          if (configmqtt.HA) {
            int instant_power = unified_dimmer.get_power();
            device_dimmer.send(String(instant_power * config.charge/100));
          } 
        #endif
      }
    }
}

if (programme_relay1.run) { 
      if (programme_relay1.stop_progr()) { 
        logging.Set_log_init("stop minuteur relay1\r\n",true);
        digitalWrite(RELAY1 , LOW);
//        device_relay1.send(String(0));

      }
 }
 else {
      if (programme_relay1.start_progr()){ 
        logging.Set_log_init("start minuteur relay1\r\n",true);
        digitalWrite(RELAY1 , HIGH);
      //  device_relay1.send(String(1));
      }
 }

 if (programme_relay2.run) { 
      if (programme_relay2.stop_progr()) { 
        logging.Set_log_init("stop minuteur relay2\r\n",true);
        digitalWrite(RELAY2 , LOW);
    //    device_relay2.send(String(0));
      }
 }
 else {
      if (programme_relay2.start_progr()){ 
        logging.Set_log_init("start minuteur relay2\r\n",true);
        digitalWrite(RELAY2 , HIGH);
      //device_relay2.send(String(1));
      }
 }

/// fonction de reboot hebdomadaire ( lundi 00:00 )

if (!AP) {
    time_reboot();
}



  task_mem.task_loop = uxTaskGetStackHighWaterMark(NULL);
  vTaskDelay(pdMS_TO_TICKS(10000));
}

/// @brief / end Loop function



void connect_to_wifi() {
  ///// AP WIFI INIT 
   

  if (AP || strcmp(configwifi.SID,"AP") == 0 ) {
      APConnect(); 
      gDisplayValues.currentState = DEVICE_STATE::UP;
      gDisplayValues.IP = String(WiFi.softAPIP().toString());
      btStop();
      return; 
  }
  else {
      #if WIFI_ACTIVE == true
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false);
      WiFi.begin(configwifi.SID, configwifi.passwd); 
      int timeoutwifi=0;
      
      logging.Set_log_init("Start Wifi Network ",true);
      logging.Set_log_init(configwifi.SID);
      logging.Set_log_init("\r\n");
      
      while ( WiFi.status() != WL_CONNECTED ) {
        delay(500);
        Serial.print(".");
        timeoutwifi++; 

        if (timeoutwifi > 40 ) {
              
              logging.Set_log_init("timeout, go to AP mode \r\n",true);
              
              logging.Set_log_init("Wifi State :",true);
              logging.Set_log_init("",true);
              
              switch (WiFi.status()) {
                  case 1:
                      logging.Set_log_init("SSID is not available");
                      break;
                  case 4:

                      logging.Set_log_init("The connection fails for all the attempts");
                      break;
                  case 5:
                      logging.Set_log_init("The connection is lost");
                      break;
                  case 6:
                      logging.Set_log_init("Disconnected from the network");
                      break;
                  default:
                      break;
          
              logging.Set_log_init("\r\n");
              } 
              break;
        }
    }

        //// timeout --> AP MODE 
        if ( timeoutwifi > 40 ) {
              WiFi.disconnect(); 
              serial_println("timeout, go to AP mode ");
              
              gDisplayValues.currentState = DEVICE_STATE::UP;
              APConnect(); 
        }


      serial_println("WiFi connected");
      logging.Set_log_init("Wifi connected\r\n",true);
      serial_println("IP address: ");
      serial_println(WiFi.localIP());
        serial_print("force du signal:");
        serial_print(WiFi.RSSI());
        serial_println("dBm");
      gDisplayValues.currentState = DEVICE_STATE::UP;
      gDisplayValues.IP = String(WiFi.localIP().toString());
      btStop();
      #endif
  }
}




void handler_before_reset() {
  #ifndef LIGHT_FIRMWARE
  const int bufferSize = 150; // Taille du tampon pour stocker le message
  char raison[bufferSize];

  snprintf(raison, bufferSize, "reboot handler: %s ",logging.loguptime()); 
  
  client.publish("memory/Routeur", raison, true);
  #endif
}

void reboot_after_lost_wifi(int timeafterlost) {
  uptime::calculateUptime();
  if ( uptime::getHours() > timeafterlost ) { 
    delay(15000);  
    config.restart = true; 
  }
}

void IRAM_ATTR function_off_screen() {
  gDisplayValues.screenbutton = true;
}

void IRAM_ATTR function_next_screen(){
  gDisplayValues.nextbutton = true;
  gDisplayValues.option++; 
  if (gDisplayValues.option > 2 ) { gDisplayValues.option = 1 ;}; 
}

