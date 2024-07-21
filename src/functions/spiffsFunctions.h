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


constexpr const char *log_conf = "/log.txt";


/// sauvegarde des logs avant reboot

void savelogs(String log) {
 
        log = String(logging.loguptime(true)) + log;

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
  
  logging.Set_log_init(logs_loaded,true);
   

}

bool test_fs_version() {
  // SPIFFS.begin() call is needed to use filesystem
  if (!SPIFFS.begin(true)) {
    logging.Set_log_init(Fail_mount_FS);
    return false;
  }
  // Open file for reading
  File file = SPIFFS.open("/version", "r");
  if (!file) {
    logging.Set_log_init(FS_version_is_not_same);
    return false;
  }
   // comparaison entre le contenu du fichier et la version du code FS_RELEASE
  String version = file.readStringUntil('\n');
  file.close();
  if (version.toInt() < FS_RELEASE ) {
    logging.Set_log_init(FS_version_is_not_same);
    
    return false;
  }
  logging.Set_log_init(FS_version_is_same);
  return true;
}
#endif