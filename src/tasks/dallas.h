#ifndef TASK_DALLAS
#define TASK_DALLAS

    #include <Arduino.h>
    #include "../config/config.h"
    #include "../config/enums.h"
    #include "../functions/dallasFunction.h"
    #include "mqtt-home-assistant.h"
   

extern DisplayValues gDisplayValues;
extern Dallas dallas ;
#ifndef LIGHT_FIRMWARE
    extern HA temperature_HA;
#endif

/**
 * Task: Lecture de la sonde de température Dallas toute les 10s
 * 
  */
extern Memory task_mem; 

void dallasread(void * parameter){
  for (;;){
    if (dallas.detect) {
    float Old_temperature = gDisplayValues.temperature;
    gDisplayValues.temperature = CheckTemperature("Inside : ", dallas.addr); 

      // réduction de la précision de la température
      //char buffer[5];
      //dtostrf(gDisplayValues.temperature,2, 1, buffer); // conversion en n.1f 
      //gDisplayValues.temperature=buffer;

    /// pour éviter le spam dans les logs
    if (Old_temperature != gDisplayValues.temperature) {
      Serial.print(" Temp C: ");
      Serial.println(gDisplayValues.temperature);
      logging.Set_log_init("temp :");
      logging.Set_log_init(String(gDisplayValues.temperature).c_str());
      logging.Set_log_init("\r\n");
    }

   } 
   task_mem.task_dallas_read = uxTaskGetStackHighWaterMark(nullptr);
   // Sleep for 5 seconds, avant de refaire une analyse
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
#endif