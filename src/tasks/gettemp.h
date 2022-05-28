#ifndef TASK_GET_TEMP
#define TASK_GET_TEMP

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
#include "../functions/Mqtt_http_Functions.h"


HTTPClient httpdimmer;

extern DisplayValues gDisplayValues;
extern Config config; 

void GetDImmerTemp(void * parameter){
 for (;;){ 
// create get request 
    String baseurl; 
      baseurl = "/state" ; 
      httpdimmer.begin(String(config.dimmer),80,baseurl);   
      int httpResponseCode = httpdimmer.GET();

      String dimmerstate = ""; 

//  read request return
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    dimmerstate = httpdimmer.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  httpdimmer.end();


// hash temp 
int starttemp = dimmerstate.indexOf(";"); 
dimmerstate = dimmerstate.substring(starttemp+1); 

gDisplayValues.temperature = dimmerstate; 
 Serial.println("temperature " + dimmerstate);

if ( config.dimmerlocal ){

}



// refresh every GETTEMPREFRESH seconds 
vTaskDelay(GETTEMPREFRESH * 1000 / portTICK_PERIOD_MS);
  }

}


#endif



