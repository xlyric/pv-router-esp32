#ifndef TASK_WATCHDOG_MEMORY
#define TASK_WATCHDOG_MEMORY

#include <Arduino.h>
#include "../config/enums.h"
#include "../config/config.h"
#include "HTTPClient.h"

extern Memory task_mem; 
extern Logs logging;
/*
//envoie http si la mémoire est trop basse
void send_alerte_http(String task_name){
    // envoie d'un message sur la page web https://ota.apper-solaire.org/alerte.html
    // pour signaler un problème de mémoire
    // envoie d'un message sur la page web https://ota.apper-solaire.org/alerte.html
    // pour signaler un problème de mémoire
    const String baseurl = "https://ota.apper-solaire.org/alerte.php?task=";
    baseurl += task_name;
   //envoie du message
    HTTPClient http;
    http.begin(baseurl);   
    int httpResponseCode = http.GET();
    http.end();
   /// stockage dans la log
    logging.Set_log_init("Alerte mémoire: ");
    logging.Set_log_init(task_name);
    logging.Set_log_init("\r\n");
}
*/

void watchdog_memory(void * parameter){
  for (;;){
    bool need_reboot = false;

    // si valeur mémoire en dessous de 500 alors alerte et en dessous de 100 reboot
    if (task_mem.task_keepWiFiAlive2 < 500) {
      //send_alerte_http("task_keepWiFiAlive2");
      if (task_mem.task_keepWiFiAlive2 < 100) { need_reboot = true; }
    }

    if (task_mem.task_serial_read_task < 500) {
      //send_alerte_http("task_serial_read_task");
      if (task_mem.task_serial_read_task < 100) { need_reboot = true; }
    }

    if (task_mem.task_dallas_read < 500) {
      //send_alerte_http("task_dallas_read");
      if (task_mem.task_dallas_read < 100) { need_reboot = true; }
    }

    if (task_mem.task_updateDimmer < 500) {
      //send_alerte_http("task_updateDimmer");
      if (task_mem.task_updateDimmer < 100) { need_reboot = true; }
    }

    if (task_mem.task_GetDImmerTemp < 500) {
      //send_alerte_http("task_GetDImmerTemp");
      if (task_mem.task_GetDImmerTemp < 100) { need_reboot = true; }
    }

    if (task_mem.task_measure_electricity < 500) {
      //send_alerte_http("task_measure_electricity");
      if (task_mem.task_measure_electricity < 100) { need_reboot = true; }
    }

    if (task_mem.task_send_mqtt < 500) {
      //send_alerte_http("task_send_mqtt");
      if (task_mem.task_send_mqtt < 100) { need_reboot = true; }
    }

    if (task_mem.task_switchDisplay < 500) {
     // send_alerte_http("task_switchDisplay");
      if (task_mem.task_switchDisplay < 100) { need_reboot = true; }
    }

    if (task_mem.task_updateDisplay < 500) {
     // send_alerte_http("task_updateDisplay");
      if (task_mem.task_updateDisplay < 100) { need_reboot = true; }
    }

    if (task_mem.task_loop < 500) {
     // send_alerte_http("task_loop");
      if (task_mem.task_loop < 100) { need_reboot = true; }
    }
     
    // si need  reboot alors reboot
    if (need_reboot) {
      savelogs(timeClient.getFormattedTime() +"-- reboot watchdog memoire -- ");
      ESP.restart();
    } 
    // Sleep for 10 seconds, avant de refaire une analyse

    vTaskDelay(pdMS_TO_TICKS(20000));
  }
}

#endif