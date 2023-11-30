#ifndef TASK_GET_TEMP
#define TASK_GET_TEMP

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"
#include "HTTPClient.h"

HTTPClient httpdimmer;

extern DisplayValues gDisplayValues;
extern Config config; 
extern Dallas dallas ;
extern Memory task_mem; 

void GetDImmerTemp(void * parameter){
 for (;;){ 
// create get request 
            if(WiFi.status() != WL_CONNECTED){   /// si pas de connexion Wifi test dans 10 s 
                vTaskDelay(10*1000 / portTICK_PERIOD_MS);
                continue;
            }

if ( !dallas.detect && String(config.dimmer) != "") {
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
  if (httpResponseCode>400) { gDisplayValues.temperature = 0;   }
  else { 
    // hash temp 
    DynamicJsonDocument doc(128);
    DeserializationError error = deserializeJson(doc, dimmerstate);
    if (error) {
      Serial.print(F("gettemp() failed: "));
      logging.Set_log_init("gettemp() failed: ",true);
      Serial.println(error.c_str());
      gDisplayValues.temperature = gDisplayValues.temperature;
      vTaskDelay(10*1000 / portTICK_PERIOD_MS);
      continue;
    }
    gDisplayValues.temperature = doc["temperature"].as<String>().toFloat();
    /* int starttemp = dimmerstate.indexOf(";"); 
      dimmerstate = dimmerstate.substring(starttemp+1);
      int lasttemp = dimmerstate.indexOf(";"); 
      //dimmerstate[lasttemp] = '\0'; 
      dimmerstate = dimmerstate.substring(0,lasttemp);

      gDisplayValues.temperature = dimmerstate; */
  }

  if (logging.serial){
    Serial.println("temperature:" + String(gDisplayValues.temperature));
  }
}
task_mem.task_GetDImmerTemp = uxTaskGetStackHighWaterMark(NULL);

// refresh every GETTEMPREFRESH seconds 
vTaskDelay(pdMS_TO_TICKS(15000));
  }

}

#endif