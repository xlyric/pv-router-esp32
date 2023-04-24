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

void Enphase_get_5(void);
void Enphase_get_7(void);

//////////////////// gestion FS


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
  DynamicJsonDocument doc(768);

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
          doc["IP_ENPHASE"] | "",  // <- source
          sizeof(configmodule.hostname));       // <- destination's capacity
  strlcpy(configmodule.port, doc["PORT_ENPHASE"] | "80",
          sizeof(configmodule.port));   // <- destination's capacity
  strlcpy(configmodule.envoy,           // <- destination
          doc["Type"] | "S",            // <- source
          sizeof(configmodule.envoy));  // <- destination's capacity
  //configmodule.pilote = doc["Pilote"] | false;
  strlcpy(configmodule.version,           // <- destination
          doc["version"] | "5",            // <- source
          sizeof(configmodule.version));
  strlcpy(configmodule.token,         
          doc["token"] | "",            
          sizeof(configmodule.token));
  //configmodule.enphase_present = doc["enphase_present"] | false;
  if (strcmp(configmodule.hostname,"") == 0) { configmodule.enphase_present=false ; configFile.close(); Serial.println("no enphase"); return false; } 
  Serial.println(configmodule.hostname);
  configFile.close();
  Serial.println(" enphase config : " + String(configmodule.hostname));
  Serial.println(" enphase mode : " + String(configmodule.envoy));
  logging.init += loguptime();
  logging.init += "Enphase used type "+ String(configmodule.envoy) +"\r\n";
  configmodule.enphase_present=true; 
  return true;

}

void saveenphase(const char *filename, const Configmodule &configmodule) {
  
  // Open file for writing
   File configFile = SPIFFS.open(enphase_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open enphase file "));
    logging.init += loguptime();
    logging.init += "Failed to open enphase file \r\n";
    return;
  } 

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  DynamicJsonDocument doc(768);

  // Set the values in the document
  doc["IP_ENPHASE"] = configmodule.hostname;
  doc["PORT_ENPHASE"] = configmodule.port;
  doc["Type"] = configmodule.envoy;
  doc["version"] = configmodule.version;
  doc["token"] = configmodule.token;
  //doc["enphase_present"] = configmodule.enphase_present;

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file enphase "));
    
  }
  // Close the file
  configFile.close();
}


//////////////////////////////////// récupération des valeurs 


void Enphase_get(void) {
  if (String(configmodule.version) == "7") {
    //Serial.println("Enphase version 7");
    Enphase_get_7();
  } else {
    //Serial.println("Enphase version 5");
    Enphase_get_5();
  }
}

void Enphase_get_5(void) {
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

  //httpenphase.begin(String(configmodule.hostname), atoi(configmodule.port), url);
  httpenphase.begin(String(configmodule.hostname), 80 , url);
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


void Enphase_get_7(void) {
  if(WiFi.isConnected() and configmodule.token != "") {
    //create an HTTPClient instance

    String url = "/404.html";
    url = String(EnvoyJ);
    String adr = String(configmodule.hostname);
    Serial.println("Enphase_Get_7 : Start > https://" + adr + url);
    //Initializing an HTTPS communication using the secure client
//    if (strcmp(configmodule.port,"80") == 0 ) { configmodule.port = "443";}
    if (httpenphase.begin("https://" + adr + url)) {
      httpenphase.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
      httpenphase.addHeader("Authorization","Bearer "+String(configmodule.token));
      httpenphase.addHeader("Accept-Encoding","gzip, deflate, br");
      httpenphase.addHeader("User-Agent","PvRouter/1.1.1");
      int httpCode = httpenphase.GET();
      // httpCode will be negative on error
      Serial.println("Enphase_Get_7 : httpcode : " + httpCode);
      if (httpCode > 0) {
        if (httpCode == HTTP_CODE_FORBIDDEN) {
          // Token invalide
          //Enphase_TokenToConfig();
          Serial.println("Enphase token invalide ou périmé");

        } else if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          // Token valide - on interroge la passerelle
          String payload = httpenphase.getString();
          if (String(configmodule.envoy) == "R") {
            url = String(EnvoyR);
            Serial.print("type R ");
            Serial.println(url);
          }
          if (String(configmodule.envoy) == "S") {
            url = String(EnvoyS);
            Serial.print("type S ");
            Serial.println(url);
          }
          httpenphase.end();
          Serial.println("https://" + adr + url);
          if (httpenphase.begin("http://" + adr + url)) { 
            httpenphase.addHeader("Authorization","Bearer "+String(configmodule.token));
            httpenphase.addHeader("Accept-Encoding","gzip, deflate, br");
            /// httpenphase.addHeader("Connection","keep-alive");
            httpenphase.addHeader("User-Agent","PvRouter/1.1.1");
            httpCode = httpenphase.GET();
            /// Serial.print(" httpcode/ fonction mesure: ");
            /// Serial.println(httpCode);
            
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
              String payload = httpenphase.getString();
              DynamicJsonDocument doc(1024);
              deserializeJson(doc, payload);
              //Serial.print(payload);
              if (String(configmodule.envoy) == "R") {
                gDisplayValues.Fronius_prod = int(doc["wattsNow"]);
                gDisplayValues.Fronius_conso = int(doc["wattHoursToday"]);
              } else {
                gDisplayValues.Fronius_prod = int(doc["production"][1]["wNow"]);
                gDisplayValues.Fronius_conso = int(doc["consumption"][1]["wNow"]);
                gDisplayValues.Fronius_totalconso = int(doc["consumption"][0]["wNow"]);
              }
              // debug
              Serial.print(" > prod: " + String(gDisplayValues.Fronius_prod));
              Serial.print(" conso: " + String(gDisplayValues.Fronius_conso));
              Serial.println(" total conso: " + String(gDisplayValues.Fronius_totalconso));
              String test = doc["consumption"][0];
              

            } else {
                  Serial.println("[HTTPS] GET... failed, error: " + httpCode);
            }

            httpenphase.end();

          }
          else {
            Serial.println("[HTTPS] GET... failed, error: " + httpCode);
          }
          httpenphase.end();
        } else {
           //Enphase_TokenToConfig();
        }  
      }
    } else {
      Serial.println("Enphase_Get_7 : error");
    
    }
    ///client->stop();
    ///delay(5000);
  
  } else {
    if (configmodule.token == "")  {
      Serial.println("Enphase version 7 : Token vide");
    }
  }
  
}


#endif
