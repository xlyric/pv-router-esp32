#ifndef WEBFUNCTIONS
#define WEBFUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#ifdef ESP32
  #include <AsyncTCP.h>
  #include <esp_task_wdt.h>
#elif defined(ESP8266)
  #include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>

//***********************************
//************* PROGRAMME PV ROUTER
//***********************************
#include "appweb.h"
#include "functions/minuteur.h"
#ifdef WEBSOCKET_CLIENT
  #include "functions/websocket.h"
#endif

//***********************************
//************* Variables externes
//***********************************
extern DisplayValues gDisplayValues;
extern Configmodule configmodule; 
extern Configwifi configwifi; 
extern Logs logging;
extern Programme programme; 
extern Programme programme_relay1; 
extern Programme programme_relay2; 
extern Programme programme_marche_forcee;
extern Programme programme_batterie;
extern Memory task_mem; 
extern SemaphoreHandle_t mutex; 


//***********************************
//************* Déclaration de fonctions
//***********************************
extern bool boost();

//***********************************
//************* Variables locales
//***********************************
String inputMessage;
AsyncWebServer server(80);
String getMinuteur(const Programme& minuteur);
String getMinuteur();
String return_Memory();

//***********************************
//************* notfound()
//***********************************
void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//***********************************
//************* compress_html()
//***********************************
void compress_html(AsyncWebServerRequest *request,String filefs , String format ) {
  AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filefs, format);
  response->addHeader("Content-Encoding", "gzip");
  response->addHeader("Cache-Control", "max-age=604800");
  yield();
  request->send(response);
}

//***********************************
//************* serveur_response()
//***********************************
void serveur_response(AsyncWebServerRequest *request, String response) {
  yield();
  request->send(200, "text/plain", response.c_str());
}

