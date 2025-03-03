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

int shelly_get_response;

int shelly_get_data(String url) {
 /// récupération en wget des informations du shelly 
HTTPClient shelly_http;
  int shelly_watt = 0;
  int port = 80;
  
  if (WiFi.status() == WL_CONNECTED) {   // si connecté on wget

  // tableau de pointeur d'url pour les différents shelly
  String baseurl[] = {"/emeter/0","/status","/rpc/EM.GetStatus?id=0","/rpc/EM1.GetStatus?id=0"};

     // serial_print("Shelly URL : ");
     // serial_println(baseurl[shelly_get_response]);

    shelly_http.begin(String(url),port,baseurl[shelly_get_response]); //HTTP  
    int httpResponseCode = shelly_http.GET();

   /// Serial.print("Shelly HTTP Response code: ");
   // Serial.println(httpResponseCode);

   if (httpResponseCode>=400) {
        Serial.println("Shelly HTTP Error");
        shelly_http.end();
        shelly_get_response ++;
        if (shelly_get_response > 3) {
          shelly_get_response = 0;
        }
        return shelly_watt;
    }

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
                return shelly_watt;
            }

            float powerValue = 0;
            /// récupération de la valeur de la puissance en fonction du mode triphasé ou non
            if (doc.containsKey("power"))  {  powerValue = doc["power"].as<float>();   }
            else if (doc.containsKey("total_power") ) { powerValue = doc["total_power"].as<float>(); }
            else if (config.Shelly_tri && doc.containsKey("total_act_power")) { powerValue = doc["total_act_power"].as<float>(); }
            else if (doc.containsKey("a_act_power")) { powerValue = doc["a_act_power"].as<float>(); }
            else if (doc.containsKey("act_power")) { powerValue = doc["act_power"].as<float>(); }

            else {
                shelly_watt = 99999;
                return shelly_watt;
            }
           
            // affichage dans le sérial de doc["total_power"] en tant que string ; 
            Serial.println("Shelly Watt : ");
            Serial.println(powerValue);

            return int(powerValue);
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