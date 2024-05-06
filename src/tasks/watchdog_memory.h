#ifndef TASK_WATCHDOG_MEMORY
#define TASK_WATCHDOG_MEMORY

#include <Arduino.h>
#include "../config/enums.h"
#include "../config/config.h"
#include "HTTPClient.h"

extern Memory task_mem; 
extern Logs logging;

void watchdog_memory(void * parameter){ // NOSONAR
  for (;;){
    bool need_reboot = false;

    // si valeur mémoire en dessous de 500 alors alerte et en dessous de 100 reboot
    if (task_mem.task_keepWiFiAlive2 < 100) {
      need_reboot = true; 
    }

    if (task_mem.task_serial_read_task < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_dallas_read < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_updateDimmer < 500) {
        need_reboot = true; 
    }

    if (task_mem.task_GetDImmerTemp < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_measure_electricity < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_send_mqtt < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_switchDisplay < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_updateDisplay < 500) {
      need_reboot = true; 
    }

    if (task_mem.task_loop < 500) {
      need_reboot = true; 
    }
     
    // si need  reboot alors reboot
    if (need_reboot) {
      savelogs("-- reboot watchdog memoire -- ");
      ESP.restart();
    } 
    // Sleep for 10 seconds, avant de refaire une analyse

    vTaskDelay(pdMS_TO_TICKS(20000));
  }
}

#endif