#ifndef ENUMS
#define ENUMS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Preferences.h> 
#include <TimeLib.h>
#include <NTPClient.h>
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
#ifdef ESP32
  #include <FS.h>
  #include "SPIFFS.h"
#endif
#include <Arduino.h>
#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>
#endif

//***********************************
//************* Variables externes
//***********************************
#ifndef LIGHT_FIRMWARE
  extern PubSubClient client;
#endif

//***********************************
//************* Constantes
//***********************************
#define TABLEAU_SIZE 7 // NOSONAR
#define SECURITEPASS "MyPassword" // NOSONAR
#if DEBUG == true
  #define serial_print(x)  Serial.print (x)
  #define serial_println(x)  Serial.println (x)
#else
  #define serial_print(x)
  #define serial_println(x)
#endif

//***********************************
//************* Variable locales
//***********************************
int tableaudemo[TABLEAU_SIZE] = {180, 3, -150, 4, 150, 5, -180}; // NOSONAR

//***********************************
//************* class DEVICE_STATE
//***********************************
enum class DEVICE_STATE {
  // The state in which the device can be. This mainly affects what
  // is drawn on the display.
  CONNECTING_WIFI,
  CONNECTING_AWS,
  FETCHING_TIME,
  UP,
};

//***********************************
//************* struct ESP32Info
//***********************************
struct ESP32Info {
  String chipModel;
  String chipModelShort;
  int chipRevision;
  int chipCores;
  String boardName;
  uint64_t chipID;
};

//***********************************
//************* struct DisplayValues
//***********************************
struct DisplayValues {
  // Place to store all the variables that need to be displayed.
  // All other functions should update these!
  double watt;
  bool wattIsValid; // Jeton pour measureEletricity.h permettant d'appliquer la puissance au dimmer (local ou distant)
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
  bool screenbutton;
  bool nextbutton;
  int option=0;
  float temperature = 0.0 ;
  int Fronius_conso; 
  int Fronius_prod; 
  int Fronius_totalconso;
  float celsius;
  //int page=1; 
  int Shelly=-1;
  double enp_prod_whLifetime;
  double enp_cons_whLifetime;
  double enp_current_power_consumption;
  double enp_current_power_production;
  int puissance_route=0;
  bool dimmer_disengaged=false;
  const String pvname = "PV-ROUTER-" + WiFi.macAddress().substring(12,14) + WiFi.macAddress().substring(15,17);
  int serial_timeout = 0 ; ///arret du service serial après x loop d'inactivité
};

//***********************************
//************* struct Config
//***********************************
struct Config {
  public:
    char hostname[16];  // à vérifier si on peut pas le supprimer // NOSONAR
    int port;  // idem  
    char apiKey[64];  // clé pour jeedom // NOSONAR 
    bool UseDomoticz;
    bool UseJeedom;
    int IDX;  // IDX pour domoticz
    int IDXdallas; // IDX pour domoticz
    char otapassword[64]; // NOSONAR
    int delta; 
    int deltaneg;
    int cosphi; // plus utilisé
    int readtime;  // temps de lecture des capteurs
    int cycle;  // cycle de lecture des capteurs
    bool sending; 
    bool autonome; // si dimmer en local 
    char dimmer[64];  // adresse IP du dimmer // NOSONAR
    bool dimmerlocal; // si dimmer en local
    float facteur; // facteur de correction de la puissance
    int num_fuse;
    int localfuse;
    int tmax;
    int trigger;
    bool mqtt;
    char mqttserver[16]; // NOSONAR
    int mqttport; 
    int IDXdimmer;
    bool dallas_present;
    
    bool polarity; 
    char Publish[100]; // NOSONAR
    int  ScreenTime;
    int voltage; 
    int offset; 
    bool flip;
    bool restart;
    char topic_Shelly[100];  // NOSONAR
    bool Shelly_tri;
    bool Shelly_mode=false;
    int SCT_13=30;
    int charge1;  // Puissance de la charge 1 déclarée dans la page web
    // @brief  // Puissance de la charge 2 déclarée dans la page web
    int charge2; 
    // @brief  // Puissance de la charge 3 déclarée dans la page web
    int charge3;
    // @brief  // Somme des 3 charges déclarées dans la page web
    int charge;

    Preferences preferences;
    const char *filename_conf = "/config.json";

  public:
    bool sauve_polarity() {
      preferences.begin("polarity", false);
      preferences.putBool("polarity",polarity);
      preferences.end();

      return true; 
    }

