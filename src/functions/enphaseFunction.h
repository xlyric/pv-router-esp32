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
String SessionId;
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
  Serial.println(" enphase version : " + String(configmodule.version));
  Serial.println(" enphase mode : " + String(configmodule.envoy));
  if (configmodule.enphase_present == true) {
    Serial.println(" enphase : actif");
    Serial.println(" enphase token : " + String(configmodule.token));
  } else {
    Serial.println(" enphase : inactif");
  }
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
    if (String(configmodule.token)!="") {
      //Serial.println("Enphase version 7");
      Enphase_get_7();
    } else {
      Serial.println("Enphase version 7 : pas de token");
    }
    
    
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

      if ((gDisplayValues.Fronius_prod = int(doc["production"][1]["wNow"])) < 0) {
        // During night, Envoy measure a negative production
        gDisplayValues.Fronius_prod = 0;
      }
      gDisplayValues.Fronius_totalconso = int(doc["consumption"][0]["wNow"]);
      gDisplayValues.Fronius_conso = gDisplayValues.Fronius_totalconso - gDisplayValues.Fronius_prod;
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

HTTPClient https;
bool initEnphase = true; // Permet de lancer le contrôle du token une fois au démarrage

bool Enphase_get_7_Production(void){
  
  int httpCode;
  bool retour = false;
  String adr = String(configmodule.hostname);
  String url = "/404.html" ;
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
          
  Serial.println("Enphase Get production : https://" + adr + url);
  if (https.begin("http://" + adr + url)) { 
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setAuthorizationType("Bearer");
    https.setAuthorization(configmodule.token);
    https.addHeader("Accept-Encoding","gzip, deflate, br");
    https.addHeader("User-Agent","PvRouter/1.1.1");
    if (!SessionId.isEmpty()) {
      https.addHeader("Cookie",SessionId);
    }
    httpCode = https.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = https.getString();
      //Serial.println(payload);
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, payload);
      if (String(configmodule.envoy) == "R") {
        gDisplayValues.Fronius_prod = int(doc["wattsNow"]);
        gDisplayValues.Fronius_conso = int(doc["wattHoursToday"]);
      } else {
        if ((gDisplayValues.Fronius_prod = int(doc["production"][1]["wNow"])) < 0) {
          // During night, Envoy measure a negative production
          gDisplayValues.Fronius_prod = 0;
        }
        gDisplayValues.Fronius_conso = gDisplayValues.Fronius_totalconso - gDisplayValues.Fronius_prod;
        gDisplayValues.Fronius_totalconso = int(doc["consumption"][0]["wNow"]);
        gDisplayValues.Fronius_conso = gDisplayValues.Fronius_totalconso - gDisplayValues.Fronius_prod;
        gDisplayValues.enp_prod_whLifetime = int(doc["production"][1]["whLifetime"]);;
        gDisplayValues.enp_cons_whLifetime = int(doc["consumption"][0]["whLifetime"]);;
        gDisplayValues.enp_current_power_consumption = gDisplayValues.Fronius_conso;
        gDisplayValues.enp_current_power_production =  gDisplayValues.Fronius_prod;
      }
      retour = true;
      // debug
      Serial.println("Enphase Get production > prod: " + String(gDisplayValues.Fronius_prod) + " conso: " + String(gDisplayValues.Fronius_conso) + " total conso: " + String(gDisplayValues.Fronius_totalconso));
    } else {
      Serial.println("[Enphase Get production] GET... failed, error: " + String(httpCode));
    }
    //https.end();
  }
  else {
    Serial.println("[Enphase Get production] GET... failed, error: " + String(httpCode));
  }
  return retour;
}

bool Enphase_get_7_JWT(void) {
  
  bool retour = false;
  String url = "/404.html";
  url = String(EnvoyJ);
  String adr = String(configmodule.hostname);
  Serial.println("Enphase contrôle tocken : https://" + adr + url);
  //Initializing an HTTPS communication using the secure client
  if (https.begin("https://" + adr + url)) {
    https.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    https.setAuthorizationType("Bearer");
    https.setAuthorization(configmodule.token);
    https.addHeader("Accept-Encoding","gzip, deflate, br");
    https.addHeader("User-Agent","PvRouter/1.1.1");
    const char * headerkeys[] = {"Set-Cookie"};
    https.collectHeaders(headerkeys, sizeof(headerkeys)/sizeof(char*));
    int httpCode = https.GET();
    
    // httpCode will be negative on error
    //Serial.println("Enphase_Get_7 : httpcode : " + httpCode);
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        retour = true;
        // Token valide
        Serial.println("Enphase contrôle tocket : TOKEN VALIDE ");
        SessionId.clear();
        SessionId = https.header("Set-Cookie");
      } else {
          Serial.println("Enphase contrôle tocket : TOKEN INVALIDE !!!");
          
      }
    }
  }
  //https.end();
  return retour;
}

void Enphase_get_7(void) {
  if(WiFi.isConnected() and configmodule.token != "") {
    //create an HTTPClient instance
    if (initEnphase == true) {
      initEnphase = false;
      Enphase_get_7_JWT();
    }
    if (Enphase_get_7_Production() == false) {
      Enphase_get_7_JWT();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  } else {
    if (configmodule.token == "")  {
      Serial.println("Enphase version 7 : Token vide");
    }
  }
  
}



#endif
