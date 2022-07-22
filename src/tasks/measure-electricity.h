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
extern Config config; 
//extern EnergyMonitor emon1;


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
                  if ( Pow_mqtt_send > 10 ) {
                  Mqtt_send(String(config.IDX), String(int(gDisplayValues.watt)));  
                  if ( (configmodule.enphase_present) || (configmodule.Fronius_present) ) {
                        Mqtt_send(String(config.IDXprod), String(gDisplayValues.Fronius_prod));  
                  }
                  Pow_mqtt_send = 0 ;
                  }
            #endif
}

long end = millis();

      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      if (configmodule.enphase_present && configmodule.pilote) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
      }
      else
      {      
            vTaskDelay((1000-(end-start)) / portTICK_PERIOD_MS);
      }

    }    
}

#endif
