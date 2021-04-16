#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>
#include "EmonLib.h"

#include "../config/config.h"
#include "../config/enums.h"
#include "mqtt-aws.h"
#include "mqtt-home-assistant.h"
#include "functions/energyFunctions.h"

extern DisplayValues gDisplayValues;
extern EnergyMonitor emon1;
extern unsigned short measurements[];
extern unsigned char measureIndex;

void measureElectricity(void * parameter)
{
    for(;;){
    //  serial_println("[ENERGY] Measuring...");
      long start = millis();

      double amps = emon1.calcIrms(1480);
      double watts = amps * HOME_VOLTAGE;

      injection();
      if ( gDisplayValues.injection == true ) { watts = 0-watts ; }
      gDisplayValues.watt = watts;
      serial_println(watts) ;
      
      //measurements[measureIndex] = watts;
      //measureIndex++;

          measureIndex = 0;
      

      long end = millis();

      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      vTaskDelay((1000-(end-start)) / portTICK_PERIOD_MS);
    }    
}

#endif
