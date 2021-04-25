#ifndef TASK_UPDATE_DISPLAY
#define TASK_UPDATE_DISPLAY

#include <Arduino.h>
#include "SSD1306Wire.h"
#include "functions/drawFunctions.h"
#include "functions/appweb.h"
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
    //display.clear();


#if WIFI_ACTIVE == true
    if(gDisplayValues.currentState == UP){
      drawTime();
      drawIP();
    }
    else { 
    drawtext10(64 ,0, "no Wifi");
    }

#endif

      // Affichage de l'état de la régulation
      drawtext10(64,16, injection_type() );
      
      // Affichage des infos de puissance ( sans les virgules )
      if ( gDisplayValues.porteuse == false) { 
      drawtext16(64,30, String("abs porteuse"));
      gDisplayValues.dimmer = 0 ; /// mise à zero du dimmer par sécurité 
      }
      else  {
      drawtext16(64 ,30, String(gDisplayValues.watt,0) + " W");
      }

      // Affichage des infos du dimmer
      drawtext16(64,48, String(gDisplayValues.dimmer) + " %");
      


    display.display();

    // Sleep for 5 seconds, then update display again!
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}


/**
 * Metafunction that takes care of drawing all the different
 * parts of the display (or not if it's turned off).
 */
/*
void updateTimeDisplay(void * parameter){
  for (;;){
    String ActualTime = timeClient.getFormattedTime();

    drawtext10(0,0,ActualTime );
    display.display();

    // Sleep for 5 seconds, then update display again!
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
*/
#endif
