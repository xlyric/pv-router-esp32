#ifndef ENPHASE_FUNCTIONS
#define ENPHASE_FUNCTIONS

// géré par multinet33

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#include <FS.h>
#include "SPIFFS.h"

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"
#include "config/enums.h"

//***********************************
//************* Variables externes
//***********************************
extern Configmodule configmodule;

//***********************************
//************* Variables locales
//***********************************
constexpr const char *enphase_conf = "/enphase.json";
bool initEnphase = true; // Permet de lancer le contrôle du token une fois au démarrage
String SessionId;
int nbErreurCtrlTocken = 0;
int nbErreurGetJsonProd = 0;
bool TockenValide=false;

//***********************************
//************* Déclaration de fonctoons
//***********************************
void Enphase_get_5(void);
void Enphase_get_7(void);
bool Enphase_get_7_JWT();

//***********************************
//************* loadenphase
//***********************************
bool loadenphase(const char *filename) {
  Serial.println("******************LOAD ENPHASE*********************");

  // Open file for reading
  File configFile = SPIFFS.open(enphase_conf, "r");
  if (!configFile) {
    Serial.println(NOT_ENPHASE);
    
    logging.Set_log_init(NOT_ENPHASE,true);
    return false;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.  
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(NOT_ENPHASE);
    
    logging.Set_log_init(NOT_ENPHASE,true);
    return false;
  }

  // Copy values from the JsonDocument to the Config

  strlcpy(configmodule.hostname,                // <- destination
          doc["IP_ENPHASE"] | "",  // <- source
          sizeof(configmodule.hostname));       // <- destination's capacity
  strlcpy(configmodule.port, doc["PORT_ENPHASE"] | "443",
          sizeof(configmodule.port));   // <- destination's capacity
  strlcpy(configmodule.envoy,           // <- destination
          doc["Type"] | "S",            // <- source
          sizeof(configmodule.envoy));  // <- destination's capacity
  strlcpy(configmodule.version,           // <- destination
          doc["version"] | "7",            // <- source
          sizeof(configmodule.version));
  strlcpy(configmodule.token,         
          doc["token"] | "",            
          sizeof(configmodule.token));

  if (strcmp(configmodule.hostname,"") == 0) { 
    configmodule.enphase_present=false ; 
    configFile.close(); 
    Serial.println("no enphase"); 
    return false; 
  } 
  configmodule.enphase_present=true; 
  Serial.println(configmodule.hostname);
  configFile.close();

  Serial.println(" enphase hostname : " + String(configmodule.hostname));
  Serial.println(" enphase version : " + String(configmodule.version));
  Serial.println(" enphase mode : " + String(configmodule.envoy));
  Serial.println(" enphase port : " + String(configmodule.port));
  Serial.println(" enphase token : " + String(configmodule.token));

  Serial.println(" CONCLUSION enphase : actif");
  
  logging.Set_log_init("Enphase used type ",true);
  logging.Set_log_init(String(configmodule.envoy).c_str());
  logging.Set_log_init("\r\n");
  logging.Set_log_init("");
  
  return true;
}

//***********************************
//************* saveenphase
//***********************************
void saveenphase(const char *filename, const Configmodule &configmodule_save) {  
  // Open file for writing
   File configFile = SPIFFS.open(enphase_conf, "w");
  if (!configFile) {
    Serial.println(F("Failed to open enphase file "));
    

    logging.Set_log_init("Failed to open enphase file \r\n",true);
    return;
  } 

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/assistant to compute the capacity.
  JsonDocument doc;

  // Set the values in the document
  doc["IP_ENPHASE"] = configmodule_save.hostname;
  doc["PORT_ENPHASE"] = configmodule_save.port;
  doc["Type"] = configmodule_save.envoy;
  doc["version"] = configmodule_save.version;
  doc["token"] = configmodule_save.token;

  // Serialize JSON to file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to file enphase "));    
  }

  // Close the file
  configFile.close();
}

//***********************************
//************* Enphase_get
//***********************************
void Enphase_get(void) {
  if (String(configmodule.version) == "7") {
    if (String(configmodule.token)!="") {
      Serial.println("Enphase v7");
      Serial.println("==>IP: " + String(configmodule.hostname));
      Enphase_get_7();
    } else {
      Serial.println("Enphase version 7 : Token vide");
    }  
  } 
  else {
    Serial.println("Enphase v5");
    Enphase_get_5();
  }
}

