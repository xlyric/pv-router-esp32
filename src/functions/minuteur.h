#ifndef MINUTEUR_FUNCTIONS
#define MINUTEUR_FUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
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

//***********************************
//************* PROGRAMME PV ROUTER
//***********************************
#include "config/config.h"
#include "functions/unified_dimmer.h"

//***********************************
//************* Variables externes
//***********************************
extern DisplayValues gDisplayValues;
extern Config config; 
extern gestion_puissance unified_dimmer; 

//***********************************
//************* Variables locales
//***********************************
struct tm timeinfo;
epoc actual_time;

bool parseTimeFromShelly(String jsonString);

//***********************************
//************* structure pour les programmateurs
//***********************************
struct Programme {
  public:
    char heure_demarrage[6]; // NOSONAR
    char heure_arret[6]; // NOSONAR
    int temperature=50;
    bool run; 
    int heure;
    int minute;
    
    int seuil_start;
    int seuil_stop;
    int seuil_temperature;
    int puissance=100;

  private:
    bool security = false;
    String name;
  
  /// setter pour le nom du programme
  public:void set_name(String name) {
    this->name = name;
  }

  /// @brief sauvegarde
  /// @param programme_conf 
  public:void saveProgramme() {
    const char * c_file = name.c_str();// NOSONAR
    JsonDocument doc;
    Serial.println("save programme " + name);
    
    //vérification cohérence des données
    if (check_data(heure_demarrage)) {strcpy(heure_demarrage, "00:00"); }
    if (check_data(heure_arret)) {strcpy(heure_arret, "00:00"); }

    // Set the values in the document
    doc["heure_demarrage"] = heure_demarrage;
    doc["heure_arret"] = heure_arret;
    doc["temperature"] = temperature;
    doc["seuil_start"] = seuil_start;
    doc["seuil_stop"] = seuil_stop;
    doc["seuil_temperature"] = seuil_temperature;
    doc["puissance"] = puissance;
    
    // Open file for writing
    File configFile = SPIFFS.open(c_file, "w");
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
    const char * c_file = name.c_str();// NOSONAR
    File configFile = SPIFFS.open(c_file, "r");

    // Allocate a temporary JsonDocument
    // Don't forget to change the capacity to match your requirements.
    // Use arduinojson.org/v6/assistant to compute the capacity.
    JsonDocument doc;

    // Deserialize the JSON document
    DeserializationError error = deserializeJson(doc, configFile);
    if (error) {
      Serial.println("Failed to read minuterie config " + name);
      return false;
    }
      
    strlcpy(heure_demarrage,                  // <- destination
      doc["heure_demarrage"] | "00:00", // <- source
      sizeof(heure_demarrage));         // <- destination's capacity
    strlcpy(heure_arret,                  // <- destination
      doc["heure_arret"] | "00:00", // <- source
      sizeof(heure_arret));         // <- destination's capacity
    temperature = doc["temperature"] | 50 ; /// defaut à 50 °
    seuil_start = doc["seuil_start"] | 0 ; /// defaut à 0 %°
    seuil_stop = doc["seuil_stop"] | 0 ; /// defaut à sans arret %
    seuil_temperature = doc["seuil_temperature"] | 0 ; /// defaut à 0 °
    puissance = doc["puissance"] | 100 ; /// defaut à 100 %
    
    configFile.close();
    return true;    
  } // public:bool loadProgramme()

  void commande_run(){
    digitalWrite(COOLER, HIGH);
    run=true; 
    logging.Set_log_init(Start_minuteur,true);
  }

  public:bool start_progr() {
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(timeinfo));

    if (!getLocalTime(&timeinfo)) {
        Serial.println("Échec récupération heure NTP");
        return false;
    }

    int heures_debut, minutes_debut;
    int heures_fin, minutes_fin;
    sscanf(heure_demarrage, "%d:%d", &heures_debut, &minutes_debut);
    sscanf(heure_arret, "%d:%d", &heures_fin, &minutes_fin);
      
