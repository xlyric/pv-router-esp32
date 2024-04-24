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

//// NTP 
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, NTP_SERVER, NTP_OFFSET_SECONDS, NTP_UPDATE_INTERVAL_MS);

void offset_heure_ete();
void timeclientEpoch_to_date(time_t epoch) ;

struct tm timeinfo;
epoc actual_time;

/// @brief ///////init du NTP 
void ntpinit() {
      // Configurer le serveur NTP et le fuseau horaire
  // timeClient.begin();
  // timeClient.update();
  //Serial.println(timeClient.getFormattedTime());
  // offset_heure_ete();
  configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", NTP_SERVER);  //Voir Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
  // Serial.println(timeClient.getFormattedTime());
  getLocalTime( &timeinfo );
  Serial.println(asctime(&timeinfo));
  
}

void timeclientEpoch_to_date(time_t epoch)  { // convert epoch to date  
  actual_time.mois = month(epoch);
  actual_time.jour = day(epoch);
  actual_time.heure = hour(epoch);
  DEBUG_PRINTLN(actual_time.mois);
  DEBUG_PRINTLN(actual_time.jour);
  DEBUG_PRINTLN(actual_time.heure);
  }

void offset_heure_ete() {
  timeclientEpoch_to_date(timeClient.getEpochTime());
  //timeClient.setTimeOffset(7200);
  if ((actual_time.jour >= 25 && actual_time.mois >= 3 && actual_time.heure >= 2)||(actual_time.mois >= 4)){
    timeClient.setTimeOffset(7200); // Fuseau horaire (en secondes, ici GMT+2)
  }
  if ((actual_time.jour >= 25 && actual_time.mois >= 10 && actual_time.heure >= 3)||(actual_time.mois >= 11)) {
    timeClient.setTimeOffset(3600); // Fuseau horaire (en secondes, ici GMT+1)
  }
}

//////// structure pour les programmateurs. 
struct Programme {
  public:char heure_demarrage[6];
  public:char heure_arret[6];
  public:int temperature=50;
  public:bool run; 
  public:int heure;
  public:int minute;
  //public:String name2;
  private:char name_minuteur[12]; 
  
  // setter name 
  public:void Set_name(String setter) {strlcpy(name_minuteur, setter.c_str(), sizeof(name_minuteur)); }


  /// @brief sauvegarde
  /// @param programme_conf 
  public:void saveProgramme() {
        //const char * c_file = name;
        //Serial.println(c_file);
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
        //const char * c_file = name;
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
          timeClient.update();
          logging.Set_log_init("minuteur: start\r\n");
  }

   public:bool start_progr() {
      int heures, minutes;
      sscanf(heure_demarrage, "%d:%d", &heures, &minutes);
      int heures_fin, minutes_fin;
      sscanf(heure_arret, "%d:%d", &heures_fin, &minutes_fin);
      
      // si heure_demarrage == heure_arret alors on retourne false
      if (strcmp(heure_demarrage, heure_arret) == 0) {
            return false;
      }
      
      // quand c'est l'heure de démarrer le programme    

      ///vérification que le ntp est synchronisé
      if(timeClient.isTimeSet()) {
        if ( heures == timeClient.getHours() && minutes == timeClient.getMinutes() && temperature > gDisplayValues.temperature ) {
            // demarrage du cooler
            commande_run();
            return true; 
        }
      }

      // remise en route en cas de reboot et si l'heure est dépassée  
      // recherche si l'heure est passée 
      bool heure_passee = false;
      if (timeClient.getHours() > heures || (timeClient.getHours() == heures && timeClient.getMinutes() > minutes )) {
                          heure_passee = true; 
      }
      // recherche si l'heure d'arret est est passée
      bool heure_arret_passee = false;
      if (timeClient.getHours() > heures_fin || (timeClient.getHours() == heures_fin && timeClient.getMinutes() >= minutes_fin )) {
                          heure_arret_passee = true; 
      }

        // remise en route en cas de reboot et si l'heure est dépassée
      if (heure_passee && !heure_arret_passee && temperature > gDisplayValues.temperature ) {
              commande_run();
                return true; 
      }
        /// ??? doublon ?
    //    if(timeClient.isTimeSet()) {
    //      if (heures < timeClient.getHours() && minutes < timeClient.getMinutes()) {
    //        commande_run();
    //        return true; 
     //     }
     //   }
       

      // protection fuite mémoire 
      if (temperature > 500) {
        savelogs(timeClient.getFormattedTime() +"-- reboot problème de fuite memoire -- ");
        ESP.restart(); 
      }

    return false; 
    }




/// @brief  stop du programme
/// @return 
public:bool stop_progr() {
  int heures, minutes;
  /// sécurité temp
  if ( gDisplayValues.temperature >= config.tmax  || gDisplayValues.temperature >= temperature ) { 
    digitalWrite(COOLER, LOW);
    logging.Set_log_init("minuteur: stop\r\n");
    run=false; 

     // protection flicking
    sscanf(heure_demarrage, "%d:%d", &heures, &minutes);  
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
      delay(7000);
    }
  return true; 
  }
  
  sscanf(heure_arret, "%d:%d", &heures, &minutes);
  if(timeClient.isTimeSet()) {
    if (heures == timeClient.getHours() && minutes == timeClient.getMinutes()) {
        logging.Set_log_init("minuteur: stop\r\n");
        digitalWrite(COOLER, LOW);
        run=false; 
        timeClient.update();
        offset_heure_ete();     
        return true; 
    }
  }
  return false; 
}

 /// vérification de la conformité de la donnée heure_demarrage[6]; 
 bool check_data(char data[6]){
  int heures, minutes;
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
  if(timeClient.isTimeSet()) {
    if (timeClient.getDay() == 1 && timeClient.getHours() == 0 && timeClient.getMinutes() == 0 && timeClient.getSeconds() <= 15) {
      savelogs(timeClient.getFormattedTime() +"-- Reboot du Lundi matin -- ");
      ESP.restart();
    }
  }
}


#endif
