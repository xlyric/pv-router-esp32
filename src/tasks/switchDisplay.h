#ifndef TASK_SWITCH_DISPLAY
#define TASK_SWITCH_DISPLAY

#include <Arduino.h>

#include "../config/config.h"

//#ifdef  TTGO
//#include <TFT_eSPI.h>
//extern TFT_eSPI display;

#define TFT_PIN 4 
extern DisplayValues gDisplayValues;
extern Config config; 

void switchDisplay(void * parameter){
int timer = millis();

for(;;){

if (digitalRead(TFT_PIN)==HIGH && config.ScreenTime !=0 ) { 
    if ( millis() > timer + config.ScreenTime*1000 ) {
      digitalWrite(TFT_PIN,LOW);
    }
}


if (digitalRead(SWITCH)==LOW || gDisplayValues.screenstate == HIGH ){ // if right button is pressed or HTTP call 
    if (digitalRead(TFT_PIN)==HIGH) {             // and the status flag is LOW
      gDisplayValues.screenstate = LOW ;      
      digitalWrite(TFT_PIN,LOW);     // and turn Off the OLED
      }                           // 
    else {                        // otherwise...
      gDisplayValues.screenstate = LOW ;
      digitalWrite(TFT_PIN,HIGH);      // and turn On  the OLED
      if (config.ScreenTime !=0 ) {
        timer = millis();
      }
    }
                  // wait a sec for the 
  }                               // hardware to stabilize


  vTaskDelay(500 / portTICK_PERIOD_MS);
  
  }
}

//#endif
#endif
