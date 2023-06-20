#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-home-assistant.h"
#include "functions/energyFunctions.h"
#include "functions/dimmerFunction.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"

extern DisplayValues gDisplayValues;
extern Configmodule configmodule; 
extern Logs Logging;

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

int slowlog = TEMPOLOG - 1 ; 
long beforetime; 
#define timemilli 3.6e+6 
float WHtempgrid=0;
float WHtempinject=0;

int Pow_mqtt_send = 0;

void measureElectricity(void * parameter)
{
    for(;;){
    //  serial_println("[ENERGY] Measuring...");
       /// vérification qu'une autre task ne va pas fausser les valeurs
      long start = millis();
      int porteuse; 
      /*if ( configmodule.enphase_present || configmodule.Fronius_present || strcmp(config.topic_Shelly,"none") != 0 ) {
            porteuse = false; || (String(configmodule.envoy) == "R")
      }*/ /// refaire des tests... 
      
      if ( configmodule.enphase_present == false && configmodule.Fronius_present == false ) {  ///correction Fred 230423--> marche pas 
            if (strcmp(config.topic_Shelly,"none") == 0 ) {
                  injection2();
                  if ( gDisplayValues.porteuse == false  && configmodule.enphase_present == false && configmodule.Fronius_present == false) {
                        gDisplayValues.watt =0 ; 
                        slowlog ++; 
                        if (slowlog == TEMPOLOG) {     logging.start  += loguptime(); logging.start +=  String("--> No sinus, check 12AC power \r\n"); slowlog =0 ; }

                  }
                  if (logging.serial){
                  serial_println(int(gDisplayValues.watt)) ;
                  }

            }
      }
      else{
            gDisplayValues.porteuse = true;

      }
     




if (!AP) {

// shelly 
      #ifdef NORMAL_FIRMWARE
            if (strcmp(config.topic_Shelly,"none") != 0)   { 
            client.loop(); // on vérifie coté mqtt si nouvelle info
            gDisplayValues.watt = gDisplayValues.Shelly ;  // on met à jour
            gDisplayValues.porteuse = true; // et c'est bon. 
            }
      #endif
///enphase
      if (configmodule.enphase_present ) {
            Enphase_get();
            //if ( configmodule.pilote ) { 
                  //// inversion des valeurs pour enphase piloteur
                  if (String(configmodule.envoy) == "S") {
                  int tempo = gDisplayValues.watt; 
                  gDisplayValues.watt = gDisplayValues.Fronius_conso ; 
                  gDisplayValues.Fronius_conso = tempo; }
                  else 
                  {  /// si c'est un modèle R, il ne fait pas les mesures. 
                  injection2();
                  }

              //    }
            }
///enphase
      if (configmodule.Fronius_present ){
            Fronius_get();
            }           


      #if WIFI_ACTIVE == true
                  Pow_mqtt_send ++ ;
                  if ( Pow_mqtt_send > 5 ) {
                  long timemesure = start-beforetime;
                  float wattheure = (timemesure * abs(gDisplayValues.watt) / timemilli) ;  
            #ifndef LIGHT_FIRMWARE
                  if (config.IDX != 0 && config.mqtt ) {Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)),"","watt");  }

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
                  }

                  // send if injection
                  if (gDisplayValues.watt < 0 ){
                        if (config.IDX != 0 && config.mqtt) {
                              Mqtt_send(String(config.IDX), String(int(-gDisplayValues.watt)),"injection","Reseau");
                              Mqtt_send(String(config.IDX), String("0") ,"grid","Reseau");
                        }
                        if (configmqtt.HA) device_inject.send(String(int(-gDisplayValues.watt)));
                        if (configmqtt.HA) device_grid.send(String("0"));
                        WHtempgrid += wattheure; 
                        if (configmqtt.HA) compteur_inject.send(String(WHtempgrid));
                        

                        if (configmqtt.HA)compteur_grid.send(String("0"));
                  }
                  else {
                        if (config.IDX != 0 && config.mqtt) {
                              Mqtt_send(String(config.IDX), String("0"),"injection","Reseau");
                              Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)),"grid","Reseau");
                        }
                        if (configmqtt.HA) device_grid.send(String(int(gDisplayValues.watt)));
                        if (configmqtt.HA) device_inject.send(String("0"));
                        if (configmqtt.HA) compteur_inject.send(String("0"));
                        WHtempinject += wattheure;
                        if (configmqtt.HA) compteur_grid.send(String(WHtempinject));

                  }
                  //maj 202030209
                  if (configmqtt.HA) temperature_HA.send(String(gDisplayValues.temperature));
                  Mqtt_send(String(config.IDXdallas), String(gDisplayValues.temperature),"","Dallas" ); //  bug#11  remonté domoticz
            #endif
                  beforetime = start; 
                  Pow_mqtt_send = 0 ;
                  }
      #endif
}

long end = millis();

      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took) ///&& configmodule.pilote
      if (configmodule.enphase_present) {
            vTaskDelay(pdMS_TO_TICKS(4000));
      }
      else
      {      
            vTaskDelay(pdMS_TO_TICKS(2000));
      }

    }    
}

#endif
