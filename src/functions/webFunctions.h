#ifdef ESP32
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "appweb.h"

extern DisplayValues gDisplayValues;
extern Configmodule configmodule; 

//***********************************
//************* Gestion du serveur WEB
//***********************************
// Create AsyncWebServer object on port 80
//WiFiClient domotic_client;
// mqtt
//PubSubClient client(domotic_client);

String inputMessage;

AsyncWebServer server(80);
//DNSServer dns;
//HTTPClient http;

		//***********************************
		//************* Setup - Web pages
		//***********************************

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


void call_pages() {



if (AP) {
    server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/index.html")){
     request->send(SPIFFS, "/index-ap.html", "text/html");
    }
    else {request->send_P(200, "text/plain", SPIFFSNO ); }
  });

  server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/config.html")){
      request->send(SPIFFS, "/config-ap.html", "text/html");
    }
    else {request->send_P(200, "text/plain", SPIFFSNO ); }

  });
}
else {
  server.on("/",HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/index.html")){
     request->send(SPIFFS, "/index.html", "text/html");
    }
    else {request->send_P(200, "text/plain", SPIFFSNO ); }
  });

  server.on("/config.html", HTTP_GET, [](AsyncWebServerRequest *request){
    if(SPIFFS.exists("/config.html")){
      request->send(SPIFFS, "/config.html", "text/html");
    }
    else {request->send_P(200, "text/plain", SPIFFSNO ); }

  });

}

  server.on("/all.min.css",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/all.min.css", "text/css");
  });

server.on("/loader.js",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/loader.js", "text/css");
  });

server.on("/google.css",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/google.css", "text/css");
  });

server.on("/jquery.min.js",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.min.js", "text/css");
  });

server.on("/jquery.easing.min.js",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery.easing.min.js", "text/css");
  });

server.on("/bootstrap.bundle.min.js",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap.bundle.min.js", "text/css");
  });


server.on("/bootstrap.bundle.min.js.map",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/bootstrap.bundle.min.js.map", "text/css");
  });

    server.on("/favicon.ico",  HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/favicon.ico", "image/png");
  });

  server.on("/fa-solid-900.woff2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/fa-solid-900.woff2", "text/css");
  });
  
    server.on("/sb-admin-2.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sb-admin-2.js", "text/javascript");
  });

  server.on("/sb-admin-2.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/sb-admin-2.min.css", "text/css");
  });

if (!configmodule.pilote) {
  server.on("/chart.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/json", getchart().c_str());
  }); 
}

  server.on("/sendmode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getSendmode().c_str());
  });
  
  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState().c_str());
  });

  server.on("/serial", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getState().c_str());
  });
  
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getconfig().c_str());
  });

  server.on("/memory", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getmemory().c_str());
  });

  server.on("/debug", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getdebug().c_str());
  });

  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getlogs().c_str());
  });

server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.json", "application/json");
  });

server.on("/doc.txt", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/doc.txt", "text/plain");
  });

/// beta
server.on("/cosphi", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getcosphi().c_str());
  });
  
server.on("/puissance", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain",  getpuissance().c_str());
  });



server.onNotFound(notFound);

/////////////////////////
////// mise à jour parametre d'envoie vers domoticz et récupération des modifications de configurations
/////////////////////////

server.on("/get", HTTP_ANY, [] (AsyncWebServerRequest *request) {
      ///   /get?send=on
    if (request->hasParam(PARAM_INPUT_1)) 		  { inputMessage = request->getParam(PARAM_INPUT_1)->value();
													config.sending = 0; 
													if ( inputMessage != "On" ) { config.sending = 1; }
													request->send(200, "text/html", getSendmode().c_str()); 	}
	   // /get?cycle=x
    if (request->hasParam(PARAM_INPUT_save)) { Serial.println(F("Saving configuration..."));
                          saveConfiguration(filename_conf, config);   
                            }
                           
	 if (request->hasParam(PARAM_INPUT_2)) { config.cycle = request->getParam(PARAM_INPUT_2)->value().toInt(); }
	 if (request->hasParam(PARAM_INPUT_3)) { config.readtime = request->getParam(PARAM_INPUT_3)->value().toInt();}
	 if (request->hasParam(PARAM_INPUT_4)) { config.cosphi = request->getParam(PARAM_INPUT_4)->value().toInt();  }
   if (request->hasParam(PARAM_INPUT_dimmer)) { request->getParam(PARAM_INPUT_dimmer)->value().toCharArray(config.dimmer,15);  }
   if (request->hasParam(PARAM_INPUT_server)) { request->getParam(PARAM_INPUT_server)->value().toCharArray(config.hostname,15);  }
   if (request->hasParam(PARAM_INPUT_mqttserver)) { request->getParam(PARAM_INPUT_mqttserver)->value().toCharArray(config.mqttserver,15);  }
   if (request->hasParam(PARAM_INPUT_publish)) { request->getParam(PARAM_INPUT_publish)->value().toCharArray(config.Publish,100);  }
   if (request->hasParam(PARAM_INPUT_delta)) { config.delta = request->getParam(PARAM_INPUT_delta)->value().toInt(); }
   if (request->hasParam(PARAM_INPUT_deltaneg)) { config.deltaneg = request->getParam(PARAM_INPUT_deltaneg)->value().toInt(); }
   if (request->hasParam(PARAM_INPUT_fuse)) { config.num_fuse = request->getParam(PARAM_INPUT_fuse)->value().toInt(); }
   if (request->hasParam(PARAM_INPUT_port)) { config.port = request->getParam(PARAM_INPUT_port)->value().toInt(); }
   if (request->hasParam(PARAM_INPUT_IDX)) { config.IDX = request->getParam(PARAM_INPUT_IDX)->value().toInt();}
   if (request->hasParam(PARAM_INPUT_IDXdimmer)) { config.IDXdimmer = request->getParam(PARAM_INPUT_IDXdimmer)->value().toInt();}
   if (request->hasParam(PARAM_INPUT_IDXprod)) { config.IDXprod = request->getParam(PARAM_INPUT_IDXprod)->value().toInt();}
   if (request->hasParam(PARAM_INPUT_API)) { request->getParam(PARAM_INPUT_API)->value().toCharArray(config.apiKey,64);}
   if (request->hasParam(PARAM_INPUT_dimmer_power)) {gDisplayValues.dimmer = request->getParam( PARAM_INPUT_dimmer_power)->value().toInt(); gDisplayValues.change = 1 ;  } 
   if (request->hasParam(PARAM_INPUT_facteur)) { config.facteur = request->getParam(PARAM_INPUT_facteur)->value().toFloat();}
   if (request->hasParam(PARAM_INPUT_tmax)) { config.tmax = request->getParam(PARAM_INPUT_tmax)->value().toInt();}
   if (request->hasParam("resistance")) { config.resistance = request->getParam("resistance")->value().toInt();}
   if (request->hasParam("screentime")) { config.ScreenTime = request->getParam("screentime")->value().toInt();}
   //reset
   if (request->hasParam(PARAM_INPUT_reset)) {Serial.println("Resetting ESP");  ESP.restart();}
      
   if (request->hasParam(PARAM_INPUT_servermode)) { inputMessage = request->getParam( PARAM_INPUT_servermode)->value();
                                            getServermode(inputMessage);
                                            request->send(200, "text/html", getconfig().c_str());
                                        }

  
    request->send(200, "text/html", getconfig().c_str());

	}); 

}