    bool recup_polarity() {
      preferences.begin("polarity", false);
      polarity = preferences.getBool("polarity", false);
      preferences.end();

      return true; 
    }
    
    void calcul_charge() {
      charge = charge1 + charge2 + charge3;
    }

    void check_trigger() {
      if (trigger < 0) { 
        trigger = 0; 
      }
      if (trigger > 100) { 
        trigger = 100; 
      }
    }

    //***********************************
    //************* Gestion de la configuration - Lecture du fichier de configuration
    //***********************************
    // Loads the configuration from a file
    String loadConfiguration() {
      String message = "";
      // Open file for reading
      File configFile = SPIFFS.open(filename_conf, "r");

      // Allocate a temporary JsonDocument
      // Don't forget to change the capacity to match your requirements.
      // Use arduinojson.org/v6/assistant to compute the capacity.
      JsonDocument doc;

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, configFile);
      if (error) {
        Serial.println(F("Failed to read, using default configuration in function loadConfiguration"));        
        message = "Failed to read, using default configuration in function loadConfiguration\r\n";
      }
      
      // Copy values from the JsonDocument to the Config
      port = doc["port"] | 8080;
      strlcpy(hostname,                  // <- destination
              doc["hostname"] | "none", // <- source
              sizeof(hostname));         // <- destination's capacity
      
      strlcpy(apiKey,                  // <- destination
              doc["apiKey"] | "Myapikeystring", // <- source
              sizeof(apiKey));         // <- destination's capacity          
      UseDomoticz = doc["UseDomoticz"] | false; 
      UseJeedom = doc["UseJeedom"] | false; 
      IDX = doc["IDX"] | 100; 
      IDXdimmer = doc["IDXdimmer"] | 110; 
      IDXdallas = doc["IDXdallas"] | 900; 
      strlcpy(otapassword,                  // <- destination
              doc["otapassword"] | "Pvrouteur2", // <- source
              sizeof(otapassword));         // <- destination's capacity
      facteur = doc["facteur"] | 0.86; 
      delta = doc["delta"] | 50; 
      num_fuse = 500;
      deltaneg = doc["deltaneg"] | 0; 
      cosphi = doc["cosphi"] | 5; 
      readtime = doc["readtime"] | 555;
      cycle = doc["cycle"] | 72;

      charge1 = doc["charge1"] | 3000;
      charge2 = doc["charge2"] | 0;
      charge3 = doc["charge3"] | 0;
      // provisionne la somme des charges
      calcul_charge();

      sending = doc["sending"] | true;
      autonome = doc["autonome"] | true;
      mqtt = doc["mqtt"] | true;
      mqttport = doc["mqttport"] | 1883;
      
      dimmerlocal = doc["dimmerlocal"] | false;
      flip = doc["flip"] | true;
      tmax = doc["tmax"] | 65;
      localfuse = doc["localfuse"] | 20;
      voltage = doc["voltage"] | 233;
      offset = doc["offset"] | -10;
      SCT_13 = doc["SCT_13"] | 30;
      trigger = doc["trigger"] | 10;

      // passe à true si au moins une fois elle a été présente
      dallas_present = doc["dallas_present"] | false;
      check_trigger();

      polarity = doc["polarity"] | false;
      strlcpy(dimmer,                  // <- destination
              doc["dimmer"] | "none", // <- source
              sizeof(dimmer));         // <- destination's capacity
      strlcpy(mqttserver,                  // <- destination
              doc["mqttserver"] | "none", // <- source
              sizeof(mqttserver));         // <- destination's capacity
      strlcpy(Publish,                  // <- destination
              doc["Publish"] | "domoticz/in", // <- source
              sizeof(Publish));         // <- destination's mqtt
      ScreenTime = doc["screentime"] | 0 ; // timer to switch of screen
      strlcpy(topic_Shelly,                  // <- destination
              doc["topic_Shelly"] | "none", // <- source
              sizeof(topic_Shelly));
      Shelly_tri = doc["Shelly_tri"] | false; /// récupération shelly mode triphasé ou monophasé
      configFile.close();

      recup_polarity();
      
      message = "config file loaded\r\n";
      
