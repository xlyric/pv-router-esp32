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
bool enphase_present = false;
int enphase_prod = 0;
int enphase_conso = 0 ; 
char *IP_ENPHASE; 

void enphase_get(void);

bool loadenphase(const char *filename) {
  // Open file for reading
  File configFile = SPIFFS.open(filename, "r");
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
    enphase_present = false;
    return false;
  }

  
  // Copy values from the JsonDocument to the Config
  
  strlcpy(IP_ENPHASE,                  // <- destination
          doc["IP_ENPHASE"] | "", // <- source
          sizeof(IP_ENPHASE));         // <- destination's capacity
  
  configFile.close();

enphase_present = true;
Serial.println(" enphase config : " + String(IP_ENPHASE));
return true;
}



void Enphase_get(void) {

String url = "/api/v1/production";

httpenphase.begin(String(IP_ENPHASE),80,url);
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

Serial.print("gDisplayValues.production / fonction mesure: " + String(gDisplayValues.Fronius_prod));
Serial.print("gDisplayValues.watt / fonction mesure: " + String(gDisplayValues.watt));
Serial.print("gDisplayValues.enphaseup / fonction mesure: : "+ String(gDisplayValues.Fronius_conso) );


}




#endif