#ifndef TASK_DALLAS
#define TASK_DALLAS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>

//***********************************
//************* PROGRAMME PVRPOUTER
//***********************************
#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/dallasFunction.h"
#include "mqtt-home-assistant.h"
   
//***********************************
//************* Variables externes
//***********************************
extern DisplayValues gDisplayValues;
extern Dallas dallas ;
extern Memory task_mem; 
extern SemaphoreHandle_t mutex;
#ifndef LIGHT_FIRMWARE
    extern HA temperature_HA;
#endif

//***********************************
//************* dallasread
//*************    Task: Lecture de la sonde de température Dallas toute les 10s
//***********************************
void dallasread(void * parameter){
  for (;;){
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      if (dallas.detect) {
        float Old_temperature = gDisplayValues.temperature;
        gDisplayValues.temperature = CheckTemperature("Inside : ", dallas.addr); 

        /// pour éviter le spam dans les logs
        if (Old_temperature != gDisplayValues.temperature) {
          Serial.print(" Temp C: ");
          Serial.println(gDisplayValues.temperature);
          logging.Set_log_init("temp :");
          logging.Set_log_init(String(gDisplayValues.temperature).c_str());
          logging.Set_log_init("\r\n");
        }
        xSemaphoreGive(mutex);  // Libère le mutex
      } 
    }

   
    task_mem.task_dallas_read = uxTaskGetStackHighWaterMark(nullptr);
    
    // Sleep for 5 seconds, avant de refaire une analyse

    vTaskDelay(pdMS_TO_TICKS(10000+(esp_random() % 61) - 30));
  }
}

#endif
