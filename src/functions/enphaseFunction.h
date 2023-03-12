#ifndef ENPHASE_FUNCTIONS
#define ENPHASE_FUNCTIONS

#include <Arduino.h>
#include <FS.h>
#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"
#include "SPIFFS.h"
#include "config/enums.h"

HTTPClient httpenphase;

const char *enphase_conf = "/enphase.json";
extern Configmodule configmodule;



bool loadenphase(const char *filename, Configmodule &configmodule) {
  // Open file for reading
  File configFile = SPIFFS.open(enphase_conf, "r");
  if (!configFile) {
    Serial.println(NOT_ENPHASE);
    logging.init += loguptime();
    logging.init += NOT_ENPHASE;

    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  DynamicJsonDocument doc(512);

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(NOT_ENPHASE);
    logging.init += loguptime();
    logging.init += NOT_ENPHASE;

    return false;
  }

  // Copy values from the JsonDocument to the Config

  strlcpy(configmodule.hostname,                // <- destination
          doc["IP_ENPHASE"] | "192.168.0.200",  // <- source
          sizeof(configmodule.hostname));       // <- destination's capacity
  strlcpy(configmodule.port, doc["PORT_ENPHASE"] | "80",
          sizeof(configmodule.port));   // <- destination's capacity
  strlcpy(configmodule.envoy,           // <- destination
          doc["Type"] | "S",            // <- source
          sizeof(configmodule.envoy));  // <- destination's capacity
  configmodule.pilote = doc["Pilote"] | true;

  configFile.close();

  Serial.println(" enphase config : " + String(configmodule.hostname));
  Serial.println(" enphase mode : " + String(configmodule.envoy));
  logging.init += loguptime();
  logging.init += "Enphase used type "+ String(configmodule.envoy) +"\r\n";
  return true;
}

void Enphase_get(void) {
  String url = "/404.html";
  Serial.println("Enphase_get : envoy type=" + String(configmodule.envoy));
  if (String(configmodule.envoy) == "R") {
    url = String(EnvoyR);
    Serial.print("type R");
    Serial.print(url);
  }
  if (String(configmodule.envoy) == "S") {
    url = String(EnvoyS);
    Serial.print("type S");
    Serial.print(url);
  }

  httpenphase.begin(String(configmodule.hostname), atoi(configmodule.port), url);
  // int httpResponseCode = httpenphase.GET();

  /// workaround because envoy is too slow
  int httpResponseCode;
  while ((httpResponseCode = httpenphase.GET()) != 200)
    ;

  // start connection and send HTTP header

  Serial.print(" httpcode/ fonction mesure: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == HTTP_CODE_OK) {
    String payload = httpenphase.getString();

    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);

    if (String(configmodule.envoy) == "R") {
      gDisplayValues.Fronius_prod = int(doc["wattsNow"]);
      gDisplayValues.Fronius_conso = int(doc["wattHoursToday"]);
    } else {
      gDisplayValues.Fronius_prod = int(doc["production"][1]["wNow"]);
      gDisplayValues.Fronius_conso = int(doc["consumption"][1]["wNow"]);
      gDisplayValues.Fronius_totalconso = int(doc["consumption"][0]["wNow"]);
    }

    String test = doc["consumption"][0];
  } else {
    Serial.println("timeout");
  }

  httpenphase.end();

  // debug
  Serial.print("prod: " + String(gDisplayValues.Fronius_prod));
  Serial.print(" conso: " + String(gDisplayValues.Fronius_conso));
  Serial.println(" total conso: " + String(gDisplayValues.Fronius_totalconso));
}

#endif
