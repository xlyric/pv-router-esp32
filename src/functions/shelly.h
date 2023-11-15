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
HTTPClient http_shelly;
  int watt_shelly = 0;
  
  if (WiFi.status() == WL_CONNECTED) {   // si connecté on wget
//    const String baseurl = "/status" ; 
    const String baseurl = "/emeter/0" ; 
    http_shelly.begin(String(url),80,baseurl);   
        
        int httpResponseCode = http_shelly.GET();
        String shellystate = "0"; 
        shellystate = http_shelly.getString();
        http_shelly.end();
        if (httpResponseCode==200) {
            watt_shelly = 0;
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, shellystate);
            auto powerValue = doc["power"];
            /// protection de la donnée
            if (powerValue.is<int>() || powerValue.is<float>()) {
                watt_shelly = powerValue.as<int>();
            }
            else {
                watt_shelly = 0;
            }
           
            //Serial.println("shelly : " + String(watt_shelly));
        }
        else {
            watt_shelly = 0;
        }
    } 

return watt_shelly; 

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