#ifndef TASK_UPDATE_DISPLAY
#define TASK_UPDATE_DISPLAY

#include <Arduino.h>
#include "functions/drawFunctions.h"
#include "functions/appweb.h"
#include "../config/config.h"

#ifdef  DEVKIT1
#include "SSD1306Wire.h"
extern SSD1306Wire display;
#endif

#ifdef  TTGO
#include <TFT_eSPI.h>
extern TFT_eSPI display;
#endif

extern DisplayValues gDisplayValues;


/**
 * Metafunction that takes care of drawing all the different
 * parts of the display (or not if it's turned off).
 */
void updateDisplay(void * parameter){
  for (;;){
    serial_println(F("lcd task"));
    #ifdef  DEVKIT1
      display.clear();
    #endif
    #ifdef  TTGO
     display.fillScreen(0);
    #endif


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
      #ifdef  DEVKIT1
        drawtext10(64,16, injection_type() );
      #endif
      #ifdef  TTGO
        drawtext10(60,16, injection_type() );
         display.setTextSize(1);
          display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextFont(4);
          display.setCursor(30, 48, 2);  display.print("Dimmer (%)");
          display.setCursor(150, 48, 2);  display.print(OLEDPOW);

      #endif

      // Affichage des infos de puissance ( sans les virgules )
      if ( gDisplayValues.porteuse == false) { 
      #ifdef  DEVKIT1
        drawtext16(64,30, String(OLEDNOSIN));
      #endif
      #ifdef  TTGO
        drawtext16(120,70, String(OLEDNOSIN) );
      #endif

      gDisplayValues.dimmer = 0 ; /// mise à zero du dimmer par sécurité 
      }
      else  {
      #ifdef  DEVKIT1
        drawtext16(64 ,30, String(gDisplayValues.watt,0) + " W");
      #endif
      #ifdef  TTGO
        
        String affiche = String(gDisplayValues.watt,0) ;
        if ( gDisplayValues.watt > config.delta ) { drawtext16TTGO(120,70, affiche,TFT_RED );}
        else if ( gDisplayValues.watt < config.deltaneg ) { drawtext16TTGO(120,70, affiche,TFT_BLUE );}
        else { drawtext16TTGO(120,70, affiche,TFT_GREEN );}
      #endif
      }

      // Affichage des infos du dimmer
      //drawtext16(64,48, String(gDisplayValues.dimmer) + " %");
      

    #ifdef  DEVKIT1
    // Affichage des infos du dimmer
      drawtext16(64,48, String(gDisplayValues.dimmer) + " %");
      display.display();
    #endif
    #ifdef  TTGO
      drawtext16TTGO(0,70, String(gDisplayValues.dimmer),TFT_GREEN );
    #endif

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
