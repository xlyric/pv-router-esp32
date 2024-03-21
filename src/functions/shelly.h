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
  
  if (WiFi.status() == WL_CONNECTED) {   // si connecté on wget
//    const String baseurl = "/status" ; 
    String baseurl = "/emeter/0" ; 
        /// mode triphasé
      if ( config.Shelly_tri) {
        String baseurl = "/rpc/EM.GetStatus?id=0" ; 
      }
    //const String baseurl = "/rpc/EM.GetStatus?id=0" ; 
    shelly_http.begin(String(url),80,baseurl);   
        
        int httpResponseCode = shelly_http.GET();
        String shelly_state = "0"; 
        shelly_state = shelly_http.getString();
        shelly_http.end();
        if (httpResponseCode==200) {
            shelly_watt = 99999;
            DynamicJsonDocument doc(1024);
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
                auto powerValue = doc["total_act_power"];
              }

            
            //auto powerValue = doc["total_act_power"];
            /// protection de la donnée
            if (powerValue.is<int>() || powerValue.is<float>()) {
                shelly_watt = powerValue.as<int>();
            }
            else {
                shelly_watt = 99999;
            }
           
            //Serial.println("shelly : " + String(shelly_watt));
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