//***********************************
//************* call_pages()
//***********************************
void call_pages() {
  if (AP) {
      server.on("/",HTTP_GET, [](AsyncWebServerRequest *request) {
        if(SPIFFS.exists("/index.html.gz")){
          compress_html(request,"/index-ap.html.gz", "text/html");
        }
        else {request->send(200, "text/html", "<html><body>Filesystem is not present. <a href='https://ota.apper-solaire.org/firmware/spiffs-ttgo.bin'>download it here</a> <br>and after  <a href='/update'>upload on the ESP here </a></body></html>" ); }
      });

      server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){
        if(SPIFFS.exists("/config.html.gz")){
          compress_html(request,"/config-ap.html.gz", "text/html");
        }
        else {
          serveur_response(request, SPIFFSNO);
        }
      });
    }
    else {
      server.on("/",HTTP_GET, [](AsyncWebServerRequest *request) {
        if(SPIFFS.exists("/index.html.gz") ){
          #ifndef LIGHT_FIRMWARE
            compress_html(request,"/index.html.gz", "text/html");
          #else
            compress_html(request,"/index-light.html.gz", "text/html");
          #endif
        }
        else {
          serveur_response(request, SPIFFSNO);
        }
      });

      server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){
        if(SPIFFS.exists("/config.html.gz")){
          #ifdef LIGHT_FIRMWARE
              compress_html(request,"/config-light.html.gz", "text/html");
          #elif ESP32D1MINI_FIRMWARE
            compress_html(request,"/config-dimmer.html.gz", "text/html");
          #else
            compress_html(request,"/config.html.gz", "text/html");
          #endif
        }
        else {
          serveur_response(request, SPIFFSNO);
        }
      });
    }

    // pages  statiques
    // Define static files array with URL paths and file paths
    const char* staticFiles[][2] = {
      {"/all.min.css", "/all.min.css"},
      {"/jquery.min.js", "/jquery.min.js"},
      {"/bootstrap.bundle.min.js", "/bootstrap.bundle.min.js"},
      {"/bootstrap.bundle.min.js.map", "/bootstrap.bundle.min.js.map"},
      {"/fa-solid-900.woff2", "/fa-solid-900.woff2"},
      {"/favicon.ico", "/favicon.ico"},
      {"/sb-admin-2.min.css", "/sb-admin-2.min.css"},
      {"/sb-admin-2.js", "/sb-admin-2.js"},
      {"/log.html", "/log.html"},
      {"/envoy.html", "/envoy.html"},
      {"/minuteur.html", "/minuteur.html"},
      {"/wifi.html", "/wifi.html"},
      {"/mqtt.html", "/mqtt.html"},
      {"/mqtt.json", "/mqtt.json"},
      {"/wifi.json", "/wifi.json"},
      {"/config.json", "/config.json"},
      {"/enphase.json", "/enphase.json"},
      {"/js/all.min.js", "/js/all.min.js"},
      {"/css/all.min.css", "/css/all.min.css"},
    };
  
    for (const auto& file : staticFiles) {
      if (strstr(file[0], ".json")) {
        server.serveStatic(file[0], SPIFFS, file[1]); // les json ne sont pas en cache
      } 
      else {
        server.serveStatic(file[0], SPIFFS, file[1]).setCacheControl("max-age=31536000"); // par contre les autres fichiers sont en cache
      }
    }

    server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", getState().c_str());
    });

    server.on("/stateshort", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", getState_short().c_str());
    });

    server.on("/statefull", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", getStateFull().c_str());
    });
    
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "application/json", getconfig().c_str());

    });

    // ajout de la commande de boost 2h   
    server.on("/boost", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      boost();    
      request->send(200, "application/json",  getMinuteur(programme_marche_forcee));
    });


    /// beta ? 
    server.on("/cosphi", HTTP_GET, [](AsyncWebServerRequest *request){
      serveur_response(request, getcosphi());
    });
    
    ///////////////
    //// wifi
    ///////////////
    server.on("/getwifi", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      serveur_response(request, getwifi());
    });

    server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "pong");
    });

    ///////////////
    //// Enphase
    ///////////////
    server.on("/getenvoy", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      serveur_response(request, getenvoy());
    });

    ///////////////
    ///// MQTT
    ///////////////
    server.on("/getmqtt", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      request->send(200, "application/json",  getmqtt().c_str()); 
    });

    ///////////////
    ///// LOGS
    ///////////////
    server.on("/log.txt", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/log.txt", "text/plain"); 
    });
    
    /// il serait bien que /getmqtt et getwifi soit directement en processing de l'appel de la page 

    ///////////////
    //// logs ////
    ///////////////
    server.on("/cs", HTTP_ANY, [](AsyncWebServerRequest *request){
      logging.Set_log_init("}1");
      serveur_response(request, logging.Get_log_init().c_str());
      // reinit de logging.log_init 
      logging.reset_log_init(); 
    });


    server.on("/reboot", HTTP_ANY, [](AsyncWebServerRequest *request){
    #ifndef LIGHT_FIRMWARE
      const int bufferSize = 150; //  Taille du tampon pour stocker le message
      char raison[bufferSize]; // NOSONAR
      getLocalTime( &timeinfo );
      snprintf(raison, bufferSize, "reboot manuel: %s", asctime(&timeinfo) ); 

        client.publish("memory/Routeur", raison, true);
    #endif
    request->redirect("/");
    yield();
    delay(1000);
    yield();
    delay(1000);
    yield();
    config.restart = true;
    ESP.restart();
    });
    
    // reset de la detection dallas précédente 
    server.on("/resetdallas", HTTP_ANY, [](AsyncWebServerRequest *request){
      config.dallas_present = false; 
      config.saveConfiguration();
      request->redirect("/");
 
    });

    server.onNotFound(notFound);

    /////////////////////////
    ////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
    /////////////////////////
    server.on("/get", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///  doc  /get?disengage_dimmer=on
      if (request->hasParam(PARAM_INPUT_1)) {
        const char* engagedimmer;
        if(request->getParam(PARAM_INPUT_1)->value() == "on") {
          engagedimmer = "dimmer disengaged";
          gDisplayValues.dimmer_disengaged = true;
        }
        else {
          engagedimmer = "dimmer engaged";
          gDisplayValues.dimmer_disengaged = false;
        }
        request->send(200, "text/html", engagedimmer);
      }
                              
      // doc /get?cycle=x
      if (request->hasParam(PARAM_INPUT_save)) { Serial.println(F("Saving configuration..."));
        logging.Set_log_init(config.saveConfiguration().c_str(),true); // configuration sauvegardée
      }
                              
      if (request->hasParam(PARAM_INPUT_2)) { 
        config.cycle = request->getParam(PARAM_INPUT_2)->value().toInt(); 
      }
      if (request->hasParam(PARAM_INPUT_3)) { 
        config.readtime = request->getParam(PARAM_INPUT_3)->value().toInt();
      }
      if (request->hasParam(PARAM_INPUT_4)) { 
        config.cosphi = request->getParam(PARAM_INPUT_4)->value().toInt();  
      }
      if (request->hasParam(PARAM_INPUT_dimmer)) { 
        request->getParam(PARAM_INPUT_dimmer)->value().toCharArray(config.dimmer,64); 
        #ifdef WEBSOCKET_CLIENT
        closeWebSocket(); setupWebSocket(); 
        #endif
      }

      if (request->hasParam(PARAM_INPUT_server)) { request->getParam(PARAM_INPUT_server)->value().toCharArray(config.hostname,16);  }
      if (request->hasParam(PARAM_INPUT_delta)) { config.delta_init = request->getParam(PARAM_INPUT_delta)->value().toInt(); config.batterie_active = false ; config.delta = config.delta_init; } // sauvegarde de la valeur initiale de delta
      if (request->hasParam(PARAM_INPUT_deltaneg)) { config.deltaneg_init = request->getParam(PARAM_INPUT_deltaneg)->value().toInt(); config.batterie_active = false; config.deltaneg = config.deltaneg_init; } // sauvegarde de la valeur initiale de deltaneg
      if (request->hasParam(PARAM_INPUT_port)) { config.port = request->getParam(PARAM_INPUT_port)->value().toInt(); }
      if (request->hasParam(PARAM_INPUT_IDX)) { config.IDX = request->getParam(PARAM_INPUT_IDX)->value().toInt();}
      if (request->hasParam(PARAM_INPUT_IDXdimmer)) { config.IDXdimmer = request->getParam(PARAM_INPUT_IDXdimmer)->value().toInt();}
      if (request->hasParam("idxdallas")) { config.IDXdallas = request->getParam("idxdallas")->value().toInt();}
      if (request->hasParam(PARAM_INPUT_API)) { request->getParam(PARAM_INPUT_API)->value().toCharArray(config.apiKey,64);}
      if (request->hasParam(PARAM_INPUT_dimmer_power)) {gDisplayValues.dimmer = request->getParam( PARAM_INPUT_dimmer_power)->value().toInt(); gDisplayValues.change = 1 ;  } 
      if (request->hasParam(PARAM_INPUT_facteur)) { config.facteur = request->getParam(PARAM_INPUT_facteur)->value().toFloat();}
      if (request->hasParam(PARAM_INPUT_tmax)) { config.tmax = request->getParam(PARAM_INPUT_tmax)->value().toInt();}
      if (request->hasParam("resistance")) { config.charge1 = request->getParam("resistance")->value().toInt(); config.calcul_charge(); }
      if (request->hasParam("resistance2")) { config.charge2 = request->getParam("resistance2")->value().toInt(); config.calcul_charge();}
      if (request->hasParam("resistance3")) { config.charge3 = request->getParam("resistance3")->value().toInt(); config.calcul_charge();}
      if (request->hasParam("screentime")) { config.ScreenTime = request->getParam("screentime")->value().toInt(); } 
      if (request->hasParam("voltage")) { config.voltage = request->getParam("voltage")->value().toInt();}
      if (request->hasParam("offset")) { config.offset = request->getParam("offset")->value().toInt();}
      if (request->hasParam("trigger")) { config.trigger = request->getParam("trigger")->value().toInt();}

      
      /// @brief  wifi
      bool wifimodif=false ; 
      if (request->hasParam("ssid")) { 
        request->getParam("ssid")->value().toCharArray(configwifi.SID,64); wifimodif=true; 
      }
      if (request->hasParam("password")) { 
        char password[64];  
        request->getParam("password")->value().toCharArray(password,64);
        if (strcmp(password,SECURITEPASS) != 0) {  ///sécurisation du mot de passe pas en clair     
          request->getParam("password")->value().toCharArray(configwifi.passwd,64); 
        }      

        wifimodif=true; 
      }
      if (request->hasParam("no_ap")) { 
        config.NO_AP = false;
        String no_ap_value = request->getParam("no_ap")->value();
        config.NO_AP = (no_ap_value == "true" || no_ap_value == "1");
        Serial.println("No AP mode : " + no_ap_value);
        config.saveConfiguration();
      }
      
      if (wifimodif) { 
        configwifi.sauve_wifi(); 
      }


      // Shelly
      if (request->hasParam("EM")) { 
        request->getParam("EM")->value().toCharArray(config.topic_Shelly,100);  
        #ifdef NORMAL_FIRMWARE
          if (strcmp(config.topic_Shelly,"none") != 0 )  
          client.subscribe(config.topic_Shelly);
          else client.unsubscribe(config.topic_Shelly);
        #endif
      }


      // enphase
      bool enphasemodif=false ; 
      if (request->hasParam("envoyserver")) { request->getParam("envoyserver")->value().toCharArray(configmodule.hostname,16); enphasemodif=true; }
      if (request->hasParam("envoyport")) { request->getParam("envoyport")->value().toCharArray(configmodule.port,5); enphasemodif=true; }
      if (request->hasParam("envmodele")) { request->getParam("envmodele")->value().toCharArray(configmodule.envoy,2);  enphasemodif=true;}
      if (request->hasParam("envversion")) { request->getParam("envversion")->value().toCharArray(configmodule.version,2); enphasemodif=true; }
      if (request->hasParam("envtoken")) { request->getParam("envtoken")->value().toCharArray(configmodule.token,512); enphasemodif=true; }
      if (enphasemodif) { 
        saveenphase(enphase_conf, configmodule);
      }

      //// MQTT
      if (request->hasParam(PARAM_INPUT_mqttserver)) { request->getParam(PARAM_INPUT_mqttserver)->value().toCharArray(config.mqttserver,16);  }
      if (request->hasParam(PARAM_INPUT_publish)) { 
        request->getParam(PARAM_INPUT_publish)->value().toCharArray(config.Publish,100); 
        logging.Set_log_init(config.saveConfiguration().c_str(),true); // configuration sauvegardée   
      }
      if (request->hasParam("mqttuser")) { request->getParam("mqttuser")->value().toCharArray(configmqtt.username,50);  }
      if (request->hasParam("mqttport")) { config.mqttport = request->getParam("mqttport")->value().toInt();}
      if (request->hasParam("mqttpassword")) {
        char password[65];  
        request->getParam("mqttpassword")->value().toCharArray(password,65);
        if (strcmp(password,SECURITEPASS) != 0) {  ///sécurisation du mot de passe pas en clair     
          request->getParam("mqttpassword")->value().toCharArray(configmqtt.password,65); 
        }
        logging.Set_log_init(configmqtt.savemqtt().c_str(),true); // configuration sauvegardée
      }

      //// Dimmer local
      if (request->hasParam("Fusiblelocal")) { config.localfuse = request->getParam("Fusiblelocal")->value().toInt();}
      if (request->hasParam("maxtemp")) { config.tmax = request->getParam("maxtemp")->value().toInt();}

      //reset
      if (request->hasParam(PARAM_INPUT_reset)) {
        Serial.println("Resetting ESP");  
        ESP.restart();
      }

      //// for check boxs in web pages  
      if (request->hasParam("servermode")) { inputMessage = request->getParam( PARAM_INPUT_servermode)->value();
      if (getServermode(inputMessage)) {
        logging.Set_log_init(config.saveConfiguration().c_str(),true); // configuration sauvegardée
        logging.Set_log_init(configmqtt.savemqtt().c_str(),true); // configuration sauvegardée
      }
        request->send(200, "text/html", getconfig().c_str());
      }

      /// relays : 0 : off , 1 : on , other : switch 
      if (request->hasParam("relay1")) { int relay = request->getParam("relay1")->value().toInt(); 
        if ( relay == 0 ) { digitalWrite(RELAY1 , HIGH); } // correction bug de démarrage en GPIO 0
        else if ( relay == 1 ) { digitalWrite(RELAY1 , LOW); } // correction bug de démarrage en GPIO 0
        else if (relay == 2) { digitalWrite(RELAY1, !digitalRead(RELAY1)); }
        int relaystate = digitalRead(RELAY1); 
        char str[8];// NOSONAR
        itoa( relaystate, str, 10 );
        request->send(200, "text/html", str );
      }
      if (request->hasParam("relay2")) { int relay = request->getParam("relay2")->value().toInt(); 
        if ( relay == 0) { digitalWrite(RELAY2 , LOW); }
        else if ( relay == 1 ) { digitalWrite(RELAY2 , HIGH); } 
        else if (relay == 2) { digitalWrite(RELAY2, !digitalRead(RELAY2)); }
        int relaystate = digitalRead(RELAY2); 
        char str[8];// NOSONAR
        itoa( relaystate, str, 10 );
        request->send(200, "text/html", str );
      }
      if (request->hasParam("SCT_13")) { config.SCT_13 = request->getParam("SCT_13")->value().toInt();  
        /// la valeur de la sonde doit être entre 20 et 100 ( )
        if (config.SCT_13 < 20) config.SCT_13 = 20;
        if (config.SCT_13 > 100) config.SCT_13 = 100;
      }

      //// minuteur 
      if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme.heure_demarrage,6);  }
      if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme.heure_arret,6);  }
      if (request->hasParam("temperature")) { programme.temperature = request->getParam("temperature")->value().toInt();  programme.saveProgramme(); }
      serveur_response(request,  getconfig());
      }); 

      server.on("/getminuteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
        esp_task_wdt_reset();
        if (request->hasParam("dimmer")) { request->send(200, "application/json",  getMinuteur(programme));  }
        else if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
        else if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
        else if (request->hasParam("batterie")) { request->send(200, "application/json",  getMinuteur(programme_batterie)); }
        else { request->send(200, "application/json",  getMinuteur());  }
    }); // /get

    server.on("/setminuteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
//      String name; 
      if (request->hasParam("dimmer")) { 
              if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme.heure_demarrage,6);  }
              if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme.heure_arret,6);  }
              if (request->hasParam("temperature")) { programme.temperature = request->getParam("temperature")->value().toInt();   }
              if (request->hasParam("puissance")) { programme.puissance = request->getParam("puissance")->value().toInt(); }
              programme.saveProgramme();
        request->send(200, "application/json",  getMinuteur(programme));  
      }
      if (request->hasParam("batterie")) { 
              if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme_batterie.heure_demarrage,6);  }
              if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme_batterie.heure_arret,6);  }
              if (request->hasParam("temperature")) { programme_batterie.temperature = request->getParam("temperature")->value().toInt();   }
              if (request->hasParam("puissance")) { programme_batterie.puissance = request->getParam("puissance")->value().toInt(); }
              programme_batterie.saveProgramme();
        request->send(200, "application/json",  getMinuteur(programme_batterie));  
      }
      if (request->hasParam("relay1")) { 
            if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme_relay1.heure_demarrage,6);  }
            if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme_relay1.heure_arret,6);  }
            if (request->hasParam("temperature")) { programme_relay1.temperature = request->getParam("temperature")->value().toInt();  }
            programme_relay1.saveProgramme(); 
      request->send(200, "application/json",  getMinuteur(programme_relay1)); 
      }
      if (request->hasParam("relay2")) { 
              if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme_relay2.heure_demarrage,6);  }
              if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme_relay2.heure_arret,6);  }
              if (request->hasParam("temperature")) { programme_relay2.temperature = request->getParam("temperature")->value().toInt();  }
              programme_relay2.saveProgramme(); 
        request->send(200, "application/json",  getMinuteur(programme_relay2)); 
      }
      else { request->send(200, "application/json",  getMinuteur()); }
    });

    server.on("/getmemory", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      request->send(200, "application/json",  return_Memory());
    });
  } // call_pages()

