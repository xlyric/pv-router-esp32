#ifndef TASK_UPDATE_DISPLAY
#define TASK_UPDATE_DISPLAY

#include <Arduino.h>
#include "SSD1306Wire.h"
#include "functions/drawFunctions.h"
#include "../config/config.h"

extern SSD1306Wire display;
extern DisplayValues gDisplayValues;

/**
 * Metafunction that takes care of drawing all the different
 * parts of the display (or not if it's turned off).
 */
void updateDisplay(void * parameter){
  for (;;){
    serial_println(F("lcd task"));
    //serial_println(F("[LCD] Updating..."));
    display.clear();

#if WIFI_ACTIVE == true
    if(gDisplayValues.currentState == UP){
      drawTime();
      drawIP();
    }
#endif
        
      drawAmpsWatts();
      


    display.display();

    // Sleep for 5 seconds, then update display again!
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

#endif