    // si heure_demarrage == heure_arret alors on retourne false
    if (strcmp(heure_demarrage, heure_arret) == 0) {
      return false;
    }
      
    // Conversion en minutes depuis minuit pour faciliter les comparaisons
    int now_minutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    int debut_minutes = heures_debut * 60 + minutes_debut;
    int fin_minutes = heures_fin * 60 + minutes_fin;
      
    // Démarrage exact à l'heure programmée
    if (heures_debut == timeinfo.tm_hour && minutes_debut == timeinfo.tm_min && 
        temperature > gDisplayValues.temperature) {
       commande_run();
       return true; 
    }

    // remise en route en cas de reboot et si l'heure est dépassée  
    // recherche si l'heure est passée 
    bool dans_plage = false;
    if (fin_minutes > debut_minutes) {
        // Plage normale (ex: 08:00 -> 18:00)
        dans_plage = (now_minutes > debut_minutes && now_minutes < fin_minutes);
    } else {
        // Plage qui traverse minuit (ex: 22:00 -> 02:00)
        // On est dans la plage si on est après le début OU avant la fin
        dans_plage = (now_minutes > debut_minutes || now_minutes < fin_minutes);
    }

// Remise en route en cas de reboot si on est dans la plage horaire
    if (dans_plage && temperature > gDisplayValues.temperature) {
      commande_run();
      return true; 
    }

    // Protection fuite mémoire 
    if (temperature > 500) {
      savelogs("-- reboot problème de fuite memoire -- ");
      ESP.restart(); 
    }
    return false; 
  } // public:bool start_progr()

  /// @brief  stop du programme
  /// @return 
  public:bool stop_progr() {
    struct tm timeinfo;
    memset(&timeinfo, 0, sizeof(timeinfo));
    
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Échec récupération heure NTP");
        return false;
    }

    int heures ;
    int minutes;
    /// sécurité temp
    if ( gDisplayValues.temperature >= config.tmax  || gDisplayValues.temperature >= temperature ) { 
      digitalWrite(COOLER, LOW);
      logging.Set_log_init(Stop_minuteur_temp,true);
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
          logging.Set_log_init(Stop_minuteur,true);
          digitalWrite(COOLER, LOW);
          run=false; 
          return true; 
      }
    }
    return false; 
  } // public:bool stop_progr()

  // vérification de la conformité de la donnée heure_demarrage[6]; 
  bool check_data(char data[6]) { // NOSONAR
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

  // démarrage si le seuil est atteint 
  bool start_seuil() {
    if ( unified_dimmer.get_power() >= seuil_start && gDisplayValues.temperature< seuil_temperature && seuil_start != seuil_stop) { 
      return true;
    }
    return false; 
  }

  /// arrêt si le seuil est atteint
  bool stop_seuil() {
    if ( unified_dimmer.get_power() >= seuil_stop && seuil_start != seuil_stop && gDisplayValues.temperature> seuil_temperature) { 
      return true;
    }
    return false;
  }

  /// arret si seuil temp est atteint
  bool stop_seuil_temp() {
    if ( gDisplayValues.temperature>= seuil_temperature && seuil_temperature != 0) { 
      return true;
    }
    return false;
  }
}; // Struct Programme

//***********************************
//************* time_reboot()
//************* fonction de reboot de l'ESP tous les lundi à  00h00  
//***********************************
void time_reboot() {
  if(getLocalTime( &timeinfo )) {
    if (timeinfo.tm_wday == 1 && timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec <= 15) {
      savelogs("-- Reboot du Lundi matin -- ");
      ESP.restart();
    }
  }
}

//***********************************
//************* ntpinit()
//***********************************
void ntpinit() {
  // Configurer le serveur NTP et le fuseau horaire
  //Voir Time-Zone: https://sites.google.com/a/usapiens.com/opnode/time-zones
  configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", NTP_SERVER);  
  getLocalTime( &timeinfo );
  Serial.println(asctime(&timeinfo));  
}


