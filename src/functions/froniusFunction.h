#ifndef FRONIUS_FUNCTIONS
#define FRONIUS_FUNCTIONS

#include <Arduino.h>
#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"

#include <FS.h>
#include "SPIFFS.h"
#include "config/enums.h"
#include "HTTPClient.h"

HTTPClient httpfronius;  

const char *fronius_conf = "/fronius.json";
extern Configmodule configmodule; 
extern Logs logging;

void Fronius_get(void);

bool loadfronius(const char *filename, Configmodule &configmodule) {
  // Open file for reading
  File configFile = SPIFFS.open(fronius_conf, "r");
  if (!configFile) {
    Serial.println(F("No Fronius used"));
    logging.init += "--> No Fronius used\r\n";
      return false;
    }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<512> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to read Fronius config"));
    logging.init += "--> Fronius not present\r\n";
    return false;
  }

  
  // Copy values from the JsonDocument to the Config
  
  strlcpy(configmodule.hostname,                  // <- destination
          doc["IP_FRONIUS"] | "", // <- source
          sizeof(configmodule.hostname));         // <- destination's capacity
  
  configFile.close();

Serial.println(" Fronius config : " + String(configmodule.hostname));
logging.init += "Fronius used\r\n";
return true;
}



void Fronius_get(void) {

String url = "/solar_api/v1/GetInverterRealtimeData.cgi?Scope=System";
String url2 = "/solar_api/v1/GetMeterRealtimeData.cgi?Scope=System";
httpfronius.begin(String(configmodule.hostname),80,url);
int httpResponseCode = httpfronius.GET();
// start connection and send HTTP header

Serial.print(" httpcode/ fonction mesure: ");
Serial.println(httpResponseCode);

#if(httpResponseCode == HTTP_CODE_OK)

    String payload = httpfronius.getString();
    DynamicJsonDocument doc(900);
    DeserializationError error = deserializeJson(doc, payload);

    int generatedPower = int(doc["Body"]["Data"]["PAC"]["Values"]["1"]);
    gDisplayValues.Fronius_prod = generatedPower;

#endif

httpfronius.end();

HTTPClient http2;
httpfronius.begin(String(configmodule.hostname),80,url2);
httpResponseCode = httpfronius.GET();
#if(httpResponseCode == HTTP_CODE_OK)


    payload = http2.getString();
    DynamicJsonDocument doc2(2048);
    error = deserializeJson(doc2, payload);
    int generatedPower2 = int(doc2["Body"]["Data"]["0"]["PowerReal_P_Sum"]);
    gDisplayValues.Fronius_conso = generatedPower2;

#endif

httpfronius.end();

//debug
//Serial.print("prod : " + String(gDisplayValues.Fronius_prod));
//Serial.print("conso: : "+ String(gDisplayValues.Fronius_conso) );


}




#endif