#ifndef TASK_SERIAL
#define TASK_SERIAL

#include <Arduino.h>

#include "../config/enums.h"
#include "../functions/appweb.h"

extern Configwifi configwifi; 

void serial_read_task(void * parameter){
  for (;;){
        serial_read();

   // Sleep for 5 seconds, avant de refaire une analyse
    //vTaskDelay(5000 / portTICK_PERIOD_MS);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

#endif