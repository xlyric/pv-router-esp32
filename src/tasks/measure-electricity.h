#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-aws.h"
#include "mqtt-home-assistant.h"
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
//extern EnergyMonitor emon1;

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
            injection2();
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

                  Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)));  
                  device_routeur.send(String(gDisplayValues.watt));
                  // send if injection
                  if (gDisplayValues.watt < 0 ){
                  Mqtt_send(String(config.IDX), String(int(-gDisplayValues.watt)),"injection");
                  Mqtt_send(String(config.IDX), String("0") ,"grid");
                  device_inject.send(String(int(-gDisplayValues.watt)));
                  device_grid.send(String("0"));
                  WHtempgrid += wattheure; 
                  compteur_inject.send(String(WHtempgrid));
                  
                  
                  compteur_grid.send(String("0"));
                  }
                  else {
                  Mqtt_send(String(config.IDX), String("0"),"injection");
                  Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)),"grid");
                  device_grid.send(String(int(gDisplayValues.watt)));
                  device_inject.send(String("0"));
                  compteur_inject.send(String("0"));
                  WHtempinject += wattheure;
                  compteur_grid.send(String(WHtempinject));
                  
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
            vTaskDelay(5500 / portTICK_PERIOD_MS);
      }
      else
      {      
            vTaskDelay((1500-(end-start)) / portTICK_PERIOD_MS);
      }

    }    
}

#endif
