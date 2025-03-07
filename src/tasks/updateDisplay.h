#ifndef TASK_UPDATE_DISPLAY
#define TASK_UPDATE_DISPLAY

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#ifdef ESP32D1MINI_FIRMWARE
  #include "SSD1306Wire.h"
#endif
#ifdef TTGO
  #include <TFT_eSPI.h>
#endif

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/config.h"
#include "functions/appweb.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"
#include "functions/display.h"

//***********************************
//************* Variables externes
//***********************************
#ifdef ESP32D1MINI_FIRMWARE
  extern SSD1306Wire display;
#endif
#ifdef TTGO
  extern TFT_eSPI display;
#endif
extern DisplayValues gDisplayValues;
extern Configmodule configmodule;
extern Memory task_mem; 

//***********************************
//************* Constante
//***********************************
#define TFT_PIN 4

//***********************************
//************* updateDisplay
//***********************************
void updateDisplay(void* parameter) {
  /**
   * Metafunction that takes care of drawing all the different
   * parts of the display (or not if it's turned off).
   */
  for (;;) {
    #ifdef TTGO
     if ( gDisplayValues.option == 0 && digitalRead(TFT_PIN)==HIGH) {
      call_display();
      }
    #elif ESP32D1MINI_FIRMWARE
      // Serial.println("Update display");
      ui.update();
      // Serial.println("Update display after");
    #endif
    
    task_mem.task_updateDisplay = uxTaskGetStackHighWaterMark(nullptr);
    // Sleep for 5 seconds, then update display again!
    vTaskDelay(pdMS_TO_TICKS(4000+(esp_random() % 61) - 30));
  } // for
} // updateDisplay

#endif
