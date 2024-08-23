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

int shelly_read_meter(String url, String meter_url, HTTPClient& shelly_http, bool& status) {
  /// récupération en wget des informations d'un compteur shelly
  status = false;
  int shelly_watt = 99999;

  shelly_http.begin(String(url),80,meter_url);    
  int httpResponseCode = shelly_http.GET();
  String shelly_state = "0"; 
  shelly_state = shelly_http.getString();
  shelly_http.end();

  if (httpResponseCode==200) {
      
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, shelly_state);

      /// protection de la validité du json
      if (error) {
          Serial.print(F("deserializeJson() failed: "));
          logging.Set_log_init("deserializeJson() failed: ",true);
          Serial.println(error.c_str());
      }
      else {
        auto powerValue = doc["power"];

        /// protection de la donnée
        if (powerValue.is<int>() || powerValue.is<float>()) {
            shelly_watt = powerValue.as<int>();
            status = true;
        }
      }
    }
  return shelly_watt;
}


int shelly_get_data(String url) {
 /// récupération en wget des informations du shelly
  HTTPClient shelly_http;
  int shelly_watt = 0;
  bool status = true;
  
  if (WiFi.status() == WL_CONNECTED) {   // si connecté on wget

    String baseurl = "/emeter/0" ; 
    shelly_watt = shelly_read_meter(url, baseurl, shelly_http, status);

    if ( status && config.Shelly_tri ) {
        shelly_watt += shelly_read_meter(url, "/emeter/1", shelly_http,status);
        shelly_watt += status ? shelly_read_meter(url, "/emeter/2", shelly_http, status) : 0;
      }
    } 
  return status ? shelly_watt : 99999; 
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