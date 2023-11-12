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



int slowlog = TEMPOLOG - 1 ; 

extern Memory task_mem; 

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
                        if (slowlog == TEMPOLOG) {     
                              logging.Set_log_init("--> No sinus, check 12AC power \r\n");
                              slowlog =0 ; 
                        }

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
         if (client.connected() && (WiFi.status() == WL_CONNECTED ))  {
            if (strcmp(config.topic_Shelly,"none") != 0)   { 
            client.loop(); // on vérifie coté mqtt si nouvelle info
            gDisplayValues.watt = gDisplayValues.Shelly ;  // on met à jour
            gDisplayValues.porteuse = true; // et c'est bon. 
            }
         }
      #endif
///enphase
      if (configmodule.enphase_present ) {
            if (WiFi.status() == WL_CONNECTED )  {
                  Enphase_get();
            }
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
///fronius
      if (configmodule.Fronius_present ){
            if (WiFi.status() == WL_CONNECTED )  {
                  Fronius_get();
            }
      }           


}

long end = millis();
      task_mem.task_measure_electricity = uxTaskGetStackHighWaterMark(NULL);
      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took) ///&& configmodule.pilote
      if (configmodule.enphase_present) {
            vTaskDelay(pdMS_TO_TICKS(5000));
      }
      else
      {      
            vTaskDelay(pdMS_TO_TICKS(3000));
      }

    }    
}

#endif
