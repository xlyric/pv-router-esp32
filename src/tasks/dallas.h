#ifndef TASK_DALLAS
#define TASK_DALLAS

    #include <Arduino.h>
    #include "../config/config.h"
    #include "../config/enums.h"
    #include "../functions/dallasFunction.h"
    #include "mqtt-home-assistant.h"
   
extern DeviceAddress addr[MAX_DALLAS]; 
//extern float previous_celsius[MAX_DALLAS]; // température précédente
extern DisplayValues gDisplayValues;
extern Dallas dallas ;
//extern int deviceCount; // nombre de sonde(s) dallas détectée(s)

//extern HA devicetemp[MAX_DALLAS];
//#ifndef LIGHT_FIRMWARE
    extern HA temperature_HA[MAX_DALLAS];
//#endif

/**
 * Task: Lecture de la sonde de température Dallas toute les 10s
 * 
  */
extern Memory task_mem; 

void dallasread(void * parameter){
  for (;;){
    if (dallas.detect) {

    sensors.requestTemperatures();
    delay(400);
        for (int a = 0; a < dallas.deviceCount; a++) {
      dallas.celsius[a]=CheckTemperature("temp_" + devAddrNames[a],addr[a]);
      //gestion des erreurs DS18B20
      if ( (dallas.celsius[a] == -127.00) || (dallas.celsius[a] == -255.00) || (dallas.celsius[a] > 200.00) ) {
        dallas.celsius[a]=dallas.previous_celsius[a];
        dallas.dallas_error[a] ++; // incrémente le compteur d'erreur
        logging.Set_log_init("Dallas" + String(a) + " : échec "+ String(dallas.dallas_error[a]) + "\r\n",true);
          }
          else { 
        dallas.celsius[a] = (roundf(dallas.celsius[a] * 10) / 10 ) + 0.1; // pour les valeurs min
        gDisplayValues.temperature = dallas.celsius[a];
        dallas.dallas_error[a] = 0; // remise à zéro du compteur d'erreur
      }   
    }

         if (!dallas.discovery_temp) {
        dallas.discovery_temp = true;
          for (int i = 0; i < dallas.deviceCount; i++) {
            temperature_HA[i].discovery();
          }
        }

   } 
   task_mem.task_dallas_read = uxTaskGetStackHighWaterMark(NULL);
   // Sleep for 5 seconds, avant de refaire une analyse
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
#endif