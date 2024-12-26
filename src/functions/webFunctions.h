#ifdef ESP32
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "appweb.h"

#include "functions/minuteur.h"

extern DisplayValues gDisplayValues;
extern Configmodule configmodule; 
extern Configwifi configwifi; 
extern Logs logging;
extern Programme programme; 
extern Programme programme_relay1; 
extern Programme programme_relay2; 
extern Memory task_mem; 


String inputMessage;

AsyncWebServer server(80);


		//***********************************
		//************* Setup - Web pages
		//***********************************

String getMinuteur(const Programme& minuteur);
String getMinuteur();
String return_Memory();

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

void compress_html(AsyncWebServerRequest *request,String filefs , String format ) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filefs, format);
      response->addHeader("Content-Encoding", "gzip");
      response->addHeader("Cache-Control", "max-age=604800");

      request->send(response);
}

void serveur_response(AsyncWebServerRequest *request, String response) {
  request->send(200, "text/plain", response.c_str());
}


void call_pages() {

//// pages ////
if (AP) {

    server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
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
  server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
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


  server.serveStatic("/all.min.css", SPIFFS, "/all.min.css").setCacheControl("max-age=31536000");
  server.serveStatic("/jquery.min.js", SPIFFS, "/jquery.min.js").setCacheControl("max-age=31536000");
  server.serveStatic("/bootstrap.bundle.min.js", SPIFFS, "/bootstrap.bundle.min.js").setCacheControl("max-age=31536000");
  server.serveStatic("/bootstrap.bundle.min.js.map", SPIFFS, "/bootstrap.bundle.min.js.map").setCacheControl("max-age=31536000");
  server.serveStatic("/fa-solid-900.woff2", SPIFFS, "/fa-solid-900.woff2").setCacheControl("max-age=31536000");
  server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico").setCacheControl("max-age=31536000");
  server.serveStatic("/sb-admin-2.min.css", SPIFFS, "/sb-admin-2.min.css").setCacheControl("max-age=31536000");
  server.serveStatic("/sb-admin-2.js", SPIFFS, "/sb-admin-2.js").setCacheControl("max-age=31536000");
  server.serveStatic("/log.html", SPIFFS, "/log.html").setCacheControl("max-age=31536000");
  
server.on("/mqtt.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/mqtt.json", "text/css");
  });

server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.json", "application/json");
  });

server.on("/envoy.html", HTTP_GET, [](AsyncWebServerRequest *request){
    compress_html(request,"/envoy.html.gz", "text/html");
  });

server.on("/enphase.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/enphase.json", "application/json");
  });

server.on("/minuteur.html",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/minuteur.html", "text/html");
  });

///// Pages 
/// Appel de fonction 

  server.on("/chart.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getchart().c_str());
  }); 

  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getState().c_str());
  });

  server.on("/statefull", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getStateFull().c_str());
  });

  
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getconfig().c_str());

  });


/// beta ? 
server.on("/cosphi", HTTP_GET, [](AsyncWebServerRequest *request){

    serveur_response(request, getcosphi());
  });
  
///////////////
//// wifi
///////////////

server.on("/wifi.html", HTTP_GET, [](AsyncWebServerRequest *request){
      compress_html(request,"/wifi.html.gz", "text/html");
  });

server.on("/getwifi", HTTP_ANY, [] (AsyncWebServerRequest *request) {
  serveur_response(request, getwifi());
});

server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "pong");
});

///////////////
///// MQTT
///////////////

server.on("/mqtt.html", HTTP_GET, [](AsyncWebServerRequest *request){
    compress_html(request,"/mqtt.html.gz", "text/html");
  });

server.on("/getmqtt", HTTP_ANY, [] (AsyncWebServerRequest *request) {
  request->send(200, "application/json",  getmqtt().c_str()); 
});

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
   config.restart = true;
   ESP.restart();
  });
  
server.onNotFound(notFound);

/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

