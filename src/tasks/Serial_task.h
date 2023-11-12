#ifndef TASK_SERIAL
#define TASK_SERIAL

#include <Arduino.h>

#include "../config/enums.h"
#include "../functions/appweb.h"

extern Configwifi configwifi; 
extern Memory task_mem; 

void serial_read_task(void * parameter){
  for (;;){
    const int timeout_task = 24; /// timeout à 2 minutes

    if (gDisplayValues.serial_timeout < timeout_task)   {
        serial_read();
        gDisplayValues.serial_timeout ++ ;
        
    }
    if (gDisplayValues.serial_timeout == timeout_task) {
      // remonté comme quoi le service serial est arrêté
      logging.Set_log_init("Serial service stopped\r\n");
      Serial.println("Serial service stopped");
      gDisplayValues.serial_timeout ++ ;
    }



   // Sleep for 5 seconds, avant de refaire une analyse
    //vTaskDelay(5000 / portTICK_PERIOD_MS);
    task_mem.task_serial_read_task = uxTaskGetStackHighWaterMark(NULL);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

#endif