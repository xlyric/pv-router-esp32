#ifndef MQTT_FUNCTIONS
#define MQTT_FUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#include <WiFi.h>
#include "HTTPClient.h"
#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>
#endif

//***********************************
//************* PROGRAMME PV ROUTER
//***********************************
#include "../config/config.h"
#include "../config/enums.h"
#include "functions/spiffsFunctions.h"

//***********************************
//************* Variables externes
//***********************************
extern Config config;
extern DisplayValues gDisplayValues;
extern Mqtt configmqtt;
extern Logs logging;
extern float WHtempgrid; 
extern float WHtempinject;

extern HA device_dimmer; 
extern HA device_routeur; 
extern HA device_routed; 
extern HA device_dimmer_power; 
extern HA device_grid;
extern HA device_inject;
extern HA compteur_inject;
extern HA compteur_grid;
extern HA switch_1;
extern HA temperature_HA;
extern HA power_factor;
extern HA power_apparent;
extern HA switch_relay1;
extern HA switch_relay2;
extern HA device_dimmer_boost;
extern xSemaphoreHandle mutex;

//***********************************
//************* Variables locales
//***********************************
WiFiClient espClient;


#ifndef LIGHT_FIRMWARE
  PubSubClient client(espClient);
  
  //***********************************
  //************* reconnect()
  //*************  reconnexion au MQTT
  //***********************************
  void reconnect() {        
    if (strcmp(config.mqttserver,"none") == 0) {
      Serial.println("MQTT_init : MQTT désactivé");
     return;
    }

    const String pvname = String("PvRouter-") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17); 
    const String topic = "homeassistant/sensor/"+ pvname +"/status";
    // Loop until we're reconnected
    while (!client.connected()) {
      logging.clean_log_init();
      Serial.println("-----------------------------");
      Serial.println("Attempting MQTT reconnection...");
      
      logging.Set_log_init(Attempting_MQTT_connexion,true);
      //affichage du RSSI
      logging.Set_log_init(String(WiFi.RSSI())+" dBm\r\n");

      // Attempt to connect
      if (client.connect(pvname.c_str(), configmqtt.username, configmqtt.password)) {
        // Once connected, publish online to the availability topic
        client.publish(topic.c_str(), "online", true);         
        client.setKeepAlive(15);  
        client.setBufferSize(1024);    
        logging.Set_log_init(MQTT_connected,true);
        Serial.println(MQTT_connected);
      } else {
        Serial.print("MQTT failed, retcode="); 
        Serial.print(client.state());
        Serial.println(" try again in 2 seconds");
        ///dans le doute si le mode AP est actif on le coupe
        Serial.println(WiFi.status());

        // Wait 2 seconds before retrying
        delay(2000);  // 24/01/2023 passage de 5 à 2s 
      } // else
    } // while
  }

  //***********************************
  //************* Mqtt_send()
  //*************  envoie MQTT vers Domoticz/Jeedom
  //***********************************
  void Mqtt_send ( String idx, String value, String otherpub = "" , String name = "")  {    
    // si IDX = 0 ou vide on ne fait rien
    if (idx != "0" || idx != "" ) { 
      String nvalue = "0" ;     
      if ( value != "0" ) { 
          nvalue = "2" ; 
      }
      
      String message; 
      if (otherpub == "" ) {
        message = R"( { "idx" : )" + idx + R"( , "svalue" : ")" + value + R"(",  "nvalue" : )" + nvalue + R"( } )";
      }

      String jdompub = String(config.Publish) + "/"+idx ;
      if (otherpub != "" ) {
        jdompub += "/"+otherpub; 
      }      

      if (client.connected() && (WiFi.status() == WL_CONNECTED ))  {
        client.loop();
        if (otherpub == "" ) {
         client.publish(config.Publish, String(message).c_str(), true);
        }
        client.publish(jdompub.c_str() , value.c_str(), true);
      }
    }
  }

  //***********************************
  //************* callback()
  //***********************************
  void callback(char* topic, byte* payload, unsigned int length) {
  // Ajout d'un espace pour le caractère nul // NOSONAR
  char arrivage[length+1]; 
  int recup = 0;

  for (int i=0;i<length;i++) {
    arrivage[i] = (char)payload[i];
  }
  arrivage[length] = '\0'; // Ajouter le caractère nul à la fin
    
  if (strcmp( topic, config.topic_Shelly ) == 0 ) {
    if (strcmp( arrivage , "unavailable" ) == 0 ) { 
      gDisplayValues.Shelly = -2; 
    }
    else { 
      DEBUG_PRINTLN("MQTT callback : Shelly = "+String(arrivage));
      // Utiliser strtol pour une conversion plus robuste
      char* endPtr;
      double shellyValue = strtod(arrivage, &endPtr);

      if (endPtr != arrivage && *endPtr == '\0') {
        // La conversion s'est déroulée avec succès
        gDisplayValues.Shelly = shellyValue;
      } else {
        DEBUG_PRINTLN("Erreur : Conversion de la chaîne en virgule flottante a échoué");
      } 
    } // if (strcmp( arrivage , "unavailable" ) == 0 ) 
  } // if (strcmp( topic, config.topic_Shelly ) == 0 )

  if (strcmp( topic, ("memory/"+compteur_grid.topic+compteur_grid.Get_name()).c_str() ) == 0 ) {        
    Serial.println("MQTT callback : compteur_grid = "+String(arrivage));
    // Utiliser strtol pour une conversion plus robuste
    char* endPtr;
    double mqttValue = strtod(arrivage, &endPtr);

    if (endPtr != arrivage && *endPtr == '\0') {
      // La conversion s'est déroulée avec succès
      WHtempgrid = mqttValue;
      recup ++;
      /// mise à jour topic officiel
      client.publish((compteur_grid.topic+compteur_grid.Get_name()).c_str(), String(WHtempgrid).c_str(),true);
    } else {
      DEBUG_PRINTLN("Erreur : Conversion de la chaîne en virgule flottante a échoué");
    }
  } // if (strcmp( topic, ("memory/"+compteur_grid.topic+compteur_grid.Get_name()).c_str() ) == 0 )

  if (strcmp( topic, ("memory/"+compteur_inject.topic+compteur_inject.Get_name()).c_str() ) == 0 ) {
    Serial.println("MQTT callback : compteur_inject = "+String(arrivage));
    // Utiliser strtol pour une conversion plus robuste
    char* endPtr;
    double mqttValue = strtod(arrivage, &endPtr);

    if (endPtr != arrivage && *endPtr == '\0') {
      // La conversion s'est déroulée avec succès
      WHtempinject= mqttValue;
      recup ++;
      /// mise à jour topic officiel
      client.publish((compteur_inject.topic+compteur_inject.Get_name()).c_str(), String(WHtempinject).c_str(),true);      
    } 
    else {
      DEBUG_PRINTLN("Erreur : Conversion de la chaîne en virgule flottante a échoué");
    }        
  } // if (strcmp( topic, ("memory/"+compteur_inject.topic+compteur_inject.Get_name()).c_str() ) == 0 )
  if (WHtempgrid != 0 && WHtempinject !=0 ) { 
    client.unsubscribe(("memory/"+compteur_grid.topic+"#").c_str()); 
  }

  // TODO, faire la récupération pour le mode boost
  }


  //***********************************
  //************* Mqtt_init()
  //***********************************
  void Mqtt_init() {
    // comparaison de config.mqttserver avec none 
    if (strcmp(config.mqttserver,"none") == 0) {
      Serial.println("MQTT_init : MQTT désactivé");
      return;
    }

    Serial.println("MQTT_init : server="+String(config.mqttserver));
    Serial.println("MQTT_init : port="+String(config.mqttport));
    
    client.setServer(config.mqttserver, config.mqttport);
    client.setCallback(callback);
        
    Serial.println("MQTT_init : connexion...");
    reconnect();

    // récupération des topics des anciennes valeurs 
    Serial.println("récupération des anciennes valeurs de consommation...");
    Serial.println(("memory/"+compteur_grid.topic+"#").c_str());
    Serial.println("memory/"+compteur_grid.topic+compteur_grid.Get_name()) ;

    client.subscribe(("memory/"+compteur_grid.topic+"#").c_str());
    client.loop();
 
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {  // Prend le mutex  
      if (config.IDXdimmer != 0 ) {  
        Mqtt_send(String(config.IDXdimmer),"0","","Dimmer"); 
      }
      if (strcmp(config.topic_Shelly,"none") != 0) 
        client.subscribe(config.topic_Shelly);
      xSemaphoreGive(mutex);  // Libère le mutex
    }
  }
#endif //ifndef LIGHT_FIRMWARE
#endif //ifndef MQTT_FUNCTIONS