//***********************************
//************* Enphase_get_5
//***********************************
void Enphase_get_5(void) {
  HTTPClient httpenphase; 
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

  httpenphase.begin(String(configmodule.hostname), 80 , url);
  /// workaround because envoy is too slow
  int httpResponseCode;
  int retryCount = 0;
  const int maxRetries = 10; // Arrêter après 10 tentatives
  
  while ((httpResponseCode = httpenphase.GET()) != 200 && retryCount < maxRetries)
  {
      retryCount++;
      delay(1000); // Attendre 1 seconde entre chaque tentative
  }

  // start connection and send HTTP header
  Serial.print(" httpcode/ fonction mesure: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode == HTTP_CODE_OK) {
    String payload = httpenphase.getString();

    JsonDocument doc; ///passé de 3200 à 1800 
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("Enphase_get_5() failed: "));
      logging.Set_log_init("Enphase_get_5() failed: ",true);
      Serial.println(error.c_str());
      return;
    }

    if (String(configmodule.envoy) == "R") {
      gDisplayValues.Fronius_prod = int(doc["wattsNow"]);
      gDisplayValues.Fronius_conso = int(doc["wattHoursToday"]);
    } 
    else {
      if ((gDisplayValues.Fronius_prod = int(doc["production"][1]["wNow"])) < 0) {
        // During night, Envoy measure a negative production
        gDisplayValues.Fronius_prod = 0;
      }

      gDisplayValues.Fronius_totalconso = int(doc["consumption"][0]["wNow"]);
      gDisplayValues.Fronius_conso = gDisplayValues.Fronius_totalconso - gDisplayValues.Fronius_prod;
    }

    gDisplayValues.porteuse = true; // si FALSE affiche No-Sin sur l'ecran
  } 
  else {
    Serial.println("Enphase Get 5 timeout");
  }

  httpenphase.end();

  // debug
  Serial.print("prod: " + String(gDisplayValues.Fronius_prod));
  Serial.print(" conso: " + String(gDisplayValues.Fronius_conso));
  Serial.println(" total conso: " + String(gDisplayValues.Fronius_totalconso));
}

//***********************************
//************* Enphase_get_7_Production
//***********************************
bool Enphase_get_7_Production(void){
  HTTPClient httpenphase;
  int httpCode;
  bool retour = false;
  String url = "/404.html" ;
  String adr = String(configmodule.hostname);
  String port = String(configmodule.port);
  String debut;

  if (String(configmodule.port) == "443")
    debut = "https://";
  else
    debut = "http://";

  Serial.println("**** Enphase_get_7_Production ****");  
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
         
  String fullurl = debut+adr+":"+port+url;
  Serial.println("Enphase Get production : " + fullurl);
  Serial.println("With SessionId : " + SessionId);
  if (httpenphase.begin(fullurl)) {
    httpenphase.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpenphase.setAuthorizationType("Bearer");
    httpenphase.setAuthorization(configmodule.token);
    httpenphase.addHeader("Accept-Encoding","gzip, deflate, br");
    httpenphase.addHeader("User-Agent","PvRouter/1.1.1");
    if (!SessionId.isEmpty()) {
      httpenphase.addHeader("Cookie",SessionId);
    }
    httpCode = httpenphase.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = httpenphase.getString();
      if (httpCode == HTTP_CODE_OK )
        Serial.println(payload);
      if (httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        Serial.println(httpenphase.getLocation());
        
      JsonDocument doc; ///passé de 3072 à 1600      
      DeserializationError error = deserializeJson(doc, payload);
      switch (error.code()) {
        case DeserializationError::Ok:
          break;
        case DeserializationError::InvalidInput:
          Serial.print(F("Deserialization error : Invalid input!"));
          logging.Set_log_init("Deserialization error : Invalid input!",true);
          return false;
          break;
        case DeserializationError::NoMemory:
          Serial.print(F("Deserialization error : Not enough memory"));
          logging.Set_log_init("Deserialization error : Not enough memory",true);
          return false;
          break;
        default:
          Serial.print(F("Deserialization error : Deserialization failed"));
          logging.Set_log_init("Deserialization error : Deserialization failed",true);
          return false;
          break;
      }
      
      if (String(configmodule.envoy) == "R") {
        gDisplayValues.Fronius_prod = int(doc["wattsNow"]);
        gDisplayValues.Fronius_conso = int(doc["wattHoursToday"]);
      } 
      else {
        if ((gDisplayValues.Fronius_prod = int(doc["production"][1]["wNow"])) < 0) {
          // During night, Envoy measure a negative production
          gDisplayValues.Fronius_prod = 0;
        }
        gDisplayValues.Fronius_totalconso = int(doc["consumption"][0]["wNow"]);
        gDisplayValues.Fronius_conso = gDisplayValues.Fronius_totalconso - gDisplayValues.Fronius_prod;
        gDisplayValues.enp_prod_whLifetime = int(doc["production"][1]["whLifetime"]);;
        gDisplayValues.enp_cons_whLifetime = int(doc["consumption"][0]["whLifetime"]);;
        gDisplayValues.enp_current_power_consumption = gDisplayValues.Fronius_conso;
        gDisplayValues.enp_current_power_production =  gDisplayValues.Fronius_prod;
      }
      // NEW
      gDisplayValues.porteuse = true; // si FALSE affiche No-Sin sur l'ecran
      nbErreurGetJsonProd=0;
      // FIN NEW
      retour = true;
      // debug
      Serial.println("Enphase Get production > prod: " + String(gDisplayValues.Fronius_prod) + " conso: " + String(gDisplayValues.Fronius_conso) + " total conso: " + String(gDisplayValues.Fronius_totalconso));
    } 
    else {
      Serial.println("[1.Enphase Get production] GET... failed, error: " + String(httpCode));
      nbErreurGetJsonProd++;
      httpenphase.end();
    }
    httpenphase.end();
  }
  else {
    nbErreurGetJsonProd++;
  }
  httpenphase.end();
  
  return retour;
}

