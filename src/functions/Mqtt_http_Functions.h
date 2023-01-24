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
extern Config config;
extern DisplayValues gDisplayValues;
extern Mqtt configmqtt;

void Mqtt_HA_hello();

/***
 *  reconnexion au serveur MQTT
 */

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("-----------------------------");
    Serial.println("Attempting MQTT reconnection...");
    // Attempt to connect

    if (client.connect("pvrouter",configmqtt.username, configmqtt.password )) {
      Serial.println("MQTT reconnect : connected");
    } else {
      Serial.print("MQTT reconnect : failed, retcode="); 
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);  // 24/01/2023 passage de 5 à 2s 
    }
  }
}

/*
*    Fonction d'envoie info MQTT vers domoticz
*/

void Mqtt_send ( String idx, String value, String otherpub = "" ) {
  
  String nvalue = "0" ; 
  
  if ( value != "0" ) { 
      nvalue = "2" ; 
      }
  
  String message; 
  if (otherpub == "" ) {
    message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";
  }

  String jdompub = String(config.Publish) + "/"+idx ;
  if (otherpub != "" ) {jdompub += "/"+otherpub; }
  


  client.loop();
    if (otherpub == "" ) {
      if (client.publish(config.Publish, String(message).c_str(), true)) {
     //   Serial.println("MQTT_send : MQTT sent to domoticz");
      }

      else {
        Serial.println("MQTT_send : error publish to domoticz ");
      }
    }
  if (client.publish(jdompub.c_str() , value.c_str(), true)){
  //  Serial.println("MQTT_send : MQTT sent to Jeedom ");
  }
  else {
Serial.println("MQTT_send : error publish to Jeedom ");
  }
  

}

/*
Fonction MQTT callbakc
*
*/
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


/*
*    Fonction d'init de MQTT 
*/

void Mqtt_init() {
  // debug
  Serial.println("MQTT_init : server="+String(config.mqttserver));
  Serial.println("MQTT_init : port="+String(config.mqttport));
  
  client.setServer(config.mqttserver, config.mqttport);
  client.setCallback(callback);
  Serial.print("MQTT_init : connexion...");

  if (client.connect("pvrouter",configmqtt.username,configmqtt.password)) {
    Serial.println("MQTT_init : connecte a MQTT... Initialisation dimmer à 0");
    Mqtt_send(String(config.IDXdimmer),"0");  
    
  }
  else {
    Serial.println("MQTT_init : /! ECHEC !/ ");
  }  

}


void Mqtt_HA_hello() {
String pvname = String("pvrouteur-") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17); 
String message = "{'device_class': 'power', 'name': '"+ pvname +"-power', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'unit_of_measurement', 'W', 'value_template': '{{ value_json.power}}' }"; 
String topic = "homeassistant/sensor/"+ pvname +"/power/config";

if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO routeur");}

message = "{'device_class': 'power', 'name': '"+ pvname +"-dimmer', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'unit_of_measurement': '%', 'value_template': '{{ value_json.dimmer}}' }"; 
topic = "homeassistant/sensor/"+ pvname +"/dimmer/config";
if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO dimmer");}
message = "{'device_class': 'temperature', 'name': '"+ pvname +"-temp', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'unit_of_measurement': '°C', 'value_template': '{{ value_json.temperature}}' }"; 
topic = "homeassistant/sensor/"+ pvname +"/temperature/config";
if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO temp");}
Serial.println (pvname);
//Serial.println (message);
//Serial.println (topic);
//if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO");}
}



#endif