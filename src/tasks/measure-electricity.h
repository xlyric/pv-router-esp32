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
#include "functions/shelly.h" 

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

      //// recherche du mode de fonctionnement
      int mode = 0;   /// 0 = porteuse  ; 1 = shelly , 2 = enphase 3 = fronius
      if (strcmp(config.topic_Shelly,"none") != 0) {
            mode = 1; 
      }
      else if (configmodule.enphase_present && String(configmodule.envoy) == "S") {
            mode = 2; 
      }
      else if (configmodule.Fronius_present) {
            mode = 3; 
      }

            /// SCT 013 
      if (mode == 0 ) { 
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

      /// que dans les cas sans mode AP
      if (!AP) {
            /// shelly
            if (mode == 1 ) { 
                  if (WiFi.status() == WL_CONNECTED )  {
                        /// on vérifie si config.topic_Shelly est une IP ou un topic mqtt
                        if (checkIP(config.topic_Shelly)) {
                              gDisplayValues.watt = shelly_get_data(config.topic_Shelly);
                        }
                        #ifdef NORMAL_FIRMWARE
                        else {
                              
                              client.loop();
                              gDisplayValues.watt = gDisplayValues.Shelly ;
                            
                        }
                        #endif
                        //  // on met à jour
                        gDisplayValues.porteuse = true; // et c'est bon. 

                  }
            }

            /// enphase
            if (mode == 2 ) { 
                  if (WiFi.status() == WL_CONNECTED )  {
                        //// inversion des valeurs pour enphase piloteur
                        Enphase_get();
                        int tempo = gDisplayValues.watt; 
                        gDisplayValues.watt = gDisplayValues.Fronius_conso ; 
                        gDisplayValues.Fronius_conso = tempo; 
                  }

            }


            /// fronius
            if (mode == 3 ) { 
                  if (WiFi.status() == WL_CONNECTED )  {
                        Fronius_get();
                  }
            }
      }





// shelly quand c était en mqtt
      #ifdef NORMAL_FIRMWARE

      #endif



long end = millis();
      task_mem.task_measure_electricity = uxTaskGetStackHighWaterMark(NULL);
      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took) ///&& configmodule.pilote
      if (mode != 0 ) {
            vTaskDelay(pdMS_TO_TICKS(5000));
      }
      else
      {      
            vTaskDelay(pdMS_TO_TICKS(3000));
      }

    }    
}

#endif
