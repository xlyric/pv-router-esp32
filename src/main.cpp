//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#include "driver/ledc.h"
#include "WiFiClientSecure.h"
#include <ESPmDNS.h>
#include <driver/adc.h>
#include <ElegantOTA.h>
#include <FS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
#include "uptime.h"
#include <driver/adc.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <RBDdimmer.h>   /// the corrected librairy  in RBDDimmer-master-corrected.rar , the original has a bug


//***********************************
//************* PROGRAMME PVROUTEUR
//***********************************
#include "config/config.h"
#include "config/enums.h"
#include "config/traduction.h"
#include "langues/lang.h"
#include "functions/spiffsFunctions.h"
#include "tasks/Dimmer.h"
#include "tasks/Serial_task.h"
#include "tasks/dallas.h"
#include "tasks/gettemp.h"
#include "tasks/mDNS.h"
#include "tasks/measure-electricity.h"
#ifndef LIGHT_FIRMWARE
  #include "tasks/send-mqtt.h"
#endif  
#include "tasks/switchDisplay.h"
#include "tasks/updateDisplay.h"
#include "tasks/watchdog_memory.h"
#include "tasks/wifi-connection.h"
#include "functions/ESP32.h"
#ifndef LIGHT_FIRMWARE
#include "functions/Mqtt_http_Functions.h"
#endif
#include "functions/WifiFunctions.h"
#include "functions/dallasFunction.h"
#include "functions/dimmerFunction.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"
#include "functions/homeassistant.h"
#include "functions/minuteur.h"
#include "functions/unified_dimmer.h"
#include "functions/webFunctions.h"


//***********************************
//************* S3 NON DECLARE
//************* A SUPPRIMER ?
//***********************************
#ifdef S3
  #include "pin_config.h"
#endif

//***********************************
//************* NTP valorisé dans 
//************* src/config/config.h
//***********************************
#if NTP
  #include <NTPClient.h>
  #include "tasks/fetch-time-from-ntp.h"
#endif

//***********************************
//************* WEBSOCKET_CLIENT
//************ Déprécié : a supprimer
//************ Usage futur : à conserver
//***********************************
#ifdef WEBSOCKET_CLIENT
  #include "functions/websocket.h"
#endif

//***********************************
//************* Afficheur Oled
//************* ESP32D1MINI_FIRMWARE
//************* option build
//************* plus nécessaire
//***********************************
#ifdef  ESP32D1MINI_FIRMWARE
 /// deja intégré dans display.h
#endif

//***********************************
//************* Afficheur TTGO
//************* TTGO
//************* option build
//***********************************
#ifdef  TTGO
  #include <TFT_eSPI.h>
  #include <SPI.h>
  TFT_eSPI display = TFT_eSPI();   
#endif

//***********************************
//************* VARIABLES GLOBALES
//***********************************
int pwmChannel = 0; //Choisit le canal 0
int frequence = 1000; //Fréquence PWM de 1 KHz
int resolution = 10; // Résolution de 8 bits, 256 valeurs possibles

//***********************************
//************* VARIABLES GLOBALES
//************* GESTION TACHES
//***********************************
TaskHandle_t myTaskmdnsdiscovery; 
TaskHandle_t myTaskwatchdogmemory;
TaskHandle_t myTaskkeepwifialive2;
TaskHandle_t myTaskserialreadtask;
TaskHandle_t myTaskupdatedisplay;
TaskHandle_t myTaskdallasread;
TaskHandle_t myTaskswitcholed;
TaskHandle_t myTaskmeasureelectricity;
TaskHandle_t myTaskupdatedimmer;
TaskHandle_t myTasksendtomqtt;
TaskHandle_t serialTaskHandle = NULL;
SemaphoreHandle_t mutex;  

//***********************************
//************* VARIABLES GLOBALES
//************* GESTION DISPLAY
//***********************************
DisplayValues gDisplayValues;

//***********************************
//************* VARIABLES GLOBALES
//************* CONFIGURATION
//***********************************
Config config; 
Configwifi configwifi; 
Configmodule configmodule; 

//***********************************
//************* VARIABLES GLOBALES
//************* PROGRAMMATEURS
//***********************************
Programme programme; 
Programme programme_relay1;
Programme programme_relay2;
Programme programme_marche_forcee;
Programme programme_batterie;

//***********************************
//************* VARIABLES GLOBALES
//************* LOGS
//***********************************
Logs logging;

//***********************************
//************* VARIABLES GLOBALES
//************* MQTT
//***********************************
Mqtt configmqtt;

//***********************************
//************* VARIABLES GLOBALES
//************* SURVEILLANCE MEMOIRE
//***********************************
Memory task_mem; 

//***********************************
//************* VARIABLES GLOBALES
//************* SONDE DALLAS
//***********************************
Dallas dallas; 

