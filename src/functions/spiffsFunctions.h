//***********************************
//************* SPIFFS 
//***********************************
#ifndef SPIFFS_FUNCTIONS
#define SPIFFS_FUNCTIONS


// File System
#ifdef ESP32
#include <FS.h>
#include "SPIFFS.h"
#include "config/enums.h"
#endif

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson


//extern String loguptime(); 
extern char *loguptime2();

const char *filename_conf = "/config.json";
const char *log_conf = "/log.txt";
extern Config config; 


//***********************************
//************* Gestion de la configuration - Lecture du fichier de configuration
//***********************************
// Loads the configuration from a file
void loadConfiguration(const char *filename, Config &config) {
  // Open file for reading
  File configFile = SPIFFS.open(filename_conf, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(1024);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read, using default configuration in function loadConfiguration"));
    
    logging.Set_log_init("Failed to read, using default configuration in function loadConfiguration\r\n",true);


  }

  
  // Copy values from the JsonDocument to the Config
  config.port = doc["port"] | 8080;
  strlcpy(config.hostname,                  // <- destination
          doc["hostname"] | "none", // <- source
          sizeof(config.hostname));         // <- destination's capacity
  
  strlcpy(config.apiKey,                  // <- destination
          doc["apiKey"] | "Myapikeystring", // <- source
          sizeof(config.apiKey));         // <- destination's capacity
      
  config.UseDomoticz = doc["UseDomoticz"] | false; 
  config.UseJeedom = doc["UseJeedom"] | false; 
  config.IDX = doc["IDX"] | 100; 
  config.IDXdimmer = doc["IDXdimmer"] | 110; 
  config.IDXdallas = doc["IDXdallas"] | 900; 
  strlcpy(config.otapassword,                  // <- destination
          doc["otapassword"] | "Pvrouteur2", // <- source
          sizeof(config.otapassword));         // <- destination's capacity
  
  config.facteur = doc["facteur"] | 0.86; 
  config.delta = doc["delta"] | 50; 
  config.num_fuse = doc["fuse"] | 70;
  config.deltaneg = doc["deltaneg"] | 0; 
  config.cosphi = doc["cosphi"] | 5; 
  config.readtime = doc["readtime"] | 555;
  config.cycle = doc["cycle"] | 72;

  config.resistance = doc["resistance"] | 1000;
  config.sending = doc["sending"] | true;
  config.autonome = doc["autonome"] | true;
  config.mqtt = doc["mqtt"] | true;
  config.mqttport = doc["mqttport"] | 1883;
  
  config.dimmerlocal = doc["dimmerlocal"] | false;
  config.flip = doc["flip"] | true;
  config.tmax = doc["tmax"] | 65;
  config.localfuse = doc["localfuse"] | 50;
  config.voltage = doc["voltage"] | 233;
  config.offset = doc["offset"] | -10;
  config.relayoff = doc["relayoff"] | 95;
  config.relayon = doc["relayon"] | 100;

  config.polarity = doc["polarity"] | false;
  strlcpy(config.dimmer,                  // <- destination
          doc["dimmer"] | "none", // <- source
          sizeof(config.dimmer));         // <- destination's capacity

   strlcpy(config.mqttserver,                  // <- destination
          doc["mqttserver"] | "none", // <- source
          sizeof(config.mqttserver));         // <- destination's capacity
   strlcpy(config.Publish,                  // <- destination
          doc["Publish"] | "domoticz/in", // <- source
          sizeof(config.Publish));         // <- destination's mqtt
  config.ScreenTime = doc["screentime"] | 0 ; // timer to switch of screen
   
   strlcpy(config.topic_Shelly,                  // <- destination
          doc["topic_Shelly"] | "none", // <- source
          sizeof(config.topic_Shelly));

  configFile.close();
  
  logging.Set_log_init("config file loaded\r\n",true);
}

//***********************************
//************* Gestion de la configuration - sauvegarde du fichier de configuration
//***********************************

void saveConfiguration(const char *filename, const Config &config) {
  
  // Open file for writing
   File configFile = SPIFFS.open(filename_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing in function Save configuration"));
    
    logging.Set_log_init("Failed to open config file for writing in function Save configuration\r\n",true);
    return;
  } 

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  DynamicJsonDocument doc(1024);

  // Set the values in the document
  doc["hostname"] = config.hostname;
  doc["port"] = config.port;
  doc["apiKey"] = config.apiKey;
  doc["UseDomoticz"] = config.UseDomoticz;
  doc["UseJeedom"] = config.UseJeedom;
  doc["IDX"] = config.IDX;
  doc["IDXdimmer"] = config.IDXdimmer;
  doc["IDXdallas"] = config.IDXdallas;
  doc["otapassword"] = config.otapassword;
  doc["delta"] = config.delta;
  doc["deltaneg"] = config.deltaneg;
  doc["cosphi"] = config.cosphi;
  doc["readtime"] = config.readtime;
  doc["cycle"] = config.cycle;
  doc["sending"] = config.sending;
  doc["autonome"] = config.autonome;
  doc["dimmer"] = config.dimmer;

  doc["dimmerlocal"] = config.dimmerlocal;
  doc["tmax"] = config.tmax;
  doc["localfuse"] = config.localfuse;

  doc["facteur"] = config.facteur;
  doc["fuse"] = config.num_fuse;
  doc["mqtt"] = config.mqtt;
  doc["mqttserver"] = config.mqttserver; 
  doc["mqttport"] = config.mqttport; 
  doc["resistance"] = config.resistance;
  doc["polarity"] = config.polarity; 
  doc["Publish"] = config.Publish;
  doc["screentime"] = config.ScreenTime; 
  doc["voltage"] = config.voltage; 
  doc["offset"] = config.offset; 
  doc["flip"] = config.flip; 
  
  doc["relayon"] = config.relayon; 
  doc["relayoff"] = config.relayoff; 
  doc["topic_Shelly"] = config.topic_Shelly; 


  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file in function Save configuration "));
    
  }

  // Close the file
  configFile.close();
}



