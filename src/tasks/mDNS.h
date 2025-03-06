#ifndef TASK_MDNS
#define TASK_MDNS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <ESPmDNS.h>
#include <WiFi.h>

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/enums.h"
#include "../config/config.h"

//***********************************
//************* Variables externes
//***********************************
extern Config config;
extern Logs logging;
extern SemaphoreHandle_t mutex;

//***********************************
//************* mdns_bye
//***********************************
void mdns_bye(String esp_name) {
  MDNS.end();   
}

//***********************************
//************* mdns_check
//***********************************
void mdns_check(String esp_name) {
  Serial.println("mDNS check pour : " + esp_name);
  IPAddress ip = MDNS.queryHost(esp_name.c_str());
  if (ip) {
    Serial.print("mDNS encore actif, IP trouvée : ");
    Serial.println(ip);
  } 
  else {
    Serial.println("mDNS arrêté, aucune IP trouvée.");
    Serial.println(ip);
  }    
}

//***********************************
//************* mdns_hello
//***********************************
void mdns_hello(String esp_name) {
    Serial.print("Démarrage mDNS pour : ");
    Serial.println(esp_name);

    int attempts = 0;
    while (!MDNS.begin(esp_name.c_str())) {
      Serial.println("Erreur MDNS, tentative...");
      attempts++;
      if (attempts > 5) {  // Si après 5 tentatives ça ne fonctionne pas
        Serial.println("Échec MDNS, redémarrage...");
        ESP.restart();
      }
      
      delay(1000);
    }
    
    Serial.println("mDNS démarré, attente avant vérification...");
    delay(2000); // Attendre pour que mDNS soit bien en place

    MDNS.addService("sunstain", "tcp", 80);
    MDNS.addServiceTxt("sunstain", "tcp", "name", esp_name.c_str());
    MDNS.addServiceTxt("sunstain", "tcp", "version", VERSION);
    MDNS.addServiceTxt("sunstain", "tcp", "compilation", COMPILE_NAME);
    MDNS.addServiceTxt("sunstain", "tcp", "fonction", "router");
    MDNS.addServiceTxt("sunstain", "tcp", "url", "https://www.sunstain.fr");
    MDNS.addServiceTxt("sunstain", "tcp", "update_url", "https://ota.apper-solaire.org/ota.php");    
}

//***********************************
//************* mdns_search
//***********************************
bool mdns_search(String type, uint16_t port) {
  int nrOfServices = MDNS.queryService(type.c_str(), "tcp");
  if (nrOfServices == 0) {
    Serial.println("No services were found.");
    return false;
  }
  else {
    Serial.print("Number of services found: ");
    Serial.println(nrOfServices);
    for (int i = 0; i < nrOfServices; i = i + 1) {
      if (MDNS.port(i) == port) {
        // condition si port 80, détecter un TXT function = dimmer
        if (port == 80) {
          int nrOfTxt = MDNS.numTxt(i);
          Serial.print("Recherche environnement dimmer Sunstain");
          for (int j = 0; j < nrOfTxt; j = j + 1) {
            // comparaison de la clé TXT  et valeuyr
            if (strcmp((MDNS.txtKey(i, j)).c_str(), "fonction") == 0 || strcmp((MDNS.txt(i, j)).c_str(), "dimmer") == 0) {
              String name_dimmer = MDNS.hostname(i) + ".local";
              name_dimmer.toCharArray(config.dimmer, name_dimmer.length() + 1);

              Serial.print(Dimmer_found);
              Serial.println(config.dimmer);
              logging.Set_log_init(Dimmer_found);
              logging.Set_log_init(config.dimmer);
              logging.Set_log_init("\r\n");
              
              // sauvegarde de la configuration
              config.saveConfiguration();
              return true;
            }
          } // for
        }
        else {
          String name_dimmer = MDNS.hostname(i) + ".local";
          name_dimmer.toCharArray(config.dimmer, name_dimmer.length() + 1);
          Serial.print(Dimmer_found);
          Serial.println(config.dimmer);
          logging.Set_log_init(Dimmer_found);
          logging.Set_log_init(config.dimmer);
          logging.Set_log_init("\r\n");
          
          // sauvegarde de la configuration
          config.saveConfiguration();
          
          return true;
        }
      }
    }
  }
  return false;
}

//***********************************
//************* mdns_discovery
//***********************************
void mdns_discovery(void *parameter) {    // NOSONAR
  for (;;) {        
    int Taskdelay = 10000;
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {  
      if (WiFi.status() == WL_CONNECTED && (strcmp(config.dimmer, "") == 0 || strcmp(config.dimmer, "none") == 0) ) { 
        if (!AP) {                       
            // recherche d'un dimmer
            if (!mdns_search("sunstain", 80)) {
                /// recherche de l'ancienne version dimmer  ( à supprimer 01/07/2025 )
                mdns_search("http", 1308);
            }
        }
      }
      else {
        Taskdelay = 600000; // 10 minutes
      }

      xSemaphoreGive(mutex);  // Libère le mutex
    }
    
    vTaskDelay(Taskdelay / portTICK_PERIOD_MS);
  } // for 
}

#endif