//***********************************
//************* Enphase_get_7_JWT
//***********************************
bool Enphase_get_7_JWT(void) {
  HTTPClient httpenphase;
  bool retour = false;
  String url = "/404.html";
  url = String(EnvoyJ);
  String adr = String(configmodule.hostname);
  String port = String(configmodule.port);
  String debut;

  Serial.println("**** Enphase_get_7_JWT ****");    
  // Ne redemmande pas le tocken a chaque fois
  if (TockenValide)
    return true;

  if (String(configmodule.port) == "443")
    debut = "https://";
  else
    debut = "http://";

  String fullurl = debut+adr+":"+port+url;
  Serial.println("full url : " + fullurl);

  //new 2025
  Serial.println("Enphase contrôle token : " + fullurl + "<----");
  if (httpenphase.begin(fullurl)) { 
    Serial.println("Connexion réussie, préparation des headers (token etc)");
    httpenphase.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    httpenphase.setAuthorizationType("Bearer");
    httpenphase.setAuthorization(configmodule.token);
    httpenphase.addHeader("Accept-Encoding","gzip, deflate, br");
    httpenphase.addHeader("User-Agent","PvRouter/1.1.1");
    const char * headerkeys[] = {"Set-Cookie"};
    httpenphase.collectHeaders(headerkeys, sizeof(headerkeys)/sizeof(char*));
    httpenphase.setReuse(true);
    int httpCode = httpenphase.GET();
    
    //new 2025
    Serial.println("Réponse Headers :");
    for (int i = 0; i < httpenphase.headers(); i++) {
        Serial.println(httpenphase.headerName(i) + ": " + httpenphase.header(i));
    }    
    Serial.println("Erreur HTTPS : " + httpenphase.errorToString(httpCode));   

    // httpCode will be negative on error
    if (httpCode > 0) {
      Serial.println("JWT : code retour : " + String(httpCode));

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        retour = true;
        // Token valide
        Serial.println("Enphase contrôle token : TOKEN VALIDE ");
        TockenValide=true;
        SessionId.clear();
        SessionId = httpenphase.header("Set-Cookie");
        if (SessionId.indexOf("sessionId") < 0) {
          retour=false;
          SessionId.clear();
          Serial.println("Enphase contrôle token : PAS DE SESSION ID !!!");
          Serial.println("SessionId : " + SessionId);
        } else {
          SessionId.remove(0, SessionId.indexOf("sessionId"));
          if (SessionId.indexOf(";")) {SessionId.remove(SessionId.indexOf(";"));}
          Serial.println("Enphase contrôle token : " + SessionId);
        }
      } 
      else {
          Serial.println("Enphase contrôle token : TOKEN INVALIDE !!!");
          nbErreurCtrlTocken++;
      }
    }
    else {
      Serial.println("Error code : " + String(httpCode));
      nbErreurCtrlTocken++;
    }
  }
  //new 2025 (else)
  else 
    nbErreurCtrlTocken++;
  
  if (nbErreurCtrlTocken == 10)  {
    Serial.println("Trop d'erreur  : redemmarrage");
    ESP.restart();
  }
  
  //new 2025
  httpenphase.end();

  return retour;
}

//***********************************
//************* Enphase_get_7
//***********************************
void Enphase_get_7(void) {
  if(WiFi.isConnected() ) {    
    //create an HTTPClient instance
    if (SessionId.isEmpty() || ( (Enphase_get_7_Production() == false) && (nbErreurGetJsonProd > 10) ) ) { // Permet de lancer le contrôle du token une fois au démarrage (Empty SessionId)
      //DEBUG
      if (SessionId.isEmpty() ) {
        Serial.println("SessionId.isEmpty() return TRUE ==> Clean SessionId, get next token ");     
      }
      else {
        Serial.println("Enphase_get_7_production() return FALSE ==> Clean SessionId, get next token ");
      }   
      nbErreurGetJsonProd=0;
      SessionId.clear();
      TockenValide=false;
      Enphase_get_7_JWT();
    }
  } 
  else {
    Serial.println("Enphase_get_7 : Wifi not connected");
  } 
}

#endif
