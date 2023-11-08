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

extern DisplayValues gDisplayValues;
extern Config config;
extern Configmodule configmodule; 
extern Logs Logging;
extern Mqtt configmqtt;


#ifndef LIGHT_FIRMWARE
      extern HA device_routeur; 
      extern HA device_grid; 
      extern HA device_inject; 
      extern HA compteur_inject;
      extern HA compteur_grid;
      extern HA temperature_HA;

      extern HA power_factor;
      extern HA power_vrms;
      extern HA power_irms;
      extern HA power_apparent;
      extern HA enphase_cons_whLifetime;
      extern HA enphase_prod_whLifetime;
      extern HA enphase_current_power_consumption;
      extern HA enphase_current_power_production;

#endif

int Pow_mqtt_send = 0;
float WHtempgrid=0;
float WHtempinject=0;
long beforetime; 
#define timemilli 3.6e+6 
extern Memory task_mem; 

void send_to_mqtt(void * parameter){
  for (;;){
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
                        power_apparent.send(String(int(PVA)));
                        power_vrms.send(String(int(Vrms)));
                        power_irms.send(String(Irms));
                        power_factor.send(String(PowerFactor));
                        enphase_cons_whLifetime.send(String(int(gDisplayValues.enp_cons_whLifetime)));
                        enphase_prod_whLifetime.send(String(int(gDisplayValues.enp_prod_whLifetime)));
                        enphase_current_power_consumption.send(String(int(gDisplayValues.enp_current_power_consumption)));
                        enphase_current_power_production.send(String(int(gDisplayValues.enp_current_power_production)));
                        temperature_HA.send(String(gDisplayValues.temperature));
                        device_dimmer.send(String(gDisplayValues.puissance_route));
                        
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
    vTaskDelay(pdMS_TO_TICKS(1500));
  }
}



#endif