server.on("/get", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///  doc  /get?disengage_dimmer=on
    if (request->hasParam(PARAM_INPUT_1)) {
                          String engagedimmer;
                          if(request->getParam(PARAM_INPUT_1)->value() == "on") {
                            engagedimmer = "dimmer disengaged";
                            gDisplayValues.dimmer_disengaged = true;
                          }
                          else {
                            engagedimmer = "dimmer engaged";
                            gDisplayValues.dimmer_disengaged = false;
                          }
                          request->send(200, "text/html", engagedimmer.c_str());}
                          
	   // doc /get?cycle=x
    if (request->hasParam(PARAM_INPUT_save)) { Serial.println(F("Saving configuration..."));

                                                    logging.Set_log_init(config.saveConfiguration(),true); // configuration sauvegardée
                            }
                           
	 if (request->hasParam(PARAM_INPUT_2)) { config.cycle = request->getParam(PARAM_INPUT_2)->value().toInt(); }
	 if (request->hasParam(PARAM_INPUT_3)) { config.readtime = request->getParam(PARAM_INPUT_3)->value().toInt();}
	 if (request->hasParam(PARAM_INPUT_4)) { config.cosphi = request->getParam(PARAM_INPUT_4)->value().toInt();  }
   if (request->hasParam(PARAM_INPUT_dimmer)) { request->getParam(PARAM_INPUT_dimmer)->value().toCharArray(config.dimmer,64);  }
   if (request->hasParam(PARAM_INPUT_server)) { request->getParam(PARAM_INPUT_server)->value().toCharArray(config.hostname,16);  }
   if (request->hasParam(PARAM_INPUT_delta)) { config.delta = request->getParam(PARAM_INPUT_delta)->value().toInt(); }
   if (request->hasParam(PARAM_INPUT_deltaneg)) { config.deltaneg = request->getParam(PARAM_INPUT_deltaneg)->value().toInt(); }
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
   if (request->hasParam("ssid")) { request->getParam("ssid")->value().toCharArray(configwifi.SID,50); wifimodif=true; }
   if (request->hasParam("password")) { 
    char password[50];  
       request->getParam("password")->value().toCharArray(password,50);
          if (strcmp(password,SECURITEPASS) != 0) {  ///sécurisation du mot de passe pas en clair     
              request->getParam("password")->value().toCharArray(configwifi.passwd,50); 
          }
      

    wifimodif=true; 
   }
   if (wifimodif) { configwifi.sauve_wifi(); }

    // Shelly
   if (request->hasParam("EM")) { request->getParam("EM")->value().toCharArray(config.topic_Shelly,100);  
      #ifdef NORMAL_FIRMWARE
      if (strcmp(config.topic_Shelly,"none") != 0 )  client.subscribe(config.topic_Shelly);
      else client.unsubscribe(config.topic_Shelly);
      #endif
   }

   // enphase
   bool enphasemodif=false ; 
   if (request->hasParam("envoyserver")) { request->getParam("envoyserver")->value().toCharArray(configmodule.hostname,16); enphasemodif=true; }
   if (request->hasParam("envmodele")) { request->getParam("envmodele")->value().toCharArray(configmodule.envoy,2);  enphasemodif=true;}
   if (request->hasParam("envversion")) { request->getParam("envversion")->value().toCharArray(configmodule.version,2); enphasemodif=true; }
   if (request->hasParam("envtoken")) { request->getParam("envtoken")->value().toCharArray(configmodule.token,425); enphasemodif=true; }
   if (enphasemodif) { saveenphase(enphase_conf, configmodule);}

   //// MQTT
   if (request->hasParam(PARAM_INPUT_mqttserver)) { request->getParam(PARAM_INPUT_mqttserver)->value().toCharArray(config.mqttserver,16);  }
   if (request->hasParam(PARAM_INPUT_publish)) { request->getParam(PARAM_INPUT_publish)->value().toCharArray(config.Publish,100); 
     logging.Set_log_init(config.saveConfiguration(),true); // configuration sauvegardée   
      }
   if (request->hasParam("mqttuser")) { request->getParam("mqttuser")->value().toCharArray(configmqtt.username,50);  }
   if (request->hasParam("mqttport")) { config.mqttport = request->getParam("mqttport")->value().toInt();}
   if (request->hasParam("mqttpassword")) {
       char password[50];  
       request->getParam("mqttpassword")->value().toCharArray(password,50);
          if (strcmp(password,SECURITEPASS) != 0) {  ///sécurisation du mot de passe pas en clair     
              request->getParam("mqttpassword")->value().toCharArray(configmqtt.password,50); 
          }
       logging.Set_log_init(configmqtt.savemqtt(),true); // configuration sauvegardée
       }

  //// Dimmer local
    if (request->hasParam("Fusiblelocal")) { config.localfuse = request->getParam("Fusiblelocal")->value().toInt();}
    if (request->hasParam("maxtemp")) { config.tmax = request->getParam("maxtemp")->value().toInt();}

   //reset
   if (request->hasParam(PARAM_INPUT_reset)) {Serial.println("Resetting ESP");  ESP.restart();}

   //// for check boxs in web pages  
   if (request->hasParam("servermode")) { inputMessage = request->getParam( PARAM_INPUT_servermode)->value();
                                            if (getServermode(inputMessage)) {
                                            logging.Set_log_init(config.saveConfiguration(),true); // configuration sauvegardée
                                            logging.Set_log_init(configmqtt.savemqtt(),true); // configuration sauvegardée
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

  server.on("/getminiteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("dimmer")) { request->send(200, "application/json",  getMinuteur(programme));  }
    else if (request->hasParam("relay1")) { request->send(200, "application/json",  getMinuteur(programme_relay1)); }
    else if (request->hasParam("relay2")) { request->send(200, "application/json",  getMinuteur(programme_relay2)); }
    else { request->send(200, "application/json",  getMinuteur());  }
  });


  server.on("/setminiteur", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      String name; 
      if (request->hasParam("dimmer")) { 
              if (request->hasParam("heure_demarrage")) { request->getParam("heure_demarrage")->value().toCharArray(programme.heure_demarrage,6);  }
              if (request->hasParam("heure_arret")) { request->getParam("heure_arret")->value().toCharArray(programme.heure_arret,6);  }
              if (request->hasParam("temperature")) { programme.temperature = request->getParam("temperature")->value().toInt();   }
              if (request->hasParam("puissance")) { programme.puissance = request->getParam("puissance")->value().toInt(); }
              programme.saveProgramme();
        request->send(200, "application/json",  getMinuteur(programme));  
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

}

String getMinuteur(const Programme& minuteur) {
    JsonDocument doc;
    getLocalTime(&timeinfo);
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

String getMinuteur() {
    JsonDocument doc;
    getLocalTime(&timeinfo);
    doc["heure"] = timeinfo.tm_hour;
    doc["minute"] = timeinfo.tm_min;

    String retour;
    serializeJson(doc, retour);
    return retour;
}

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