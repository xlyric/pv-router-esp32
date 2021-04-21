#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>
#include "EmonLib.h"

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-aws.h"
#include "mqtt-home-assistant.h"
#include "functions/energyFunctions.h"
#include "functions/dimmerFunction.h"
#include "functions/drawFunctions.h"

extern DisplayValues gDisplayValues;
extern EnergyMonitor emon1;


int Pow_mqtt_send = 0;

void measureElectricity(void * parameter)
{
    for(;;){
    //  serial_println("[ENERGY] Measuring...");
       /// vérification qu'une autre task ne va pas fausser les valeurs
      long start = millis();

     
      double amps = emon1.calcIrms(1480);
      double watts = amps * HOME_VOLTAGE;

      //injection();
      //if ( gDisplayValues.injection == true ) { serial_print("-") ; }
      gDisplayValues.watt = watts;
      serial_println(int(watts)) ;

      //serial_println(gDisplayValues.injection) ;
     
     
      long end = millis();
     
#if WIFI_ACTIVE == true
      Pow_mqtt_send ++ ;
      if ( Pow_mqtt_send > 10 ) {
        Mqtt_send(config.IDX, String(int(gDisplayValues.watt)));  
        Pow_mqtt_send = 0 ;
      }
#endif
        
  


      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      vTaskDelay((1000-(end-start)) / portTICK_PERIOD_MS);
    }    
}

#endif
