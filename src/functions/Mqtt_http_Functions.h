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


String node_mac = WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
// String node_ids = WiFi.macAddress().substring(0,2)+ WiFi.macAddress().substring(4,6)+ WiFi.macAddress().substring(8,10) + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17);
String node_id = String("PvRouter-") + node_mac; 
String topic = "homeassistant/sensor/"+ node_id +"/status";  
String topic_Xlyric = "Xlyric/"+ node_id +"/";

String command_switch = String(topic_Xlyric + "switch/command");
// String command_number = String(topic_Xlyric + "number/command");
// String command_select = String(topic_Xlyric + "select/command");
// String command_button = String(topic_Xlyric + "button/command");

void callback(char* Subscribedtopic, byte* message, unsigned int length);
// void Mqtt_HA_hello(); // non utilisé maintenant 
void reconnect();
/***
 *  reconnexion au serveur MQTT
 */

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("-----------------------------");
    Serial.println("Attempting MQTT reconnection...");
    // Attempt to connect

    if (client.connect(node_id.c_str(), configmqtt.username, configmqtt.password, String(topic_Xlyric +"status").c_str(), 2, true, "offline", false)) {       //Connect to MQTT server
      client.publish(String(topic_Xlyric +"status").c_str(), "online", true);         // Once connected, publish online to the availability topic
      client.subscribe(command_switch.c_str());
      client.loop();

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
*    Fonction d'envoie info MQTT
*/

void Mqtt_send_DOMOTICZ ( String idx, String value, String otherpub = "" ) {
  
  String nvalue = "0" ; 
  
  if ( value != "0" ) { 
      nvalue = "2" ; 
      }
  
  String message; 
  if (otherpub == "" ) {
    message = "  { \"idx\" : " + idx +" ,   \"svalue\" : \"" + value + "\",  \"nvalue\" : " + nvalue + "  } ";
  }

  // String jdompub = String(config.Publish) + "/"+idx ;
  // if (otherpub != "" ) {jdompub += "/"+otherpub; }
  


  //client.loop();
    if (otherpub == "" ) {
      if (client.publish(config.Publish, String(message).c_str(), true)) {
     //   Serial.println("MQTT_send : MQTT sent to domoticz");
      }

      else {
        Serial.println("MQTT_send : error publish to domoticz ");
      }
    }
//   if (client.publish(jdompub.c_str() , value.c_str(), true)){
//   //  Serial.println("MQTT_send : MQTT sent to Jeedom ");
//   }
//   else {
// Serial.println("MQTT_send : error publish to Jeedom ");
//   }
  

}

/*
Fonction MQTT callback
*
*/


void callback(char* Subscribedtopic, byte* message, unsigned int length) {

  StaticJsonDocument<64> doc2;
  deserializeJson(doc2, message);
  if (strcmp( Subscribedtopic, command_switch.c_str() ) == 0) { 
    if (doc2.containsKey("Switch1")) { 
        int relay = doc2["Switch1"]; 
        if ( relay == 0) { digitalWrite(RELAY1 , LOW); }
        else { digitalWrite(RELAY1 , HIGH); } 
        logging.start += "RELAY1 at " + String(relay) + "\r\n"; 
        switch_1.send(String(relay));
    }
    if (doc2.containsKey("Switch2")) { 
        int relay = doc2["Switch2"]; 
        if ( relay == 0) { digitalWrite(RELAY2 , LOW); }
        else { digitalWrite(RELAY2 , HIGH); } 
        logging.start += "RELAY2 at " + String(relay) + "\r\n"; 
        switch_2.send(String(relay));
    }
  } 
}

/*
*    Fonction d'init de MQTT 
*/

void Mqtt_init() {
  // String pvname = String("PvRouter-") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17); 
  // String topic = "homeassistant/sensor/"+ pvname +"/status";

  // debug
  Serial.println("MQTT_init : server="+String(config.mqttserver));
  Serial.println("MQTT_init : port="+String(config.mqttport));
  
  client.setServer(config.mqttserver, config.mqttport);
  client.setCallback(callback);
  Serial.print("MQTT_init : connexion...");
  reconnect();
  // if (client.connect(pvname,configmqtt.username, configmqtt.password, topic.c_str(), 2, true, "offline")) {       //Connect to MQTT server
  //   client.publish(topic.c_str(), "online", true);         // Once connected, publish online to the availability topic
  //   Serial.println("MQTT_init : connecte a MQTT... Initialisation dimmer à 0");
  //   if (configmqtt.DOMOTICZ){ Mqtt_send_DOMOTICZ(String(config.IDXdimmer),"0"); }
    
  // }
  // else {
  //   Serial.println("MQTT_init : /! ECHEC !/ ");
  // }  

}


// void Mqtt_HA_hello() {
// String pvname = String("pvrouteur-") + WiFi.macAddress().substring(12,14)+ WiFi.macAddress().substring(15,17); 
// String message = "{'device_class': 'power', 'name': '"+ pvname +"-power', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'unit_of_measurement', 'W', 'value_template': '{{ value_json.power}}' }"; 
// String topic = "homeassistant/sensor/"+ pvname +"/power/config";

// if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO routeur");}

// message = "{'device_class': 'power', 'name': '"+ pvname +"-dimmer', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'unit_of_measurement': '%', 'value_template': '{{ value_json.dimmer}}' }"; 
// topic = "homeassistant/sensor/"+ pvname +"/dimmer/config";
// if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO dimmer");}
// message = "{'device_class': 'temperature', 'name': '"+ pvname +"-temp', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'unit_of_measurement': '°C', 'value_template': '{{ value_json.temperature}}' }"; 
// topic = "homeassistant/sensor/"+ pvname +"/temperature/config";
// message = "{'device_class': 'switch', 'name': '"+ pvname +"-switch', 'state_topic': 'homeassistant/sensor/"+ pvname +"/state', 'value_template': '{{ value_json.switch}}' }"; 
// topic = "homeassistant/sensor/"+ pvname +"/switch/config";
// if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO temp");}
// Serial.println (pvname);
// //Serial.println (message);
// //Serial.println (topic);
// //if (client.publish(topic.c_str(), String(message).c_str(), true))  {  Serial.println("HELLO");}
// }



#endif