// Fonction adaptée pour récupérer l'heure du Shelly
bool ntpinit_Shelly() {
  // Récupération de l'heure sur le Shelly à l'adresse 192.168.33.1 dans le Json 
  WiFiClient client;
  
  Serial.println("Connexion au Shelly...");
  
  if (client.connect("192.168.33.1", 80)) {
    Serial.println("Connecté au Shelly");
    
    // Envoi de la requête HTTP
    client.print("GET /status HTTP/1.1\r\n");
    client.print("Host: 192.168.33.1\r\n");
    client.print("Connection: close\r\n\r\n");
    
    // sur les version pro, c'est ici : /rpc/Shelly.GetStatus --> prendre unixtime 
    // Attendre la réponse
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println("Timeout de connexion");
        client.stop();
        return false;
      }
    }
    
    // Lire la réponse HTTP
    String response = "";
    bool jsonStarted = false;
    
    while (client.available()) {
      String line = client.readStringUntil('\n');
      
      // Chercher le début du JSON (après les headers HTTP)
      if (line.startsWith("{")) {
        jsonStarted = true;
      }
      
      if (jsonStarted) {
        response += line;
      }
    }
    
    client.stop();
    
    // Parser le JSON pour extraire l'heure
    if (response.length() > 0) {
      return parseTimeFromShelly(response);
    } else {
      Serial.println("Aucune réponse JSON reçue");
      return false;
    }
    
  } else {
    Serial.println("Échec de connexion au Shelly");
    return false;
  }
}

bool parseTimeFromShelly(String jsonString) {
  // Créer un document JSON
  JsonDocument doc;
  
  // Parser le JSON
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) {
    Serial.print("Erreur de parsing JSON: ");
    Serial.println(error.c_str());
    return false;
  }
  
  // Extraire le timestamp Unix
  if (!doc["unixtime"].isNull()) {
    unsigned long unixTime = doc["unixtime"].as<unsigned long>();
    
    // Convertir le timestamp Unix en structure tm
    time_t rawTime = (time_t)unixTime;
    
    // Appliquer le fuseau horaire CET/CEST manuellement
    // Le Shelly donne probablement l'heure locale, mais on s'assure de la cohérence
    struct tm* tempTimeinfo = gmtime(&rawTime);
    
    // Copier dans notre structure globale
    timeinfo = *tempTimeinfo;
    
    // *** ÉTAPE CRUCIALE : Mettre à jour l'horloge système ESP32 ***
    struct timeval tv;
    tv.tv_sec = rawTime;
    tv.tv_usec = 0;
    
    if (settimeofday(&tv, NULL) == 0) {
      Serial.println("Horloge système ESP32 mise à jour avec succès");
      
      // Configurer le fuseau horaire pour que localtime() fonctionne correctement
      //setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
      //tzset();
      
      // Maintenant mettre à jour timeinfo avec l'heure locale
      struct tm* tempTimeinfo = localtime(&rawTime);
      timeinfo = *tempTimeinfo;
      
      Serial.print("Heure récupérée du Shelly: ");
      Serial.println(asctime(&timeinfo));
      
      // Vérification : obtenir l'heure système
      time_t verification;
      time(&verification);
      Serial.print("Vérification horloge système: ");
      Serial.println(ctime(&verification));
      
    } else {
      Serial.println("ERREUR: Impossible de mettre à jour l'horloge système");
      return false;
    }
      // Afficher aussi l'heure au format HH:MM du JSON pour vérification
    if (!doc["time"].isNull()) {
      String shellyTime = doc["time"].as<String>();
      Serial.print("Heure Shelly (format HH:MM): ");
      Serial.println(shellyTime);
    }
    
    return true;
  }
  Serial.println("Pas de timestamp Unix trouvé dans le JSON");
  return false;
}

#endif // MINUTEUR_FUNCTIONS