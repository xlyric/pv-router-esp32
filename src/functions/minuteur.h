#ifndef MINUTEUR_FUNCTIONS
#define MINUTEUR_FUNCTIONS

/// fonction pour la programmation du minuteur
#include <Wire.h> 
#include <ArduinoJson.h> // ArduinoJson : https://github.com/bblanchon/ArduinoJson

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// File System
#ifdef ESP32
  #include <FS.h>
  #include "SPIFFS.h"
  #include "config/enums.h"
#endif

#include "config/config.h"


extern DisplayValues gDisplayValues;
extern Config config; 

struct tm timeinfo;
epoc actual_time;

/// @brief ///////init du NTP 
void ntpinit() {
      // Configurer le serveur NTP et le fuseau horaire
  configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", NTP_SERVER);  //Voir Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
  getLocalTime( &timeinfo );
  Serial.println(asctime(&timeinfo));
  
}

//////// structure pour les programmateurs. 
struct Programme {
  public:
    char heure_demarrage[6]; // NOSONAR
    char heure_arret[6]; // NOSONAR
    int temperature=50;
    bool run; 
    int heure;
    int minute;

  private:char name_minuteur[12];  // NOSONAR
  
  // setter name 
  public:void Set_name(String setter) {strlcpy(name_minuteur, setter.c_str(), sizeof(name_minuteur)); }


  /// @brief sauvegarde
  /// @param programme_conf 
  public:void saveProgramme() {

        DynamicJsonDocument doc(192);

              ////vérification cohérence des données
        if (check_data(heure_demarrage)) {strcpy(heure_demarrage, "00:00"); }
        if (check_data(heure_arret)) {strcpy(heure_arret, "00:00"); }
      
        // Set the values in the document
        doc["heure_demarrage"] = heure_demarrage;
        doc["heure_arret"] = heure_arret;
        doc["temperature"] = temperature;
        
          // Open file for writing
        File configFile = SPIFFS.open(name_minuteur, "w");
        if (!configFile) {
          Serial.println(F("Failed to open config file for writing"));
          return;
        }

        // Serialize JSON to file
        if (serializeJson(doc, configFile) == 0) {
          Serial.println(F("Failed to write to file"));
        }
        
        configFile.close();
  }

  /// @brief chargement
  /// @param programme_conf 
  

  public:bool loadProgramme() {
        File configFile = SPIFFS.open(name_minuteur, "r");

        // Allocate a temporary JsonDocument
        // Don't forget to change the capacity to match your requirements.
        // Use arduinojson.org/v6/assistant to compute the capacity.
        DynamicJsonDocument doc(192);

        // Deserialize the JSON document
        DeserializationError error = deserializeJson(doc, configFile);
        if (error) {
          Serial.println(F("Failed to read minuterie config "));
          return false;
        }
      
        strlcpy(heure_demarrage,                  // <- destination
                doc["heure_demarrage"] | "", // <- source
                sizeof(heure_demarrage));         // <- destination's capacity
        
        strlcpy(heure_arret,                  // <- destination
                doc["heure_arret"] | "", // <- source
                sizeof(heure_arret));         // <- destination's capacity
        temperature = doc["temperature"] | 50 ; /// defaut à 50 °
        configFile.close();
      return true;    
  }

  void commande_run(){
          digitalWrite(COOLER, HIGH);
          run=true; 
          logging.Set_log_init("minuteur: start\r\n",true);
  }

   public:bool start_progr() {
      int heures;
      int minutes;
      sscanf(heure_demarrage, "%d:%d", &heures, &minutes);
      int heures_fin;
      int minutes_fin;
      sscanf(heure_arret, "%d:%d", &heures_fin, &minutes_fin);
      
      // si heure_demarrage == heure_arret alors on retourne false
      if (strcmp(heure_demarrage, heure_arret) == 0) {
            return false;
      }
      
      // quand c'est l'heure de démarrer le programme    

      ///vérification que le ntp est synchronisé
      if(getLocalTime( &timeinfo )) {
        if ( heures == timeinfo.tm_hour && minutes == timeinfo.tm_min && temperature > gDisplayValues.temperature ) {
            // demarrage du cooler
            commande_run();
            return true; 
        }
      }

      // remise en route en cas de reboot et si l'heure est dépassée  
      // recherche si l'heure est passée 
      bool heure_passee = false;
      if (timeinfo.tm_hour > heures || (timeinfo.tm_hour == heures && timeinfo.tm_min > minutes )) {
                          heure_passee = true; 
      }
      // recherche si l'heure d'arret est est passée
      bool heure_arret_passee = false;
      if (timeinfo.tm_hour > heures_fin || (timeinfo.tm_hour == heures_fin && timeinfo.tm_min >= minutes_fin )) {
                          heure_arret_passee = true; 
      }

        // remise en route en cas de reboot et si l'heure est dépassée
      if (heure_passee && !heure_arret_passee && temperature > gDisplayValues.temperature ) {
              commande_run();
                return true; 
      }

       

      // protection fuite mémoire 
      if (temperature > 500) {
        savelogs("-- reboot problème de fuite memoire -- ");
        ESP.restart(); 
      }

    return false; 
    }




/// @brief  stop du programme
/// @return 
public:bool stop_progr() {
  int heures ;
  int minutes;
  /// sécurité temp
  if ( gDisplayValues.temperature >= config.tmax  || gDisplayValues.temperature >= temperature ) { 
    digitalWrite(COOLER, LOW);
    logging.Set_log_init("minuteur: stop\r\n",true);
    run=false; 

     // protection flicking
    sscanf(heure_demarrage, "%d:%d", &heures, &minutes);  
    if (heures == timeinfo.tm_hour && minutes == timeinfo.tm_min) {
      delay(7000);
    }
  return true; 
  }
  
  sscanf(heure_arret, "%d:%d", &heures, &minutes);
  if(getLocalTime( &timeinfo )) {
    if (heures == timeinfo.tm_hour && minutes == timeinfo.tm_min ) {
        logging.Set_log_init("minuteur: stop\r\n",true);
        digitalWrite(COOLER, LOW);
        run=false; 
        return true; 
    }
  }
  return false; 
}

 /// vérification de la conformité de la donnée heure_demarrage[6]; 
 bool check_data(char data[6]){ // NOSONAR
  int heures; 
  int minutes;
  int result = sscanf(data, "%d:%d", &heures, &minutes);
  if (result != 2) {
    Serial.println("Erreur de lecture de l'heure");
    return true;
  }
  if (heures >= 0 && heures <= 23 && minutes >= 0 && minutes <= 59) {
      return false;
    }
  Serial.println("Erreur de lecture de l'heure");
  return true;
 }

};

//// fonction de reboot de l'ESP tous les lundi à  00h00  
void time_reboot() {
  if(getLocalTime( &timeinfo )) {
    if (timeinfo.tm_wday == 1 && timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec <= 15) {
      savelogs("-- Reboot du Lundi matin -- ");
      ESP.restart();
    }
  }
}


#endif
