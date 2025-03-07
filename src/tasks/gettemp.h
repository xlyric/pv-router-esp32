#ifndef TASK_GET_TEMP
#define TASK_GET_TEMP

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#include "HTTPClient.h"

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"

//***********************************
//************* Variables externes
//***********************************
extern DisplayValues gDisplayValues;
extern Config config; 
extern Dallas dallas ;
extern Memory task_mem; 
extern SemaphoreHandle_t mutex;

//***********************************
//************* Variables locales
//***********************************
HTTPClient httpdimmer;

//***********************************
//************* GetDImmerTemp
//***********************************
void GetDImmerTemp(void * parameter){
 for (;;) { 
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {  // Prend le mutex
      // create get request 
      if (WiFi.status() != WL_CONNECTED) {   /// si pas de connexion Wifi test dans 10 s 
        vTaskDelay(10*1000 / portTICK_PERIOD_MS);
        continue;
      }

      if ( !dallas.detect && ( String(config.dimmer) != "" || String(config.dimmer) != "none")) {
        const String baseurl = "/state" ; 
        httpdimmer.begin(String(config.dimmer),80,baseurl);   
        int httpResponseCode = httpdimmer.GET();
        String dimmerstate = "0"; 

        //  read request return
        if (httpResponseCode>0) {
          if (logging.serial){
            Serial.print("gettemp HTTP Response code: ");
            Serial.println(httpResponseCode);
          }
          dimmerstate = httpdimmer.getString();
        }
        else {
          Serial.print("Get Dimmer temp : Error code: ");
          Serial.println(httpResponseCode);
        }
        
        // Free resources
        httpdimmer.end();

        if (httpResponseCode>400) { 
          gDisplayValues.temperature = 0;   
        }
        else { 
          // hash temp 
          JsonDocument doc;
          DeserializationError error = deserializeJson(doc, dimmerstate);

          if (error) {
            Serial.print(F("gettemp() failed: "));
            logging.Set_log_init("gettemp() failed: \r\n",true);
            Serial.println(error.c_str());
            gDisplayValues.temperature = gDisplayValues.temperature;
            vTaskDelay(10*1000 / portTICK_PERIOD_MS);
            continue;
          }
          gDisplayValues.temperature = doc["temperature"].as<String>().toFloat();          
        }

        if (logging.serial){
          Serial.println("temperature:" + String(gDisplayValues.temperature));
        }
      }

      xSemaphoreGive(mutex);  // Libère le mutex
      task_mem.task_GetDImmerTemp = uxTaskGetStackHighWaterMark(nullptr);
    }

    // refresh every GETTEMPREFRESH seconds 
    vTaskDelay(pdMS_TO_TICKS(15000+(esp_random() % 61) - 30));
  }
}

#endif
