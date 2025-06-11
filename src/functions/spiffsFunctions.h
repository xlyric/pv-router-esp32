#ifndef SPIFFS_FUNCTIONS
#define SPIFFS_FUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson
#ifdef ESP32
  #include <FS.h>
  #include "SPIFFS.h"  
#endif

//***********************************
//************* PROGRAMME PV ROUTER
//***********************************
#include "config/enums.h"

//***********************************
//************* Variables externes
//***********************************
extern Logs logging;

//***********************************
//************* Variables locales
//***********************************
constexpr const char *log_conf = "/log.txt";

//***********************************
//************* savelogs()
//************* sauvegarde des logs avant reboot
//***********************************
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

//***********************************
//************* loadlogs()
//***********************************
void loadlogs() {
  // Open file for reading
  File configFile = SPIFFS.open(log_conf, "r");

  // chargement de la log jusqu'a la fin du fichier EOF
  while (configFile.available()) {
    char line[64]; // Ajustez la taille du tableau en fonction de la longueur maximale de la ligne
    sniprintf(line, sizeof(line), "%s", configFile.readStringUntil('\n')); // Formatage de la ligne pour éviter les problèmes de buffer overflow
    logging.Set_log_init(line);
  }
  configFile.close();
  
  logging.Set_log_init(logs_loaded,true);
}

//***********************************
//************* test_fs_version()
//***********************************
bool test_fs_version(bool log = true) {
  // SPIFFS.begin() call is needed to use filesystem
  if (!SPIFFS.begin(true)) {
     if (log) { logging.Set_log_init(Fail_mount_FS); }
    return false;
  }

  // Open file for reading
  File file = SPIFFS.open("/version", "r");
  if (!file) {
     if (log) { logging.Set_log_init(FS_version_is_not_same); }
    return false;
  }

  // comparaison entre le contenu du fichier et la version du code FS_RELEASE
  String version = file.readStringUntil('\n');
  file.close();
  if (version.toInt() < FS_RELEASE ) {
    if (log) { 
      logging.Set_log_init(FS_version_is_not_same); 
    }
 
    return false;
  }

  if (log) {  
    logging.Set_log_init(FS_version_is_same); 
  }
  
  return true;
}
#endif
