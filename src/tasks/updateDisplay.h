#ifndef TASK_UPDATE_DISPLAY
#define TASK_UPDATE_DISPLAY

#include <Arduino.h>

#include "../config/config.h"
#include "functions/appweb.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"
#include "functions/display.h"

#ifdef DEVKIT1
#include "SSD1306Wire.h"
extern SSD1306Wire display;
#endif

#ifdef TTGO
#include <TFT_eSPI.h>
extern TFT_eSPI display;
#endif

extern DisplayValues gDisplayValues;
extern Configmodule configmodule;

#define TFT_PIN 4

/**
 * Metafunction that takes care of drawing all the different
 * parts of the display (or not if it's turned off).
 */
extern Memory task_mem; 

void updateDisplay(void* parameter) {
  for (;;) {
     if ( gDisplayValues.option == 0 && digitalRead(TFT_PIN)==HIGH){
      call_display();
      }
      task_mem.task_updateDisplay = uxTaskGetStackHighWaterMark(nullptr);
      // Sleep for 5 seconds, then update display again!
      vTaskDelay(pdMS_TO_TICKS(4000));
  } // for
} // updateDisplay




#endif
