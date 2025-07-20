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
void mdns_bye(const char* esp_name) {
  MDNS.end();   
}

//***********************************
//************* mdns_check
//***********************************
void mdns_check(const char* esp_name) {
  Serial.print("mDNS check pour : ");
  Serial.println(esp_name);

  IPAddress ip = MDNS.queryHost(esp_name);

  if (ip != IPAddress(0, 0, 0, 0)) {
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
void mdns_hello(const char* esp_name) {
  Serial.print("Démarrage mDNS pour : ");
  Serial.println(esp_name);

  int attempts = 0;
  while (!MDNS.begin(esp_name)) {
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
  MDNS.addServiceTxt("sunstain", "tcp", "name", esp_name);
  MDNS.addServiceTxt("sunstain", "tcp", "version", VERSION);
  MDNS.addServiceTxt("sunstain", "tcp", "compilation", COMPILE_NAME);
  MDNS.addServiceTxt("sunstain", "tcp", "fonction", "router");
  MDNS.addServiceTxt("sunstain", "tcp", "url", "https://www.sunstain.fr");
  MDNS.addServiceTxt("sunstain", "tcp", "update_url", "https://ota.apper-solaire.org/ota.php");    
}

//***********************************
//************* mdns_search
//***********************************
bool mdns_search(const char* type, uint16_t port) {
  int nrOfServices = MDNS.queryService(type, "tcp");
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
              char buffer[32];
              snprintf(buffer, sizeof(buffer), "%s.local", MDNS.hostname(i).c_str());
              strncpy(config.dimmer, buffer, sizeof(config.dimmer) - 1);
              config.dimmer[sizeof(config.dimmer) - 1] = '\0'; // Assurer la terminaison de la chaîne

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
          char buffer[32];
          snprintf(buffer, sizeof(buffer), "%s.local", MDNS.hostname(i).c_str());
          strncpy(config.dimmer, buffer, sizeof(config.dimmer) - 1);
          config.dimmer[sizeof(config.dimmer) - 1] = '\0'; // Assurer la terminaison de la chaîne
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
      if (WiFi.status() == WL_CONNECTED && (strcmp(config.dimmer, "") == 0 || strcmp(config.dimmer, "none") == 0) ) { 
        if (!AP) {                       
          // recherche d'un dimmer
          if (!mdns_search("sunstain", 80)) {
            // recherche de l'ancienne version dimmer  ( à supprimer 01/07/2025 )
            mdns_search("http", 1308);
          }
        }
      }
      else {
        Taskdelay = 600000+(esp_random() % 61) - 30; // 10 minutes
      }
    //si config.dimmer défini, on arrête la tâche
    if (strcmp(config.dimmer, "") != 0 && strcmp(config.dimmer, "none") != 0) {
      vTaskDelete(NULL);
      Serial.println("Service mDNS arrêté, dimmer trouvé.");
    }    
    vTaskDelay(Taskdelay / portTICK_PERIOD_MS);
  } // for 
}





#endif
