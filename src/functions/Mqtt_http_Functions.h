#ifndef MQTT_FUNCTIONS
#define MQTT_FUNCTIONS

    #include <Arduino.h>
    #include "../config/config.h"
    #include "../config/enums.h"
    #include "functions/spiffsFunctions.h"
    #include <PubSubClient.h>
    #include <WiFi.h>
    #include "HTTPClient.h"
        
    WiFiClient espClient;
    PubSubClient client(espClient);

    extern DisplayValues gDisplayValues;

/***
 *  reconnexion au serveur MQTT
 */

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient",MQTT_USER, MQTT_PASSWORD )) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc="); 
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*
*    Fonction d'envoie info MQTT vers domoticz
*/

void Mqtt_send ( String idx, String value ) {
  
  String nvalue = "0" ; 
  
  if ( value != "0" ) { 
      nvalue = "2" ; 
      }
  
  String message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";

  String jdompub = String(config.Publish) + "/"+idx ;

  client.loop();
  client.publish(config.Publish, String(message).c_str(), true);
  client.publish(jdompub.c_str() , value.c_str(), true);

}

/*
*    Fonction d'init de MQTT 
*/

void Mqtt_init() {

  client.connect("pvrouter");
  client.setServer(config.mqttserver, 1883);
  Mqtt_send(String(config.IDXdimmer),"0");   

}

#endif