      return message;
    } //String loadConfiguration()

    //***********************************
    //************* Gestion de la configuration - sauvegarde du fichier de configuration
    //***********************************

    String saveConfiguration() {
      String message = "";
      // Open file for writing
      File configFile = SPIFFS.open(filename_conf, "w");

      if (!configFile) {
        Serial.println(F("Failed to open config file for writing in function Save configuration"));
        message = "Failed to open config file for writing in function Save configuration\r\n";
      
        return message;
      } 

      // vérification de la valeur de trigger
      check_trigger();  
      // Allocate a temporary JsonDocument
      // Don't forget to change the capacity to match your requirements.
      // Use arduinojson.org/assistant to compute the capacity.
      JsonDocument doc;

      // Set the values in the document
      doc["hostname"] = hostname;
      doc["port"] = port;
      doc["apiKey"] = apiKey;
      doc["UseDomoticz"] = UseDomoticz;
      doc["UseJeedom"] = UseJeedom;
      doc["IDX"] = IDX;
      doc["IDXdimmer"] = IDXdimmer;
      doc["IDXdallas"] = IDXdallas;
      doc["otapassword"] = otapassword;
      doc["delta"] = delta;
      doc["deltaneg"] = deltaneg;
      doc["cosphi"] = cosphi;
      doc["readtime"] = readtime;
      doc["cycle"] = cycle;
      doc["sending"] = sending;
      doc["autonome"] = autonome;
      doc["dimmer"] = dimmer;
      doc["dimmerlocal"] = dimmerlocal;
      doc["tmax"] = tmax;
      doc["localfuse"] = localfuse;
      doc["facteur"] = facteur;
      doc["trigger"] = trigger;
      doc["mqtt"] = mqtt;
      //protection contre les champs vides qui font planter le programme
      if (strlen(mqttserver) == 0) { //NOSONAR
        strlcpy(mqttserver,"none",16); 
      }
      doc["mqttserver"] = mqttserver; 
      doc["mqttport"] = mqttport; 
      doc["charge1"] = charge1;
      doc["charge2"] = charge2;
      doc["charge3"] = charge3;
      doc["polarity"] = polarity; 
      doc["Publish"] = Publish;
      doc["screentime"] = ScreenTime; 
      doc["voltage"] = voltage; 
      doc["offset"] = offset; 
      doc["flip"] = flip; 
      doc["topic_Shelly"] = topic_Shelly; 
      doc["Shelly_tri"] = Shelly_tri;
      doc["SCT_13"] = SCT_13;
      doc["dallas_present"] = dallas_present;
      message = "config file saved\r\n";

      // Serialize JSON to file
      if (serializeJson(doc, configFile) == 0) {
        Serial.println(F("Failed to write to file in function Save configuration "));
        message = "Failed to write to file in function Save configuration\r\n";
      }

      // Close the file
      configFile.close();
      return message;
    } // String saveConfiguration() 
}; // Struct Config

//***********************************
//************* struct Configwifi
//***********************************
struct Configwifi { 
  Preferences preferences;

  char SID[32]; // NOSONAR
  char passwd[64]; // NOSONAR

  public:bool sauve_wifi() {
    preferences.begin("credentials", false);
    preferences.putString("ssid",SID);
    preferences.putString("password",passwd);
    preferences.end();

    return true; 
  }

  public:bool recup_wifi() {
    preferences.begin("credentials", false);
    String tmp; 
    tmp = preferences.getString("ssid", "AP");
    tmp.toCharArray(SID,32);
    tmp = preferences.getString("password", "");
    tmp.toCharArray(passwd,64);
    preferences.end();
    if (strcmp(SID,"") == 0) { 
      return false; 
    }

    return true;
  }
}; // struct Configwifi

//***********************************
//************* struct Mqtt
//***********************************
struct Mqtt {
  public: 
    char username[64]; // NOSONAR
    char password[64]; // NOSONAR
    bool HA;
    const char *mqtt_conf = "/mqtt.json";

  String loadmqtt() {
    String message = "";
    // Open file for reading
    File configFile = SPIFFS.open(mqtt_conf, "r");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
      Serial.println(F("Failed to read MQTT config "));      
      message = "Failed to read MQTT config\r\n";
      return message;
    }
    
    // Copy values from the JsonDocument to the Config    
    strlcpy(username,                  // <- destination
            doc["MQTT_USER"] | "", // <- source
            sizeof(username));         // <- destination's capacity
    strlcpy(password,                  // <- destination
            doc["MQTT_PASSWORD"] | "", // <- source
            sizeof(password));         // <- destination's capacity
    HA = doc["HA"] | true;
    configFile.close();
    
    message = "MQTT config loaded\r\n";