//***********************************
//************* VARIABLES GLOBALES
//************* MQTT HA
//***********************************
unsigned short measurements[LOCAL_MEASUREMENTS]; // NOSONAR
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
    HA device_dimmer_boost;
    HA device_dimmer_alarm_temp;
#endif

//***********************************
//************* VARIABLES GLOBALES
//************* DEPRECIEES A SUPPR
//*************   measureIndex dans drawFunction.h
//***********************************
unsigned char measureIndex = 0;

//***********************************
//************* VARIABLES FONCTIONS
//***********************************
void connect_to_wifi();
void handler_before_reset();
void reboot_after_lost_wifi(int timeafterlost);
void IRAM_ATTR function_off_screen();
void IRAM_ATTR function_next_screen();
bool boost();
void myTask(void *pvParameters);

//***********************************
//************* VARIABLES GLOBALES
//************* DIMMERS
//***********************************
gestion_puissance unified_dimmer;
dimmerLamp dimmer1(outputPin, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
#ifdef outputPin2
  dimmerLamp dimmer2(outputPin2, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
#endif
#ifdef outputPin3
  dimmerLamp dimmer3(outputPin3, zerocross); //initialase port for dimmer for ESP8266, ESP32, Arduino due boards
#endif

//***********************************
//************* SETUP
//***********************************
void setup()
{
  #if DEBUG == true
    Serial.begin(115200);
  #endif 
  char temp_buffer[128];
  // Redirection des messages de DEBUG ves le port série
  // CORE_DEBUG_LEVEL est définie dans les options de BUILD
  #if CORE_DEBUG_LEVEL > ARDUHAL_LOG_LEVEL_NONE
    Serial.setDebugOutput(true);
  #endif

  // Initialisation de la LOG
  Serial.println("\n================== " + String(VERSION) + " ==================");
  logging.Set_log_init("197}11}1");
  snprintf(temp_buffer, sizeof(temp_buffer), "################# %s ################# \n", Reason_for_reset);
  logging.Set_log_init(temp_buffer);
  esp_reset_reason_t reason = esp_reset_reason();
  char buf_int[12];
  itoa(reason, buf_int, 10);
  logging.Set_log_init(buf_int);
  snprintf(temp_buffer, sizeof(temp_buffer), "\n################# %s ############### \n", Starting_System);
  logging.Set_log_init(temp_buffer);
  
  //démarrage file system
  Serial.println("start SPIFFS");
  logging.Set_log_init(Start_filesystem,true);
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Initialization failed!");
    return;
  }
  // début comptage reboot pour retrait du blocage AP
  if (switch_ap_mode()) { reset_wifi(); }
  // Vérification de la version du File System
  test_fs_version();

  // Program & FS size
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
  
  // ACTIVATION DU COOLER
  pinMode(COOLER, OUTPUT);
  digitalWrite(COOLER, HIGH);

  // Configuration du canal PWM (Pulse With Modulation)
  ledcSetup(pwmChannel, frequence, resolution);
  //ledcAttachPin(outputPin, pwmChannel); // NOSONAR

  // Gestion JOTTA/SSR2 (outputPin2)
  // Gestion RELAY3/SSR3 (outputPin3)
  #ifdef outputPin2
    pinMode(outputPin2, OUTPUT);
  #endif
  #ifdef outputPin3
    pinMode(outputPin3, OUTPUT);
    //ledcAttachPin(outputPin2, pwmChannel); // NOSONAR 
    //ledcAttachPin(outputPin3, pwmChannel);  // NOSONAR
  #endif

  // Configuration de la résolution des conversions analogique-numérique (ACD)
  // Résolution ACD 12bits 
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_DB_12);
  adc1_config_channel_atten(ADC1_CHANNEL_3, ADC_ATTEN_DB_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
   

  //***********************************
  //************* Setup -  récupération du fichier de configuration
  //***********************************
  // Should load default config if run for the first time
  Serial.println(F("Loading configuration..."));
    config.loadConfiguration();

  //***********************************
  //************* Setup -  récupération des LOGS
  //***********************************
  loadlogs();

  // Initialisation de l'écran OLED
  // ESP32D1MINI_FIRMWARE : option de BUILD
  #ifdef ESP32D1MINI_FIRMWARE
    oled.init();
    oled.wait_for_wifi(0);
  #endif

  //***********************************
  //************* Setup -  récupération de la config Wifi ou AP
  //***********************************
  if (configwifi.recup_wifi()) {
     logging.Set_log_init(Loading_wifi_configuration,true);
       AP=false; 
  } 
  else {
    Serial.println(F("mode AP please configure password before Wifi"));
  }

  //***********************************
  //************* Setup -  setup MQTT
  //***********************************
  configmqtt.loadmqtt();
  // vérification que le nom du serveur MQTT est différent de none
  if (strcmp(config.mqttserver,"none") == 0 ) {
    config.mqtt = false; 
  }
  else {
    config.mqtt = true; 
  }
  
  //***********************************
  //************* Setup -  setup FRONIUS
  //***********************************  
  configmodule.Fronius_present=false;
  configmodule.Fronius_present = loadfronius(fronius_conf);

  //***********************************
  //************* Setup -  setup ENPHASE
  //***********************************  
  configmodule.enphase_present=false;   
  loadenphase(enphase_conf);
 
  //***********************************
  //************* Setup -  sonde DALLAS 18B20 locale
  //***********************************  
  Serial.println("start 18b20");
  sensors.begin();
  // Recherche de sonde
  dallas.detect = dallaspresent();
  // detection d'incohérence entre une dallas précédement détecté et maintenant non détecté
  if (dallas.detect == false && config.dallas_present == true) {
    dallas.detect = true;  // ce qui remontera l'incohérence et la tentative de reconnexion en plus de la protection
    logging.Set_log_init("Dallas précédente non retrouvé au reboot, mise en sécurité\r\n");
    dallas.lost = true;
  }
  // inversement, si présente mais pas noté dans la conf alors on en registre
  if (dallas.detect == true && config.dallas_present == false) {
    config.dallas_present = true; 
    // on sauvegarde la conf
    config.saveConfiguration();
  }
  Serial.println("debugdallas");
  Serial.println(config.dallas_present);
  Serial.println(dallas.detect);
  Serial.println(dallas.lost);

  // Mise à jour de la résolution des conversions analogique-numérique (ACD)
  // Résolution ACD 12bits 
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12);
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_12);
  adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_12);
  #ifdef TTGO
    pinMode(ADC_INPUT, INPUT);
  #endif
  // déclaration switch
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, HIGH); //correction bug de démarrage en GPIO 0
  digitalWrite(RELAY2, LOW);

  //***********************************
  //************* Setup - connexion Wifi
  //***********************************    
  // Affichage OLED 1
  #ifdef  ESP32D1MINI_FIRMWARE
    oled.wait_for_wifi(1);
  #endif
  // reset du timer de reboot pour connexion Wifi 
  // Connexion WiFi
  connect_to_wifi();
  // Affichage OLED 1
  #ifdef  ESP32D1MINI_FIRMWARE
    oled.wait_for_wifi(2);
  #endif
  
  //***********************************
  //************* Setup - mDNS
  //***********************************    
  mdns_hello(gDisplayValues.pvname.c_str());
  Serial.println(mDNS_Responder_Started);
  Serial.println(gDisplayValues.pvname);
  logging.Set_log_init(mDNS_Responder_Started,true);
  snprintf(temp_buffer, sizeof(temp_buffer), "mDNS Responder started for %s.local\n", gDisplayValues.pvname.c_str());
  logging.Set_log_init(temp_buffer);  

  //***********************************
  //************* Setup - infos ESP32
  //***********************************    
  ESP32Info espInfo = getESP32Info();
  Serial.println("=== ESP32 Information ===");
  Serial.print("Chip Model: "); Serial.println(espInfo.chipModel);
  Serial.print("Chip Revision: "); Serial.println(espInfo.chipRevision);
  Serial.print("Number of CPU Cores: "); Serial.println(espInfo.chipCores);
  Serial.print("Board Name: "); Serial.println(espInfo.boardName);
  Serial.print("Chip ID: "); Serial.println(espInfo.chipID, HEX);

  //***********************************
  //************* Setup - init Affichage OLED
  //***********************************    
  #if OLED_ON == true
    Serial.println(OLEDSTART);
    // Initialising OLED   
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
    #endif // TTGO

  #endif // OLED_ON

  //***********************************
  //************* Setup - NPT
  //***********************************
  // utilisation si il pas en 192.168.33.0/24
  bool Shelly_local = false;
  IPAddress localIP = WiFi.localIP();
  if (localIP[0] == 192 && localIP[1] == 168 && localIP[2] == 33) {
    Serial.println("L'adresse IP est dans le sous-réseau 192.168.33.x");
    ntpinit_Shelly();
    gDisplayValues.Shelly_local = true; // pour le mode AP
  } else {
    Serial.println("L'adresse IP n'est PAS dans le sous-réseau 192.168.33.x");
    ntpinit();
  }

  //***********************************
  //************* Setup - Dimmer
  //***********************************  
  Dimmer_setup();

  //***********************************
  //************* Setup - vérif FS
  //***********************************   
  // vérification de la présence d'index.html
  if(!SPIFFS.exists("/index.html.gz")){
    Serial.println(SPIFFSNO);  
    logging.Set_log_init(SPIFFSNO,true);
  }

  //***********************************
  //************* Setup - Chargement config
  //***********************************   
  // vérification de la présence du fichier de configuration
  if(!SPIFFS.exists(config.filename_conf)){
    Serial.println(CONFNO);  
    logging.Set_log_init(CONFNO,true);
  }
  /// chargement des conf de minuteries
  Serial.println("Loading minuterie");
  programme.set_name("/dimmer");
  programme.loadProgramme();
  programme_marche_forcee.set_name("/marche_forcee");
  programme_relay1.set_name("/relay1");
  programme_relay1.loadProgramme();
  programme_relay2.set_name("/relay2");
  programme_relay2.loadProgramme();
  programme_batterie.set_name("/batterie");
  programme_batterie.loadProgramme();
  
  // Initialize Dimmer State 
  gDisplayValues.dimmer = 0;

  //***********************************
  //************* Setup - init Semaphore
  //***********************************   
  mutex = xSemaphoreCreateMutex();  // Création du mutex

  #if WIFI_ACTIVE == true
    #if WEBSSERVER == true
      //***********************************
      //************* Setup -  demarrage du webserver et affichage de l'oled
      //***********************************
      Serial.println("start Web server");
      call_pages();
    #endif // WEBSERVER


    //***********************************
    //************* Setup -  init xTask
    //***********************************
    // ----------------------------------------------------------------
    // TASK: Connect to WiFi & keep the connection alive (dépréciée -> keepWifiAlive2)
    // ----------------------------------------------------------------
    /*
    if (!AP) { // NOSONAR
      xTaskCreate( // NOSONAR
        keepWiFiAlive, // NOSONAR
        "keepWiFiAlive",  // Task name // NOSONAR
        8000,            // Stack size (bytes) // NOSONAR
        NULL,             // Parameter // NOSONAR
        5,                // Task priority // NOSONAR
        NULL          // Task handle // NOSONAR      
      );  //pdMS_TO_TICKS(30000)// NOSONAR
    } //AP // NOSONAR
    */
    // ----------------------------------------------------------------
    // TASK: watchdog Mémory
    // ----------------------------------------------------------------
    /*

    xTaskCreatePinnedToCore(
      watchdog_memory,
      "watchdog_memory",  // Task name
      4000,            // Stack size (bytes)
      NULL,             // Parameter
      3,                // Task priority

      &myTaskwatchdogmemory,          // Task handle
      0
    );  
    */
    // ----------------------------------------------------------------
    // TASK: recherche de dimmer
    // ----------------------------------------------------------------
    xTaskCreatePinnedToCore (
        mdns_discovery,
        "mdns_discovery",  // Task name
        3000,            // Stack size (bytes)
        NULL,             // Parameter
        1,                // Task priority
        &myTaskmdnsdiscovery,          // Task handle
        ARDUINO_RUNNING_CORE
    );  // le service est arreté si un dimmer est trouvé ou déjà configuré --> necessite reboot pour rechercher de nouveau
    // ----------------------------------------------------------------
    // TASK: remettre le wifi en route en cas de passage en mode AP
    // ----------------------------------------------------------------

    xTaskCreate(
      keepWiFiAlive2,
      "keepWiFiAlive",  // Task name
      3000,            // Stack size (bytes)
      NULL,             // Parameter
      2,                // Task priority

      &myTaskkeepwifialive2          // Task handle      
    );  
  #endif // WIFI_ACTIVE
  // ----------------------------------------------------------------
  // TASK: envoie d'information série
  // ----------------------------------------------------------------
  //// la task a un timeout et le mode série ne répond plus après 2 minutes ce qui laisse le temps de faire les reglages wifi sur l'OTA
  xTaskCreate(
    serial_read_task,
    "Serial Read",      // Task name
    3000,            // Stack size (bytes)
    NULL,             // Parameter
    2,                // Task priority
    &myTaskserialreadtask              // Task handle
  );  /// le service est arreté après 2 minutes

  // ----------------------------------------------------------------
  // TASK: Update the display every second
  // ----------------------------------------------------------------
  //       This is pinned to the same core as Arduino
  //       because it would otherwise corrupt the OLED
  // ----------------------------------------------------------------
  #if OLED_ON == true
    #ifdef ESP32D1MINI_FIRMWARE
      Serial.println("init oled 0.7'' ");
      init_ui();
      if (config.flip) {
        display.flipScreenVertically();
      }

    #endif // ESP32D1MINI_FIRMWARE
    xTaskCreate(
      updateDisplay,
      "UpdateDisplay",  // Task name
      3500,            // Stack size (bytes)
      NULL,             // Parameter
      2,                // Task priority
      &myTaskupdatedisplay             // Task handle      
    );  
  #endif // OLED_ON


  // ----------------------------------------------------------------
  // Task: Read Dallas Temp
  // ----------------------------------------------------------------
  if (dallas.detect) {
    xTaskCreate(
      dallasread,
      "Dallas local temp",  // Task name
      4000,                  // Stack size (bytes)
      NULL,                   // Parameter
      2,                      // Task priority
      &myTaskdallasread       // Task handle
    ); 
  }

  // ----------------------------------------------------------------
  // Task: Update Dimmer power
  // ----------------------------------------------------------------
  #ifdef TTGO
    #ifndef ESP32D1MINI_FIRMWARE
      attachInterrupt(SWITCH, function_off_screen, FALLING);
      attachInterrupt(BUTTON_LEFT, function_next_screen, FALLING);
      xTaskCreate( 
        switchDisplay,
        "Swith Oled",  // Task name
        3000,                  // Stack size (bytes)
        NULL,                   // Parameter
        2,                      // Task priority
        &myTaskswitcholed                    // Task handle
      ); 
    #endif // ESP32D1MINI_FIRMWARE
  #endif // TTGO

  // ----------------------------------------------------------------
  // Task: measure electricity consumption ;)
  // ----------------------------------------------------------------
  xTaskCreate(
    measureElectricity,
    "Measure electricity",  // Task name
    5000,                  // Stack size (bytes)
    NULL,                   // Parameter
    4,                      // Task priority
    &myTaskmeasureelectricity                    // Task handle
  );  

  // ----------------------------------------------------------------
  // Task: Update Dimmer power
  // ----------------------------------------------------------------

  #if WIFI_ACTIVE == true
    #if DIMMER == true
      xTaskCreatePinnedToCore(
        updateDimmer,
        "Update Dimmer",  // Task name
        4000,                  // Stack size (bytes)
        NULL,                   // Parameter
        4,                      // Task priority
        &myTaskupdatedimmer,                    // Task handle
        ARDUINO_RUNNING_CORE
      );  
      // ----------------------------------------------------------------
      // Task: Get Dimmer temp
      // ----------------------------------------------------------------
      if (!dallas.detect) {
        xTaskCreatePinnedToCore(
          GetDImmerTemp,
          "Update temp",  // Task name
          4000,                  // Stack size (bytes)
          NULL,                   // Parameter
          2,                      // Task priority
          NULL,                    // Task handle
          ARDUINO_RUNNING_CORE
        );  
      }
      // ----------------------------------------------------------------
      // Task: MQTT send
      // ----------------------------------------------------------------
      #ifndef LIGHT_FIRMWARE
        xTaskCreatePinnedToCore(
          send_to_mqtt,
          "Update MQTT",  // Task name
          4500,                  // Stack size (bytes)
          NULL,                   // Parameter
          2,                      // Task priority
          &myTasksendtomqtt,                    // Task handle
          ARDUINO_RUNNING_CORE
        );  
      #endif
    #endif // DIMMER = true
  #endif // WIFI_ACTIVE=true

        if (gDisplayValues.Shelly_local) {
          // on arrete les services non necessaires
          // pour le mode AP Shelly
          // arret de la task mdns_discovery
          if (myTaskmdnsdiscovery != NULL) {
            vTaskDelete(myTaskmdnsdiscovery);
            myTaskmdnsdiscovery = NULL;
          }
          // arret de la task send_to_mqtt
          if (myTasksendtomqtt != NULL) {
            vTaskDelete(myTasksendtomqtt);
            myTasksendtomqtt = NULL;
          }
        }


  //***********************************
  //************* Setup - init ElegantOTA
  //***********************************   
  #if WIFI_ACTIVE == true
    #if WEBSSERVER == true
      ElegantOTA.begin(&server);
      server.begin(); 
    #endif

    //***********************************
    //************* Setup - MQTT init
    //***********************************   
    #ifndef LIGHT_FIRMWARE
      if (!AP) {
          if (config.mqtt) {
            Mqtt_init();            
            if (!client.connected() && (WiFi.status() == WL_CONNECTED )) { 
              reconnect(); 
              delay (1000);
            }
            // HA autoconf
            if (configmqtt.HA) 
              init_HA_sensor(); 
              //Serial.println(F("Home Assistant enabled"));           
          }
      }
    #endif // not LIGHT_FIRMWARE

    gDisplayValues.dimmer = 0; 
    dimmer_change(config.dimmer, config.IDXdimmer, gDisplayValues.dimmer,0); 
  #endif // WIFI_ACTIVE == true

  //***********************************
  //************* Setup - handler before reboot
  //***********************************  
  esp_register_shutdown_handler( handler_before_reset );
  logging.power=true; 
  logging.sct=true; 
  logging.sinus=true; 

  /// affichage de l'heure  GMT +1 dans la log
  logging.Set_log_init(End_Start,true);
  savelogs(" -- fin du précédent reboot -- ");

  /// envoie de l'info de reboot
  constexpr const int bufferSize = 150; // Taille du tampon pour stocker le message
  char raison[bufferSize];     
  snprintf(raison, bufferSize, "restart : %s", logging.loguptime()); 
  #ifndef LIGHT_FIRMWARE 
    client.publish("memory/Routeur", raison, true);
  #endif

  //***********************************
  //************* Setup - initialisaiton du socket web
  //***********************************    
  #ifdef WEBSOCKET_CLIENT
    setupWebSocket(); // initialisation du socket web
  #endif

  programme_marche_forcee.temperature = config.tmax;

  #ifdef DEMO
  Serial.println(F("Demo mode enabled"));
  logging.Set_log_init("Demo mode enabled",true);
  #endif
  // reset timer reboot reset wifi
  createTempFile("/ap_mode.txt", "0");
}

