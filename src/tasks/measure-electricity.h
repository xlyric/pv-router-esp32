#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
// #include "mqtt-home-assistant.h"
#include "functions/energyFunctions.h"
#include "functions/dimmerFunction.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"

extern DisplayValues gDisplayValues;
extern Configmodule configmodule; 
extern Logs Logging;
extern HA device_routeur; 
extern HA device_grid; 
extern HA device_inject; 
extern HA compteur_inject;
extern HA compteur_grid;
extern HA temperature_HA;
extern HA device_alarm_temp;

extern HA power_factor;
extern HA power_vrms;
extern HA power_irms;
extern HA power_apparent;


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
      
      
      if ( configmodule.pilote == false ) {
            injection3();
            if ( gDisplayValues.porteuse == false ) {
                  gDisplayValues.watt =0 ; 
                  slowlog ++; 
                  if (slowlog == TEMPOLOG) { logging.start += "--> No sinus, check 12AC power \r\n"; slowlog =0 ; }

            }
            serial_println(int(gDisplayValues.watt)) ;

      }
      else{
            gDisplayValues.porteuse = true;
      }
     

if (!AP) {
      if (configmodule.enphase_present ) {
            Enphase_get();
            if ( configmodule.pilote ) { 
                  //// inversion des valeurs pour enphase piloteur
                  int tempo = gDisplayValues.watt; 
                  gDisplayValues.watt = gDisplayValues.Fronius_conso ;
                  gDisplayValues.Fronius_conso = tempo; 
                  }
            }

      if (configmodule.Fronius_present ){
            Fronius_get();
            }           


            #if WIFI_ACTIVE == true
                  Pow_mqtt_send ++ ;
                  if ( Pow_mqtt_send > 5 ) {
                  long timemesure = start-beforetime;
                  float wattheure = (timemesure * abs(gDisplayValues.watt) / timemilli) ;  

                  if (config.IDX != 0) {Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)));  }
                  if (configmqtt.HA) {
                        device_routeur.send(String(int(gDisplayValues.watt)));
                        power_apparent.send(String(int(PVA)));
                        power_vrms.send(String(int(Vrms)));
                        power_irms.send(String(Irms));
                        power_factor.send(String(PowerFactor));
                  }
                  // send if injection
                  if (gDisplayValues.watt < 0 ){
                  if (config.IDX != 0) {
                        Mqtt_send(String(config.IDX), String(int(-gDisplayValues.watt)),"injection");
                        Mqtt_send(String(config.IDX), String("0") ,"grid");
                  }
                  if (configmqtt.HA) device_inject.send(String(int(-gDisplayValues.watt)));
                  if (configmqtt.HA) device_grid.send(String("0"));
                  WHtempgrid += wattheure; 
                  if (configmqtt.HA) compteur_inject.send(String(WHtempgrid));
                  
                  
                  // if (configmqtt.HA)compteur_grid.send(String("0"));
                  }
                  else {
                        if (config.IDX != 0) {
                              Mqtt_send(String(config.IDX), String("0"),"injection");
                              Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)),"grid");
                        }
                  if (configmqtt.HA) device_grid.send(String(int(gDisplayValues.watt)));
                  if (configmqtt.HA) device_inject.send(String("0"));
                  // if (configmqtt.HA) compteur_inject.send(String("0"));
                  WHtempinject += wattheure;
                  if (configmqtt.HA) compteur_grid.send(String(WHtempinject));
                  //maj 202030209
                  if (configmqtt.HA && discovery_temp) {
                        temperature_HA.send(String(gDisplayValues.temperature));
                        device_alarm_temp.send(stringbool(security));}

                  if (config.IDX != 0 && discovery_temp) {Mqtt_send(String("temperature"), String(gDisplayValues.temperature) );}
                  }

                  beforetime = start; 
                  Pow_mqtt_send = 0 ;
                  }             
            #endif
}

long end = millis();

      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      if (configmodule.enphase_present && configmodule.pilote) {
            vTaskDelay(3000 / portTICK_PERIOD_MS);
      }
      else
      {      
            vTaskDelay((1500-(end-start)) / portTICK_PERIOD_MS);
      }

    }    
}

#endif