    return message;    
  } // String loadmqtt()

  String savemqtt() {
    String message = "";
    // Open file for writing
    File configFile = SPIFFS.open(mqtt_conf, "w");
    if (!configFile) {
      Serial.println(F("Failed to open config file for writing in function mqtt configuration"));      
      message = "Failed to open config file for writing in function mqtt configuration\r\n";
      return message;
    } 

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/assistant to compute the capacity.
    JsonDocument doc;

    // Set the values in the document
    doc["MQTT_USER"] = username;
    doc["MQTT_PASSWORD"] = password;
    doc["HA"] = HA;
    message = "MQTT config saved\r\n";
    // Serialize JSON to file
    if (serializeJson(doc, configFile) == 0) {
      Serial.println(F("Failed to write to file in function Save configuration "));
      message = "Failed to write to file in function Save configuration\r\n";
    }

    // Close the file
    configFile.close();

    return message;
  } //String savemqtt()
}; // struct Mqtt

//***********************************
//************* struct Memory
//***********************************
struct Memory {
  public: 
    int task_keepWiFiAlive2=5000;   
    int task_serial_read_task=5000;
    int task_dallas_read=5000;
    int task_updateDimmer=5000; 
    int task_GetDImmerTemp=5000;
    int task_measure_electricity=5000;
    int task_send_mqtt=5000;
    int task_switchDisplay=5000;
    int task_updateDisplay=5000;
    int task_loop=5000; 
    int task_mdns_discovery = 5000;    
}; // struct Memory

//***********************************
//************* struct Configmodule
//***********************************
struct Configmodule {
  public: 
    char hostname[16]; // NOSONAR
    char port[5]; // NOSONAR
    bool enphase_present=false; 
    bool Fronius_present=false;
    char envoy[5]; // NOSONAR
    char version[2]; // NOSONAR 
    char token[512]; //correction suite remonté de multinet // NOSONAR
}; // struct Configmodule


//***********************************
//************* struct Configmodule
/// @brief  partie délicate car pas mal d'action sur la variable log_init et donc protection de la variable ( pour éviter les pb mémoire )
//***********************************
struct Logs {
  private:
    char log_init[LOG_MAX_STRING_LENGTH]; // NOSONAR
    int MaxString = LOG_MAX_STRING_LENGTH * .9 ;
    bool lock_log = false; // protection en écriture de la variable log_init

  public:
    bool sct;
    bool sinus;
    bool power;
    bool serial=false; 

  ///setter log_init --> ajout du texte dans la log
public: void Set_log_init(const char* setter, bool logtime = false) {

    if (lock_log) {
      return;
    } else {
        lock_log = true;
        // Vérifier si la longueur de la chaîne ajoutée ne dépasse pas LOG_MAX_STRING_LENGTH
        
        size_t setterLength = strlen(setter);
        size_t logInitLength = strlen(log_init);
        size_t logUptimeLength = strlen(loguptime()); 
        size_t maxLength = LOG_MAX_STRING_LENGTH - 1; // Toujours laisser de la place pour le '\0'

        // Vérifier si la taille totale dépasse la capacité
        if (setterLength + logInitLength >= maxLength) {
          reset_log_init();
        } else {
          // Vérifier si on peut ajouter le uptime
          if (logtime && (setterLength + logInitLength + logUptimeLength >= maxLength)) {
            // Ne pas ajouter l'uptime
          }

          // Ajouter l'uptime en premier si nécessaire
          if (logtime) {
            strncat(log_init, loguptime(), maxLength - logInitLength - 1);
          }

          // Ajouter le setter
          strncat(log_init, setter, maxLength - logInitLength - 1);
        }
        lock_log = false;
    }
  }

  ///getter log_init
  public:String Get_log_init() {return log_init; }

  //clean log_init
  public:void clean_log_init() {
    if (strlen(log_init) > (LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/5)) ) {
      reset_log_init();
    }

    ///si risque de fuite mémoire
    if (strlen(log_init) >(LOG_MAX_STRING_LENGTH - (LOG_MAX_STRING_LENGTH/50)) ) {
      ESP.restart();  
    }
  }

  //reset log_init
  public:void reset_log_init() {
    log_init[0] = '\0';
    strcat(log_init,"197}11}1");
  }

  char *loguptime(bool day=false) {
    static char uptime_stamp[20]; // Vous devrez définir une taille suffisamment grande pour stocker votre temps // NOSONAR
    time_t maintenant;
    time(&maintenant);
    if (day) {
      strftime(uptime_stamp, sizeof(uptime_stamp), "%d/%m/%Y %H:%M:%S\t ", localtime(&maintenant));
    } 
    else {
      strftime(uptime_stamp, sizeof(uptime_stamp), "%H:%M:%S\t ", localtime(&maintenant));
    }
    
    return uptime_stamp;
  }
};