//***********************************
//************* LOOP
//***********************************



void loop() {

static int delta_backup = 0;
static int deltaneg_backup = 0;

/*
  #ifdef DEBUGLEVEL1

  //***********************************
  //************* Loop - affichage de la mémoire dispo / xTasks
  //***********************************    
  
    // Function to check stack for a specific task and print the result
    auto printTaskStack = [](TaskHandle_t taskHandle, const char* taskName) {
      if (taskHandle != NULL) {
        Serial.print("Stack min restante de ");
        Serial.print(taskName);
        Serial.print(" : ");
        Serial.println(uxTaskGetStackHighWaterMark(taskHandle));
      }
    };  
    // Check stack space for all tasks
    //printTaskStack(myTaskmdnsdiscovery, "myTaskmdnsdiscovery"); // la tache s'arrete 
    printTaskStack(myTasksendtomqtt, "myTasksendtomqtt");
    printTaskStack(myTaskupdatedimmer, "myTaskupdatedimmer");
    printTaskStack(myTaskmeasureelectricity, "myTaskmeasureelectricity");
    printTaskStack(myTaskswitcholed, "myTaskswitcholed");
    printTaskStack(myTaskdallasread, "myTaskdallasread");
    printTaskStack(myTaskupdatedisplay, "myTaskupdatedisplay");
    //printTaskStack(myTaskserialreadtask, "myTaskserialreadtask"); // la tache s'arrete
    printTaskStack(myTaskkeepwifialive2, "myTaskkeepwifialive2");
    printTaskStack(myTaskwatchdogmemory, "myTaskwatchdogmemory");
  #endif
*/
    //**** Loop - vérification de la connexion au serveur MQTT **** 
    #ifndef LIGHT_FIRMWARE
    if (config.mqtt) {
      if (!client.connected()) {
        reconnect(); 
      }
      client.loop(); // Keep the MQTT connection alive
    }
    #endif // not LIGHT_FIRMWARE

    int retry_wifi = 0;

  //***********************************
  //************* Loop - web socket loop
  //*********************************** 

  #ifdef WEBSOCKET_CLIENT
    //handleWebSocket(); // Keep the WebSocket connection alive
    webSocket.loop();
  #endif

  //***********************************
  //************* Loop - mode AP - gestion perte wifi apres 6H
  //*********************************** 
  if (AP) {
    reboot_after_lost_wifi(6);
  }

  //***********************************
  //************* Loop - mode AP - redemmarrage sur demande
  //*********************************** 
  if (config.restart) {
    Serial.print(PV_RESTART);
    savelogs("-- reboot demande par l'utilisateur -- ");
    ESP.restart();
  }

  //***********************************
  //************* Loop -  vérification de la taille du buffer log_init ( 600 caractères max ) il est créé à 650 caractères ( enums.h )
  //*********************************** 
  logging.clean_log_init();

  //***********************************
  //************* Loop -  vérification de la connexion wifi 
  //*********************************** 
  if ( WiFi.status() != WL_CONNECTED ) {
    connect_to_wifi();
  }

  //***********************************
  //************* Loop -  gestion du nombre de stations connectées en AP
  //*********************************** 
  if (AP) {
    int number_client = WiFi.softAPgetStationNum(); // Nombre de stations connectées à ESP
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

  // gestion de la température minimum.
  
 
      if (gDisplayValues.temperature < config.tmin ) {
        if (!config.preheat && config.dimmerlocal && dallas.detect && dallas.lost == false ) {
          // Si la température est inférieure à la température minimale, mise en route du dimmer au limiteur localfuse
          unified_dimmer.set_power(config.localfuse, "Préchauffage");
          Serial.println("Température minimale atteinte, préchauffage activé");
          logging.Set_log_init("Préchauffage activé \n", true);
          config.preheat = true;
        } 
      }
      else if ( config.preheat ) //&& gDisplayValues.temperature > config.tmin (condition inutile car gDisplayValues.temperature > tmin pour arriver ici)
        { config.preheat = false;
          unified_dimmer.set_power(0, "Fin préchauffage");
          Serial.println("Fin préchauffage, dimmer arrêté");
          logging.Set_log_init("Fin préchauffage, dimmer arrêté \n", true);
        }

   //***********************************
  //************* Loop -  gestion des activités minuteurs
  //*********************************** 
  if (config.dimmerlocal) {
    ///////////////// gestion des activité minuteur 
    //// Dimmer 
    Serial.println(unified_dimmer.get_power());
    if (programme.run || programme_marche_forcee.run) { 
      //  minuteur en cours
      if (programme.stop_progr() || programme_marche_forcee.stop_progr() ) { 
        unified_dimmer.dimmer_off("minuteur"); 
        unified_dimmer.set_power(0, "minuteur"); 
        config.preheat = false;
        if (dallas.detect) {
          dallas.security=true;
        }
        DEBUG_PRINTLN("programme.run");
        Serial.println("stop minuteur dimmer");
        //arret du ventilateur
        digitalWrite(COOLER, LOW);
        /// retrait de la securité dallas          
        // on remet les valeurs de temps programme_marche_force à 00:00
        strcpy(programme_marche_forcee.heure_demarrage, "00:00"); // NOSONAR
        strcpy(programme_marche_forcee.heure_arret, "00:00");  // NOSONAR        

      } // if (programme.stop_progr() || programme_marche_forcee.stop_progr() )
    } // if (programme.run || programme_marche_forcee.run)  

    else { 
      // minuteur à l'arret
      if (programme.start_progr() ||  programme_marche_forcee.start_progr() ){ 
        int sysvar_puissance; 
        if ( programme.puissance > config.localfuse ) {     
          sysvar_puissance=config.localfuse; 
        }
        else { 
          sysvar_puissance = programme.puissance; 
        } 
        unified_dimmer.set_power(sysvar_puissance);
        delay (50);
        Serial.println("start minuteur ");
        //demarrage du ventilateur 
        digitalWrite(COOLER, HIGH);      

      } // if (programme.start_progr() ||  programme_marche_forcee.start_progr() )
    } // else
  } // if (config.dimmerlocal)


  /// gestion de l'offset batterie
   
    //if (programme_batterie.start_progr() && !batterie_active && gDisplayValues.temperature > programme_batterie.temperature ) {
    if ( gDisplayValues.temperature > programme_batterie.temperature && !config.batterie_active && strcmp(programme_batterie.heure_demarrage, programme_batterie.heure_arret) != 0) {
      // Sauvegarde des valeurs d'origine
      delta_backup = config.delta;
      deltaneg_backup = config.deltaneg;
      // Application de l'offset puissance_batterie (utilise temperature comme offset selon ta consigne)
      // programme_batterie.puissance doit toujours être positif 
      if (programme_batterie.puissance < 0) {
        programme_batterie.puissance = -programme_batterie.puissance; // on ne peut pas avoir d'offset négatif
      }
      config.delta = delta_backup - programme_batterie.puissance;
      config.deltaneg = deltaneg_backup - programme_batterie.puissance;
      config.batterie_active = true;
      logging.Set_log_init("Batterie active, offset appliqué \n", true);
    }
   
    //else if (batterie_active ) {
      //  if ( ( programme_batterie.run && programme_batterie.stop_progr()) || gDisplayValues.temperature < programme_batterie.temperature)  {
    else if ( gDisplayValues.temperature < programme_batterie.temperature && config.batterie_active )  {
        config.delta = delta_backup;
        config.deltaneg = deltaneg_backup;
        config.batterie_active = false;
        logging.Set_log_init("Batterie désactivée, offset réinitialisé \n", true);
      }
    //}

  /// fin gestion de l'offset batterie

  //***********************************
  //************* Loop -  gestion des programmes
  //***********************************   
  if (programme_relay1.run) { 
    if (programme_relay1.stop_progr()) { 
      logging.Set_log_init(Stop_minuteur_relay1,true);
      digitalWrite(RELAY1 , HIGH); //correction bug de démarrage en GPIO 0
    }
  }
  else {
    if (programme_relay1.start_progr()){ 
      logging.Set_log_init(Start_minuteur_relay1,true);
      digitalWrite(RELAY1 , LOW); //correction bug de démarrage en GPIO 0
    }
  }
  if (programme_relay2.run) { 
    if (programme_relay2.stop_progr()) { 
      logging.Set_log_init(Stop_minuteur_relay2,true);
      digitalWrite(RELAY2 , LOW);
    }
  }
  else {
    if (programme_relay2.start_progr()){ 
      logging.Set_log_init(Start_minuteur_relay2,true);
      digitalWrite(RELAY2 , HIGH);
    }
  }
  //// protection contre l'absence de commande  
  if ( !programme.run && !programme_marche_forcee.run && !config.preheat ) { 
    unified_dimmer.auto_off(AUTO_OFF);
  }

  //***********************************
  //************* Loop -  fonction de reboot hebdomadaire ( lundi 00:00 )
  //***********************************   
  if (!AP) {
    time_reboot();
  }
  
  task_mem.task_loop = uxTaskGetStackHighWaterMark(nullptr);
  vTaskDelay(pdMS_TO_TICKS(10000));
}

//***********************************
//************* connect_to_wifi
//***********************************
void connect_to_wifi() {
  ///// AP WIFI INIT 
  if (AP || strcmp(configwifi.SID,"AP") == 0 ) {
      APConnect(); 
      gDisplayValues.currentState = DEVICE_STATE::UP;
      gDisplayValues.IP = WiFi.softAPIP();
      btStop();
      return; 
  }
  else {
    #if WIFI_ACTIVE == true
      WiFi.mode(WIFI_STA);
      WiFi.setSleep(false);
      WiFi.begin(configwifi.SID, configwifi.passwd); 
      int timeoutwifi=0;
      logging.Set_log_init(Start_Wifi_Network,true);
      logging.Set_log_init(configwifi.SID);
      logging.Set_log_init("\r\n");
    
      while ( WiFi.status() != WL_CONNECTED ) {
        delay(500);
        Serial.print(".");
        timeoutwifi++; 

        if (timeoutwifi > 40 ) {              
              logging.Set_log_init(timeout_AP_mode,true);          
              logging.Set_log_init(Wifi_State,true);
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
          } // (timeoutwifi > 40 )
      } // while ( WiFi.status() != WL_CONNECTED )

      //// timeout --> AP MODE 
      if ( timeoutwifi > 40 ) {
        WiFi.disconnect(); 
        serial_println(timeout_AP_mode);
        
        gDisplayValues.currentState = DEVICE_STATE::UP;
        APConnect(); 
        
      }

      serial_println(Wifi_connected);
      logging.Set_log_init(Wifi_connected,true);
      serial_println("IP address: ");
      serial_println(WiFi.localIP());
      serial_print(signal_level);
      serial_print(WiFi.RSSI());
      serial_println("dBm");
      gDisplayValues.currentState = DEVICE_STATE::UP;
      gDisplayValues.IP = WiFi.localIP();
      btStop();
      WiFi.setAutoReconnect(true);
    #endif // WIFI_ACTIVE == true
  } // else
} // if (AP || strcmp(configwifi.SID,"AP") == 0 )


//***********************************
//************* handler_before_reset
//***********************************
void handler_before_reset() {
  #ifndef LIGHT_FIRMWARE
    const int bufferSize = 150; // Taille du tampon pour stocker le message
    char raison[bufferSize];
    snprintf(raison, bufferSize, "reboot handler: %s ",logging.loguptime()); 
    client.publish("memory/Routeur", raison, true);
  #endif
}

//***********************************
//************* reboot_after_lost_wifi
//***********************************
void reboot_after_lost_wifi(int timeafterlost) {
  uptime::calculateUptime();
  if ( uptime::getHours() > timeafterlost ) { 
    delay(15000);  
    config.restart = true; 
  }
}

//***********************************
//************* function_off_screen
//***********************************
void IRAM_ATTR function_off_screen() {
  gDisplayValues.screenbutton = true;
}

//***********************************
//************* function_next_screen
//***********************************
void IRAM_ATTR function_next_screen(){
  gDisplayValues.nextbutton = true;
  gDisplayValues.option++; 
  if (gDisplayValues.option > 2 ) { gDisplayValues.option = 1 ;}; 
}

//***********************************
//************* boost
//***********************************
bool boost(){
    time_t now = time(nullptr);
    if (programme_marche_forcee.run) {
      // sur bug avec mqtt, on fait differement : on change l'heure de fin pour mettre à maintenant
      strftime(programme_marche_forcee.heure_arret, 6, "%H:%M", localtime(&now));
      now += TIME_BOOST;
      strftime(programme_marche_forcee.heure_demarrage, 6, "%H:%M", localtime(&now));
      return false;
    }
    
    // programation de l'heure de démarrage
    strftime(programme_marche_forcee.heure_demarrage, 6, "%H:%M", localtime(&now));
    
    // ajout de 2h
    now += TIME_BOOST;

    // programmaton de l'heure d'arrêt
    strftime(programme_marche_forcee.heure_arret, 6, "%H:%M", localtime(&now));

    // ajout de la température de consigne
    programme_marche_forcee.temperature = config.tmax;
    programme_marche_forcee.puissance = programme.puissance;
    return true;
}

