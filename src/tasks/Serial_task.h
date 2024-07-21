#ifndef TASK_SERIAL
#define TASK_SERIAL

#include <Arduino.h>

#include "../config/enums.h"
#include "../functions/appweb.h"

extern Configwifi configwifi; 
extern Memory task_mem; 

void serial_read_task(void * parameter){ // NOSONAR
  for (;;){
    const int timeout_task = 36; /// timeout à 2 minutes

    if (gDisplayValues.serial_timeout < timeout_task)   { 
        serial_read();
        gDisplayValues.serial_timeout ++ ;
        
    }
    if (gDisplayValues.serial_timeout == timeout_task) {
      // remonté comme quoi le service serial est arrêté
      logging.Set_log_init(Serial_service_stopped);
      Serial.println(Serial_service_stopped);
      gDisplayValues.serial_timeout ++ ;
    }

    logging.clean_log_init();

   // Sleep for 5 seconds, avant de refaire une analyse

    task_mem.task_serial_read_task = uxTaskGetStackHighWaterMark(NULL);
    if (AP) { vTaskDelay(pdMS_TO_TICKS(3000)); }
    else { vTaskDelay(pdMS_TO_TICKS(5000)); }
    
  }
}

#endif