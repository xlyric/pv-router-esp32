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

void GetDImmerTemp(void * parameter){
 for (;;){ 
// create get request 

if ( !dallas.detect && String(config.dimmer) != "") {
  String baseurl; 
  baseurl = "/state" ; 
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
  if (httpResponseCode>400) { gDisplayValues.temperature = "0";   }
  else { 
    // hash temp 
    DynamicJsonDocument doc(64);
    deserializeJson(doc, dimmerstate);
    gDisplayValues.temperature = doc["temperature"].as<String>();
    /* int starttemp = dimmerstate.indexOf(";"); 
      dimmerstate = dimmerstate.substring(starttemp+1);
      int lasttemp = dimmerstate.indexOf(";"); 
      //dimmerstate[lasttemp] = '\0'; 
      dimmerstate = dimmerstate.substring(0,lasttemp);

      gDisplayValues.temperature = dimmerstate; */
  }

  if (logging.serial){
    Serial.println("temperature:" + gDisplayValues.temperature);
  }
}

// refresh every GETTEMPREFRESH seconds 
vTaskDelay(pdMS_TO_TICKS(15000));
  }

}

#endif