#ifndef DISPLAY_FUNCTIONS
#define DISPLAY_FUNCTIONS

#include "../config/config.h"
#include "functions/appweb.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"
#include "functions/display.h"

void affichage_normal();
void affichage_autre();
void call_display();

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
extern Config config;

void call_display(){
    //serial_println(F("lcd task"));
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
    switch (gDisplayValues.option) {
          case 0:
              affichage_normal();
            break;
          case 1:
              affichage_autre();
              drawtext10TTGO(0, 40, "Flip screen", TFT_GREEN);
            break;
          case 2:
              affichage_autre();
              drawtext10TTGO(0, 80, "Switch SCT-013", TFT_GREEN);
            break;
          default:
              affichage_normal();
            break;
    }

}


void affichage_normal(){
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
                        display.print("Routed (W)");
                        display.setCursor(150, 48, 2);

                        if (strcmp(config.topic_Shelly,"none") == 0)   display.print(OLEDPOW);
                        else display.print("Shelly");
                        
                  #endif

                  // affichage de la température du dimmer
                  #ifdef TTGO
                        if (int(gDisplayValues.temperature) != 0) {
                          ////// retrait à 1 décimale après la virgule
                          
                          if (int(gDisplayValues.temperature) > 40) {
                            drawtext10TTGO(150, 16, String(gDisplayValues.temperature,1), TFT_GREEN);
                          } else if (int(gDisplayValues.temperature) < 25) {
                            drawtext10TTGO(150, 16, String(gDisplayValues.temperature,1), TFT_BLUE);
                          } else {
                            drawtext10TTGO(150, 16, String(gDisplayValues.temperature,1), TFT_WHITE);
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
                  #ifdef DEVKIT1
                      // Affichage des infos du dimmer
                      drawtext16(64, 48, String(gDisplayValues.puissance_route) + " %");
                      display.display();
                  #endif
                  #ifdef TTGO
                      if (gDisplayValues.puissance_route < 10000 ) {
                      drawtext16TTGO(0, 70, String(gDisplayValues.puissance_route), TFT_GREEN);
                      }
                      else {
                      drawtext16TTGO(0, 70, "9999", TFT_GREEN);
                      }
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
}

void affichage_autre(){
   drawtext10TTGO(0, 40, "Flip screen", TFT_WHITE);
   drawtext10TTGO(0, 80, "Switch SCT-013", TFT_WHITE);
}

#endif