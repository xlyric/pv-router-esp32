#ifndef ENPHASE_FUNCTIONS
#define ENPHASE_FUNCTIONS

#include <Arduino.h>
#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"

#include <FS.h>
#include "SPIFFS.h"
#include "config/enums.h"

HTTPClient httpenphase;  

const char *enphase_conf = "/enphase.json";
extern Configmodule configmodule; 

void enphase_get(void);

bool loadenphase(const char *filename, Configmodule &configmodule) {
  // Open file for reading
  File configFile = SPIFFS.open(enphase_conf, "r");
  if (!configFile) {
    Serial.println(F("No Enphase used"));
    
    return false;
    }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read Enphase config"));
    
    return false;
  }

  
  // Copy values from the JsonDocument to the Config
  
  strlcpy(configmodule.hostname,                  // <- destination
          doc["IP_ENPHASE"] | "192.168.0.200", // <- source
          sizeof(configmodule.hostname));         // <- destination's capacity
  
  configFile.close();


Serial.println(" enphase config : " + String(configmodule.hostname));
return true;
}



void Enphase_get(void) {

String url = "/api/v1/production.json";

httpenphase.begin(String(configmodule.hostname),80,url);
int httpResponseCode = httpenphase.GET();
// start connection and send HTTP header

Serial.print(" httpcode/ fonction mesure: ");
Serial.println(httpResponseCode);

#if(httpResponseCode == HTTP_CODE_OK)

    String payload = httpenphase.getString();
    DynamicJsonDocument doc(900);
    DeserializationError error = deserializeJson(doc, payload);

    long generatedPower = doc["wattsNow"];
    gDisplayValues.Fronius_prod = generatedPower;
    long Powertoday = doc["wattHoursToday"];
    gDisplayValues.Fronius_conso = Powertoday; 
#endif

httpenphase.end();

Serial.print("E prod: " + String(gDisplayValues.Fronius_prod));
Serial.print("E daily: "+ String(gDisplayValues.Fronius_conso) );


}




#endif