//***********************************
//************* struct Dallas
//***********************************
struct Dallas {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12]; // NOSONAR
  byte addr[8]; // NOSONAR
  float celsius = 0.00 ;
  bool security = 0;
  bool detect = false;
  bool lost = false; 
};

//***********************
//**** struct HA 
//***********************
#ifndef LIGHT_FIRMWARE
struct HA {
  /* HA */
  private:String name; 
  public:void Set_name(String setter) {name=setter; }
  public:String Get_name() {return name;}

  private:String dev_cla; 
  public:void Set_dev_cla(String setter) {
    dev_cla=setter; 
    if (setter=="switch") { 
      topic = "homeassistant/switch/"+ String(node_id) +"/"; 
    }
  }

  private:String unit_of_meas; 
  public:void Set_unit_of_meas(String setter) {
    unit_of_meas=setter; 
  }

  private:String stat_cla; 
  public:void Set_stat_cla(String setter) {
    stat_cla=setter; 
  }

  private:String entity_category; 
  public:void Set_entity_category(String setter) {
    entity_category=setter; 
  }

  private:String icon; 
  public:void Set_icon(String setter) {
    icon = R"("ic": ")" + setter + R"(", )"; 
  }

  bool cmd_t; 

  private:String IPaddress;
  private:String state_topic; 
  private:String stat_t; 
  private:String avty_t;

  private:const String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
  private:const String node_id = String("PvRouter-") + node_mac; 
  public:String topic = "homeassistant/sensor/"+ node_id +"/";
      
  private:String device_declare() { 
    String info = R"(
        "dev": {
            "ids": ")" + String(node_id) + R"(",
            "name": ")" + String(node_id) + R"(",
            "sw": "PvRouter )" + String(VERSION) + R"(",
            "mdl": "ESP32 TTGO )" + IPaddress + R"(",
            "mf": "Cyril Poissonnier",
            "cu": "http://)" + IPaddress + R"("
        })";
        return info;
    }

  private:String uniq_id; 
  private:String value_template; 

  public:void discovery() {
    String dev_switch= "";

    if (dev_cla =="switch" ) { 
      dev_switch = R"(
          "pl_on": "{ \")" + name + R"(\" : \"1\" }",
          "pl_off": "{ \")" + name + R"(\" : \"0\" }",
          "stat_on": 1,
          "stat_off": 0,
          )";
    }
    else {
      dev_switch = R"(
        "unit_of_meas": ")" + unit_of_meas + R"(",
        "stat_cla": ")" + stat_cla + R"(",
      )"; 
    }

    IPaddress =   WiFi.localIP().toString() ;
    String device= "{ \"dev_cla\": \""+dev_cla+"\","
      "\"name\": \""+ name +"-"+ node_mac + "\"," 
      // "\"state_topic\": \""+ topic +"state\","
      "\"stat_t\": \""+ topic +"state"+name+"\","
      "\"avty_t\": \""+ topic +"status\","
      "\"uniq_id\": \""+ node_mac + "-" + name +"\", "
      "\"val_tpl\": \"{{ value_json."+name +" }}\", "
      + dev_switch + 
      "\"cmd_t\": \""+ topic +"command\","
      "\"cmd_tpl\": \"{{ value_json."+name +" }}\", "
      "\"exp_aft\": \""+ MQTT_INTERVAL +"\", "
      + icon
      + device_declare() + 
    "}";

    if (dev_cla =="" ) { 
      dev_cla = name; 
    }
    char final_topic[100];
    if (strlen(name.c_str()) != 0 ) {
      snprintf(final_topic, sizeof(final_topic), "%s%s/config", topic, name.c_str());
      client.publish(final_topic , device.c_str() , true); // déclaration autoconf dimmer
    }
    else {
      snprintf(final_topic, sizeof(final_topic), "%sconfig", topic);
      client.publish(final_topic, device.c_str() , true); // déclaration autoconf dimmer
    }       
  } // discovery

  public:void send(String value) {
    // vérification que value est un nombre
    String message ="";
    if (value.toFloat() == 0 && value != "0") { 
      message = "  { \""+name+"\" : \"" + value.c_str() + "\"  } "; 
    }
    else {
      message = "  { \""+name+"\" : "+ value.c_str() + "}"; 
    }

    client.publish((topic+"state"+name).c_str() , message.c_str(), false); // false for exp_aft in discovery
  }  // send()
};
#endif

//***********************
//**** struct epoc
//***********************
struct epoc {
  public:
    int heure;
    int minute;
    int seconde;
    int jour;
    int mois;
    int annee;
};

#endif