//***********************************
//************* getMinuteur(const Programme& minuteur)
//***********************************
String getMinuteur(const Programme& minuteur) {
  JsonDocument doc;
    if (millis() < 5000) {
        Serial.println("System not ready yet");
        esp_task_wdt_reset();
        return "false";
  }
  yield();
  struct tm timeinfo;  // Déclaration locale
  if (!gDisplayValues.Shelly_local) {
        if (!getLocalTime(&timeinfo)) {
          Serial.println("Failed to obtain time");
          esp_task_wdt_reset();
          return "false";
      }
  }
  yield();
  doc["heure_demarrage"] = minuteur.heure_demarrage;
  doc["heure_arret"] = minuteur.heure_arret;
  doc["temperature"] = minuteur.temperature;
  doc["heure"] = timeinfo.tm_hour;
  doc["minute"] = timeinfo.tm_min;
  doc["puissance"] = minuteur.puissance;

  String retour;
  serializeJson(doc, retour);
  return retour;
}

//***********************************
//************* getMinuteur()
//***********************************
String getMinuteur() {
  //if (gDisplayValues.Shelly_local) { return "{}"; } // si Shelly local, pas de minuteur
  JsonDocument doc;
      if (millis() < 5000) {
        Serial.println("System not ready yet");
        esp_task_wdt_reset();
        return "false";
  }
  yield();
  
  struct tm timeinfo;  // Déclaration locale
      if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        esp_task_wdt_reset();
        return "false";
    }
  yield();
  doc["heure"] = timeinfo.tm_hour;
  doc["minute"] = timeinfo.tm_min;

  String retour;
  serializeJson(doc, retour);
  return retour;
}

//***********************************
//************* return_Memory()
//***********************************
String return_Memory() {
  JsonDocument doc;
  doc["task_GetDImmerTemp"] = task_mem.task_GetDImmerTemp;
  doc["task_dallas_read"] = task_mem.task_dallas_read;
  doc["task_keepWiFiAlive2"] = task_mem.task_keepWiFiAlive2;
  doc["task_measure_electricity"] = task_mem.task_measure_electricity;
  doc["task_send_mqtt"] = task_mem.task_send_mqtt;
  doc["task_serial_read_task"] = task_mem.task_serial_read_task;
  doc["task_switchDisplay"] = task_mem.task_switchDisplay;
  doc["task_updateDimmer"] = task_mem.task_updateDimmer;
  doc["task_updateDisplay"] = task_mem.task_updateDisplay;
  doc["task_loop"] = task_mem.task_loop;

  String retour;
  serializeJson(doc, retour);
  return retour;
}
#endif