///////////////////////////////////
////////config MQTT
///////////////////////////////////
const char *mqtt_conf = "/mqtt.json";
extern Mqtt configmqtt;

bool loadmqtt(const char *filename, Mqtt &configmqtt) {
  // Open file for reading
  File configFile = SPIFFS.open(mqtt_conf, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(512);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read MQTT config "));
    
    logging.Set_log_init("Failed to read MQTT config\r\n",true);
    return false;
  }

  
  // Copy values from the JsonDocument to the Config
  
  strlcpy(configmqtt.username,                  // <- destination
          doc["MQTT_USER"] | "", // <- source
          sizeof(configmqtt.username));         // <- destination's capacity
  
  strlcpy(configmqtt.password,                  // <- destination
          doc["MQTT_PASSWORD"] | "", // <- source
          sizeof(configmqtt.password));         // <- destination's capacity
  configmqtt.HA = doc["HA"] | true;
  configFile.close();
  
  logging.Set_log_init("MQTT config loaded\r\n",true);

return true;    
}

void savemqtt(const char *filename, const Mqtt &configmqtt) {
  
  // Open file for writing
   File configFile = SPIFFS.open(mqtt_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing in function mqtt configuration"));
    
    logging.Set_log_init("Failed to open config file for writing in function mqtt configuration\r\n",true);
    return;
  } 

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  DynamicJsonDocument doc(512);

  // Set the values in the document
  doc["MQTT_USER"] = configmqtt.username;
  doc["MQTT_PASSWORD"] = configmqtt.password;
  doc["HA"] = configmqtt.HA;
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file in function Save configuration "));
    
    logging.Set_log_init("Failed to write to file in function Save configuration\r\n",true);
    
  }

  // Close the file
  configFile.close();
}


///////////////////////////////////
///// config Wifi 
///////////////////////////////////
const char *wifi_conf = "/wifi.json";
extern Configwifi configwifi; 
extern Logs logging;

bool loadwifi(const char *filename, Configwifi &configwifi) {
  // Open file for reading
  File configFile = SPIFFS.open(wifi_conf, "r");

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(512);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read wifi config, AP mode activated "));   
    logging.Set_log_init("Failed to read wifi config, AP mode activated\r\n",true);

    return false;
  }
  
  // Copy values from the JsonDocument to the Config
  
  strlcpy(configwifi.SID,                  // <- destination
          doc["SID"] | "xxx", // <- source
          sizeof(configwifi.SID));         // <- destination's capacity
  
  strlcpy(configwifi.passwd,                  // <- destination
          doc["passwd"] | "xxx", // <- source
          sizeof(configwifi.passwd));         // <- destination's capacity
  configFile.close();

return true;    
}

void saveWifi(const char *filenamewifi, const Configwifi &configwifi) {
  
  // Open file for writing
   File configFile = SPIFFS.open(wifi_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing in function wifi configuration"));    
    logging.Set_log_init("Failed to open config file for writing in function wifi configuration\r\n",true);
    return;
  } 

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  DynamicJsonDocument doc(1024);

  // Set the values in the document
  doc["SID"] = configwifi.SID;
  doc["passwd"] = configwifi.passwd;
  
  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file in function Save configuration "));    
    logging.Set_log_init("Failed to write to file in function Save configuration\r\n",true);
    
  }

  // Close the file
  configFile.close();
}

/// sauvegarde des logs avant reboot

void savelogs(String log) {
  
  // Open file for writing
   File configFile = SPIFFS.open(log_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for logs"));
    
    logging.Set_log_init("Failed to open config file for logs\r\n",true);
    return;
  } 

  // ajout dans le fichier les logs 
  configFile.println(log);
  
  // Close the file
  configFile.close();
}

/// chargement des logs
void loadlogs() {
  // Open file for reading
  File configFile = SPIFFS.open(log_conf, "r");

/// chargement de la log jusqu'a la fin du fichier EOF
  while (configFile.available()) {
    logging.Set_log_init(configFile.readStringUntil('\n'));
  }
  configFile.close();
  
  logging.Set_log_init("logs loaded\r\n",true);
   

}


#endif