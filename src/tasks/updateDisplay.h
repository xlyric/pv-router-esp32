#ifndef TASK_UPDATE_DISPLAY
#define TASK_UPDATE_DISPLAY

#include <Arduino.h>

#include "../config/config.h"
#include "functions/appweb.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"

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


/**
 * Metafunction that takes care of drawing all the different
 * parts of the display (or not if it's turned off).
 */
void updateDisplay(void* parameter) {
  for (;;) {
    serial_println(F("lcd task"));
Serial.print("IDXprod=");
Serial.println(config.IDXprod);
    #ifdef DEVKIT1
        display.clear();
    #endif
    #ifdef TTGO
        display.fillScreen(0);
    #endif

    #if WIFI_ACTIVE == true
        if (gDisplayValues.currentState == UP) {
          drawTime();
          drawIP();
        } else {
          drawtext10(64, 0, "no Wifi");
        }

    #endif
    // mode info 
    if (gDisplayValues.page==1) {
        // Affichage de l'état de la régulation
        #ifdef DEVKIT1
            drawtext10(64, 16, injection_type());
        #endif
        #ifdef TTGO
              drawtext10(30, 16, injection_type());
              display.setTextSize(1);
              display.setTextColor(TFT_WHITE, TFT_BLACK);
              display.setTextFont(4);
              display.setCursor(30, 48, 2);
              display.print("Dimmer (%)");
              display.setCursor(150, 48, 2);
              display.print(OLEDPOW);
        #endif

        // affichage de la température du dimmer
        #ifdef TTGO
              if (gDisplayValues.temperature.toInt() != 0) {
                ////// retrait à 1 décimale après la virgule
                int longueur = gDisplayValues.temperature.length();

                if (gDisplayValues.temperature.toInt() > 40) {
                  drawtext10TTGO(150, 16, gDisplayValues.temperature.substring(0, longueur - 1), TFT_GREEN);
                } else if (gDisplayValues.temperature.toInt() < 25) {
                  drawtext10TTGO(150, 16, gDisplayValues.temperature.substring(0, longueur - 1), TFT_BLUE);
                } else {
                  drawtext10TTGO(150, 16, gDisplayValues.temperature.substring(0, longueur - 1), TFT_WHITE);
                }
              }
        #endif

        // Affichage des infos de puissance ( sans les virgules )
        if (gDisplayValues.porteuse == false) {
          #ifdef DEVKIT1
            drawtext16(64, 30, String(OLEDNOSIN));
          #endif
          #ifdef TTGO
            drawtext16(120, 70, String(OLEDNOSIN));
          #endif

          gDisplayValues.dimmer = 0;  /// mise à zero du dimmer par sécurité
        } else {
          #ifdef DEVKIT1
                drawtext16(64, 30, String(gDisplayValues.watt, 0) + " W");
          #endif
          #ifdef TTGO
              String affiche = String(gDisplayValues.watt, 0);
              if (gDisplayValues.watt > config.delta) {
                drawtext16TTGO(120, 70, affiche, TFT_RED);
              } else if (gDisplayValues.watt < config.deltaneg) {
                if (gDisplayValues.watt < -999)
                /// correction dépassement de ligne pour les injections < -1000
                {
                  drawtext16TTGO(100, 70, affiche, TFT_BLUE);
                } else {
                  drawtext16TTGO(120, 70, affiche, TFT_BLUE);
                }

              } else {
                drawtext16TTGO(120, 70, affiche, TFT_GREEN);
              }       
          #endif
        }

        // Affichage des infos du dimmer
        // drawtext16(64,48, String(gDisplayValues.dimmer) + " %");

        #ifdef DEVKIT1
            // Affichage des infos du dimmer
            drawtext16(64, 48, String(gDisplayValues.dimmer) + " %");
            display.display();
        #endif
        #ifdef TTGO
            drawtext16TTGO(0, 70, String(gDisplayValues.dimmer), TFT_GREEN);
        #endif

        // display Fronius ligne du bas
        if (configmodule.Fronius_present) {
          #ifdef DEVKIT1
                display.setTextAlignment(TEXT_ALIGN_LEFT);
                display.setFont(ArialMT_Plain_10);
                display.drawString(0, 63, String(gDisplayValues.Fronius_prod));
                display.drawString(60, 63, String(gDisplayValues.Fronius_conso));
          #endif
          #ifdef TTGO
              display.setCursor(0, 120, 2);
              display.setTextColor(TFT_WHITE, TFT_BLACK);
              display.setTextSize(1);
              display.print("Prod : ");
              display.println(String(gDisplayValues.Fronius_prod, 0) + "W");
              display.setCursor(110, 120, 2);
              display.print("Conso : ");
              display.println(String(gDisplayValues.Fronius_prod + gDisplayValues.Fronius_conso, 0) + "W");
          #endif
        }

        // display Enphase ligne du bas
        if (configmodule.enphase_present) {
          #ifdef DEVKIT1
            display.setTextAlignment(TEXT_ALIGN_LEFT);
            display.setFont(ArialMT_Plain_10);
            display.drawString(0, 63, String(gDisplayValues.Fronius_prod));
            display.drawString(60, 63, String(gDisplayValues.Fronius_conso));
          #endif
          #ifdef TTGO
              display.setCursor(0, 120, 2);
              display.setTextColor(TFT_WHITE, TFT_BLACK);
              display.setTextSize(1);
              display.print("Prod : ");
              display.println(String(gDisplayValues.Fronius_prod) + "W");

              display.setCursor(110, 120, 2);
              if (String(configmodule.envoy) == "R") {
                display.print("Today : ");
                display.println(String(gDisplayValues.Fronius_conso) + "Wh");
              } else {
                // 20220616 - multinet : affichage de la conso total de la maison
                display.print("Total conso : ");
                display.println(String(gDisplayValues.Fronius_totalconso) + "W");
              }
          #endif
        }
      } else { // autre page
          char aff[]="Page  ";
          aff[5]=gDisplayValues.page+'0';
          display.drawString(aff,10,40,4);
      }
      // Sleep for 5 seconds, then update display again!
      vTaskDelay(5000 / portTICK_PERIOD_MS);
  } // for
} // updateDisplay

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
