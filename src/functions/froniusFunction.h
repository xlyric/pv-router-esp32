#ifndef FRONIUS_FUNCTIONS
#define FRONIUS_FUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#include <FS.h>
#include "SPIFFS.h"
#include "HTTPClient.h"

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/config.h"
#include "../config/enums.h"
#ifndef LIGHT_FIRMWARE
  #include "../functions/Mqtt_http_Functions.h"
#endif
#include "config/enums.h"

//***********************************
//************* Variables externes
//***********************************
extern Configmodule configmodule; 
extern Logs logging;

//***********************************
//************* Variables locales
//***********************************
HTTPClient httpfronius;  
constexpr const char *fronius_conf = "/fronius.json";

//***********************************
//************* Déclaration de fonctions
//***********************************
void Fronius_get(void);

//***********************************
//************* loadfronius
//***********************************
bool loadfronius(const char *filename) {
  // Open file for reading
  File configFile = SPIFFS.open(fronius_conf, "r");
  if (!configFile) {
    Serial.println(NOT_FRONIUS);  
    logging.Set_log_init(NOT_FRONIUS,true);

    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(NOT_FRONIUS);    
    logging.Set_log_init(NOT_FRONIUS,true);

    return false;
  }
 
  // Copy values from the JsonDocument to the Config

  strlcpy(configmodule.hostname,                  // <- destination
          doc["IP_FRONIUS"] | "", // <- source
          sizeof(configmodule.hostname));         // <- destination's capacity
  
  configFile.close();
  
  Serial.println(" Fronius config : " + String(configmodule.hostname));
  logging.Set_log_init("Fronius used\r\n",true);
  
  return true;
}

//***********************************
//************* Fronius_get
//***********************************
void Fronius_get(void) {
  const String url = "/solar_api/v1/GetInverterRealtimeData.cgi?Scope=System";
  const String url2 = "/solar_api/v1/GetMeterRealtimeData.cgi?Scope=System";
  httpfronius.begin(String(configmodule.hostname),80,url);
  int httpResponseCode = httpfronius.GET();

  // start connection and send HTTP header
  Serial.print(" httpcode/ fonction mesure: ");
  Serial.println(httpResponseCode);

  #if (httpResponseCode == HTTP_CODE_OK)
      String payload = httpfronius.getString();
      httpfronius.end();
      JsonDocument doc;

      DeserializationError error = deserializeJson(doc, payload);
      if (error) {
          Serial.print(F("Fronius_get() failed: "));
          logging.Set_log_init("Fronius_get() failed: ",true);
          Serial.println(error.c_str());
          return;
      }
      
      int generatedPower = int(doc["Body"]["Data"]["PAC"]["Values"]["1"]);
      gDisplayValues.Fronius_prod = generatedPower;
  #endif

  httpfronius.end();
  
  httpfronius.begin(String(configmodule.hostname),80,url2);
  httpResponseCode = httpfronius.GET();
  #if(httpResponseCode == HTTP_CODE_OK)
      payload = httpfronius.getString();
      httpfronius.end();
      JsonDocument doc2;
      error = deserializeJson(doc2, payload);
      if (error) {
        Serial.print(F("Fronius_get() failed: "));
        logging.Set_log_init("Fronius_get() failed: ",true);
        Serial.println(error.c_str());

        return;
      }

      int generatedPower2 = int(doc2["Body"]["Data"]["0"]["PowerReal_P_Sum"]);
      gDisplayValues.Fronius_conso = generatedPower2;
  #endif

  httpfronius.end();
}

#endif
