#ifndef SHELLY
#define SHELLY
/// fonction pour la récupération en Wget des données du Shelly

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
#include "HTTPClient.h"

extern Logs logging; 
extern Config config;
extern DisplayValues gDisplayValues;


int shelly_get_data(String url) {
 /// récupération en wget des informations du shelly 
HTTPClient shelly_http;
  int shelly_watt = 0;
  int port = 80;
  
  if (WiFi.status() == WL_CONNECTED) {   // si connecté on wget

    String baseurl = "/emeter/0" ; 
        /// mode triphasé
      if ( config.Shelly_tri ) {
        if (!config.Shelly_mode) {
          baseurl = "/rpc/EM.GetStatus?id=0" ; // pour le 3EM
        }
        if (config.Shelly_mode) {
          baseurl = "/status" ; // pour le 3EM type 2 
        }
      }

   // détection de si l'url à un :port
   // if (url.indexOf(":") > 0) {
  //      port = url.substring(url.indexOf(":")+1).toInt();
  //      url = url.substring(0,url.indexOf(":"));
  //  }
    
  //  Serial.println("info shelly");
  //  Serial.println(url);
  //  Serial.println(port);
//Serial.println(baseurl);

    shelly_http.begin(String(url),port,baseurl);   
    int httpResponseCode = shelly_http.GET();

   if (httpResponseCode==404) {
        config.Shelly_mode = true;
        return shelly_watt;
    }
       
       //Serial.println(httpResponseCode);

        String shelly_state = "0"; 
        shelly_state = shelly_http.getString();
        shelly_http.end();
        if (httpResponseCode==200) {
            shelly_watt = 99999;
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, shelly_state);

            /// protection de la validité du json
            if (error) {
                Serial.print(F("deserializeJson() failed: "));
                logging.Set_log_init("deserializeJson() failed: ",true);
                Serial.println(error.c_str());
                shelly_watt = 99999;
                return shelly_watt;
            }

            auto powerValue = doc["power"];
              /// mode triphasé
              if (config.Shelly_tri ) {
                if (!config.Shelly_mode) { 
                  powerValue = doc["total_act_power"];
                }
                if (config.Shelly_mode) { 
                  powerValue = doc["total_power"];
                }
                if ( powerValue.isNull() || doc["total_power"].as<String>() == "null") {
                 config.Shelly_mode = true;
                return shelly_watt;
                }
              }
            
            // affichage dans le sérial de doc["total_power"] en tant que string ; 
            Serial.println("Shelly Watt : ");
            Serial.println(doc["total_power"].as<String>());
            Serial.println(baseurl);

             /// protection de la donnée
            if (powerValue.is<int>() || powerValue.is<float>()) {
                shelly_watt = powerValue.as<int>();
            }
            else {
                shelly_watt = 99999;
            }

        }
        else {
            shelly_watt = 99999;
        }
    } 

return shelly_watt; 

}

bool checkIP(String inputString) {
  IPAddress ip;
  
  // Essayez de convertir la chaîne en adresse IP
  if (ip.fromString(inputString)) {
    return true;
  } else {
    return false;
  }
}


#endif