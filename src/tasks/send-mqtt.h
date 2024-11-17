#ifndef TASK_MQTT
#define TASK_MQTT

///// cette task servira à faire les remontés MQTT pour à terme les retirer des autres fonctions et éviter certains bugs 
#include <Arduino.h>
#include <WiFiClientSecure.h>
   
#include "../config/config.h"
#include "../config/enums.h"
#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-home-assistant.h"
#include "functions/Mqtt_http_Functions.h"
#include "functions/energyFunctions.h"

#ifndef LIGHT_FIRMWARE
  #include <PubSubClient.h>
  extern PubSubClient client;
#endif

extern DisplayValues gDisplayValues;
extern Config config;
extern Configmodule configmodule; 
extern Logs Logging;
extern Mqtt configmqtt;




#ifndef LIGHT_FIRMWARE
      extern HA device_routeur; 
      extern HA device_grid; 
      extern HA device_routed; // Ajout RV - 20230304
      extern HA device_dimmer; // Ajout RV - 20230304
      extern HA device_inject; 
      extern HA compteur_inject;
      extern HA compteur_grid;
      extern HA temperature_HA;

      extern HA power_factor;
            
      extern HA power_apparent;

#endif

int Pow_mqtt_send = 0;
float WHtempgrid;
float WHtempinject;
long beforetime; 
#define timemilli 3.6e+6 
extern Memory task_mem; 
// task to send data to MQTT
void send_to_mqtt(void * parameter){ // NOSONAR
  for (;;){
            if(!WiFi.isConnected()){   /// si pas de connexion Wifi test dans 10 s 
                vTaskDelay(10*1000 / portTICK_PERIOD_MS);
                continue;
            }

    /// vérification que l'envoie mqtt est souhaité et les connexions actives
    #ifndef LIGHT_FIRMWARE
    if (config.mqtt && (WiFi.status() == WL_CONNECTED ) && client.connected() ){
    #else
      if (config.mqtt && (WiFi.status() == WL_CONNECTED ) ){
    #endif
        long start = millis();
    
         #if WIFI_ACTIVE == true
                  Pow_mqtt_send ++ ;
                  if ( Pow_mqtt_send > 5 ) {
                  long timemesure = start-beforetime;
                  float wattheure = (timemesure * abs(gDisplayValues.watt) / timemilli) ;  
            #ifndef LIGHT_FIRMWARE
             // domoticz et jeedom
                  if (config.IDX != 0 ) {Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)),"","watt");  }
                  if (config.IDXdallas != 0) {Mqtt_send(String(config.IDXdallas), String(gDisplayValues.temperature),"","Dallas" ); }//  bug#11  remonté domoticz 
            // HA
                  if (configmqtt.HA) {
                        device_routeur.send(String(int(gDisplayValues.watt)));
                        device_routed.send(String(gDisplayValues.puissance_route));
                        device_dimmer_power.send(String(int((unified_dimmer.get_power()) * config.charge/100)));
                        power_apparent.send(String(int(PVA)));
                      
                        power_factor.send(String(PowerFactor));

                        temperature_HA.send(String(gDisplayValues.temperature));
                        device_dimmer.send(String(int(unified_dimmer.get_power()))); // Modif RV - pour être plus en accord avec le nommage sur les dimmers
                        switch_relay1.send(String(digitalRead(RELAY1)));
                        switch_relay2.send(String(digitalRead(RELAY2)));
                        
                  }

                    /// remonté énergie domoticz et jeedom
                  // send if injection
                  if (gDisplayValues.watt < 0 ){
                        if (config.IDX != 0 && config.mqtt) {
                              Mqtt_send(String(config.IDX), String(int(-gDisplayValues.watt)),"injection","Reseau");
                              Mqtt_send(String(config.IDX), String("0") ,"grid","Reseau");
                        }
                        if (configmqtt.HA) {
                        device_inject.send(String(int(-gDisplayValues.watt)));
                        device_grid.send(String("0"));
                        WHtempgrid += wattheure; 
                        compteur_inject.send(String(WHtempgrid));
                            //envoie vers mqtt des état injection et consommation 
    
                        client.publish(("memory/"+compteur_grid.topic+compteur_grid.Get_name()).c_str(), String(WHtempgrid).c_str(),true); 
                      
                        compteur_grid.send(String("0"));
                        }
                  }
                  else {
                        if (config.IDX != 0 && config.mqtt) {
                              Mqtt_send(String(config.IDX), String("0"),"injection","Reseau");
                              Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)),"grid","Reseau");
                        }
                        if (configmqtt.HA) {
                        device_grid.send(String(int(gDisplayValues.watt)));
                        device_inject.send(String("0"));
                        compteur_inject.send(String("0"));
                        WHtempinject += wattheure;
                        compteur_grid.send(String(WHtempinject));
                        client.publish(("memory/"+compteur_inject.topic+compteur_inject.Get_name()).c_str(), String(WHtempinject).c_str(),true);
                        }

                  }
                  //maj 202030209
                  
            #endif
                  beforetime = start; 
                  Pow_mqtt_send = 0 ;
                  }
      #endif   
      } 
      task_mem.task_send_mqtt = uxTaskGetStackHighWaterMark(NULL);
   // Sleep for 10 seconds
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}



#endif