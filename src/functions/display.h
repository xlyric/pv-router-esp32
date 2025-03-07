#ifndef DISPLAY_FUNCTIONS
#define DISPLAY_FUNCTIONS

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <TimeLib.h>
#ifdef ESP32D1MINI_FIRMWARE
  #include <OLEDDisplay.h>
  #include "OLEDDisplayUi.h"
  #include "SSD1306Wire.h"
  #include <WiFiUdp.h>
#endif
#ifdef TTGO
  #include <TFT_eSPI.h>
#endif

//***********************************
//************* PROGRAMME PVROUTEUR
//***********************************
#include "../config/config.h"
#include "functions/appweb.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"
#include "functions/display.h"

//***********************************
//************* Déclaration de fonctions
//***********************************
void affichage_normal();
void affichage_autre();
void call_display();
#ifdef ESP32D1MINI_FIRMWARE
  SSD1306Wire display(0x3c, 25, 27);
  OLEDDisplayUi   ui( &display );
  #define defaultFont ArialMT_Plain_16
  void display_routage(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y);
  void display_temperature(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y);
  void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state);
  void drawProgress(OLEDDisplay *display, int percentage, String label);
  void init_ui(); 
#endif

//***********************************
//************* Variables externes
//***********************************
extern DisplayValues gDisplayValues;
extern Configmodule configmodule;
extern Config config;
#ifdef TTGO
  extern TFT_eSPI display;
#endif

//***********************************
//************* call_display
//***********************************
void call_display(){
  #ifdef ESP32D1MINI_FIRMWARE
      display.clear();
      display.drawString(100, 0, "dB");
      display.display();
  #endif
  #ifdef TTGO
      display.fillScreen(0);
  #endif

  #if WIFI_ACTIVE == true
    if (gDisplayValues.currentState == DEVICE_STATE::UP) {

      drawIP();
    } 
    else {
      drawtext10(64, 0, "no Wifi");
    }
  #endif

  // mode info 
  switch (gDisplayValues.option) {
    #ifdef TTGO
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
    #endif
      default:
        #ifdef TTGO
          affichage_normal();
        #elif ESP32D1MINI_FIRMWARE
          ui.update();
        #endif
      break;
  } // switch
}

//***********************************
//************* affichage_normal
//***********************************
void affichage_normal(){
  //************* Affichage de l'état de la régulation
  #ifdef ESP32D1MINI_FIRMWARE
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

    if (strcmp(config.topic_Shelly,"none") == 0)   
      display.print(OLEDPOW);
    else 
    display.print("Shelly");    
  #endif

  // affichage de la température du dimmer
  #ifdef TTGO
    if (int(gDisplayValues.temperature) != 0) {
      ////// retrait à 1 décimale après la virgule
      
      if (int(gDisplayValues.temperature) > 40) {
        drawtext10TTGO(150, 16, String(gDisplayValues.temperature,1), TFT_GREEN);
      } 
      else if (int(gDisplayValues.temperature) < 25) {
        drawtext10TTGO(150, 16, String(gDisplayValues.temperature,1), TFT_BLUE);
      } 
      else {
        drawtext10TTGO(150, 16, String(gDisplayValues.temperature,1), TFT_WHITE);
      }
    }
  #endif

  // Affichage des infos de puissance ( sans les virgules )
  if (gDisplayValues.porteuse == false) {
    #ifdef ESP32D1MINI_FIRMWARE
      drawtext16(64, 30, String(OLEDNOSIN));
    #endif

    #ifdef TTGO
      drawtext16(120, 70, String(OLEDNOSIN));
    #endif
    gDisplayValues.dimmer = 0;  /// mise à zero du dimmer par sécurité
  } 
  else {
    #ifdef ESP32D1MINI_FIRMWARE
      drawtext16(64, 30, String(gDisplayValues.watt, 0) + " W");
    #endif
    
    #ifdef TTGO
      String affiche = String(gDisplayValues.watt, 0);
      if (gDisplayValues.watt > config.delta) {
        drawtext16TTGO(120, 70, affiche, TFT_RED);
      } 
      else if (gDisplayValues.watt < config.deltaneg) {
        /// correction dépassement de ligne pour les injections < -1000
        if (gDisplayValues.watt < -999) {
          drawtext16TTGO(100, 70, affiche, TFT_BLUE);
        } 
        else {
          drawtext16TTGO(120, 70, affiche, TFT_BLUE);
        }
      } 
      else {
        drawtext16TTGO(120, 70, affiche, TFT_GREEN);
      }       
    #endif
  }

  // Affichage des infos du dimmer
  #ifdef ESP32D1MINI_FIRMWARE
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
    #ifdef ESP32D1MINI_FIRMWARE
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
    #ifdef ESP32D1MINI_FIRMWARE
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
      } 
      else {
        // 20220616 - multinet : affichage de la conso total de la maison
        display.print("Total conso : ");
        display.println(String(gDisplayValues.Fronius_totalconso) + "W");
      }
    #endif
  }
}

//***********************************
//************* affichage_autre
//***********************************
void affichage_autre(){
  #ifdef TTGO 
    drawtext10TTGO(0, 40, "Flip screen", TFT_WHITE);
    drawtext10TTGO(0, 80, "Switch SCT-013", TFT_WHITE);
  #endif
}

//***********************************
//************* Spécifique OLED
//***********************************
#ifdef ESP32D1MINI_FIRMWARE
  const uint8_t activeSymbole[] PROGMEM = {
      B00000000,
      B00000000,
      B00011000,
      B00100100,
      B01000010,
      B01000010,
      B00100100,
      B00011000
  };

  const uint8_t inactiveSymbole[] PROGMEM = {
      B00000000,
      B00000000,
      B00000000,
      B00000000,
      B00011000,
      B00011000,
      B00000000,
      B00000000
  };

  struct Oled {
    // init oled
    void init() {
      // Initialising the UI will init the display too.
      display.init();
      display.clear();
      display.display();
      //display.flipScreenVertically();
      display.setFont(ArialMT_Plain_10);
      display.setTextAlignment(TEXT_ALIGN_CENTER);
      display.setContrast(50, 100, 30);
      } 
  
    // affichage de l'IP
    void display_ip() {
        display.setFont(ArialMT_Plain_10);
        String ip = WiFi.localIP().toString();
        String rssi = String(WiFi.RSSI());
        display.drawString(0, 0, ip);
        display.drawString(100, 0, rssi + "dB");
        display.setFont(defaultFont);
    }

    void wait_for_wifi(uint8_t counter) {
      display.clear();
      display.drawString(64, 10, "Connecting to WiFi");
      display.drawXbm(46, 30, 8, 8, counter % 3 == 0 ? activeSymbole : inactiveSymbole);
      display.drawXbm(60, 30, 8, 8, counter % 3 == 1 ? activeSymbole : inactiveSymbole);
      display.drawXbm(74, 30, 8, 8, counter % 3 == 2 ? activeSymbole : inactiveSymbole);
      display.display();
    }    
  }; // struct Oled

  Oled oled; 

  FrameCallback frames[] = { display_temperature, display_routage };
  int numberOfFrames = 2;
  OverlayCallback overlays[] = { drawHeaderOverlay };
  int numberOfOverlays = 1;

  // task oled et meteo 
  void oled_task() {
    ui.update();
  }
      
  //// affichage des infos basse de l'écran
    void drawHeaderOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
      display->setColor(WHITE);
      display->setFont(ArialMT_Plain_10);
      display->setTextAlignment(TEXT_ALIGN_LEFT);
      display->drawString(0, 54, String(gDisplayValues.puissance_route));
      display->setTextAlignment(TEXT_ALIGN_RIGHT);
      display->drawString(128, 54, String(int(gDisplayValues.watt)) );
      display->drawHorizontalLine(0, 52, 128);
    }

    void drawProgress(OLEDDisplay *display, int percentage, String label) {
      display->clear();
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->setFont(ArialMT_Plain_10);
      display->drawString(64, 10, label);
      display->drawProgressBar(2, 28, 124, 10, percentage);
      display->display();
    }

    // affichage de la température
    void display_temperature(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y) {
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->setFont(ArialMT_Plain_24);
      // int puissance_temp = unified_dimmer.get_power();
      String info_puissance = "P: "+ String(int(gDisplayValues.watt)) + "W";
      display->drawString(64 + x, 0 + y, String( info_puissance ));
      display->setFont(ArialMT_Plain_24);
      String inside_temp = String(gDisplayValues.temperature) + "°C";
      display->drawString(64 + x, 24 + y, String( inside_temp ));
    }

    void display_routage(OLEDDisplay *display, OLEDDisplayUiState* state, short int x, short int y) {
      display->setTextAlignment(TEXT_ALIGN_CENTER);
      display->setFont(ArialMT_Plain_24);
      String info_puissance =  "R: " + String(gDisplayValues.puissance_route) + "W";
      display->drawString(64 + x, 0 + y, String( info_puissance ));
      display->setFont(ArialMT_Plain_24);
      String inside_temp = String(gDisplayValues.temperature) + "°C";
      display->drawString(64 + x, 24 + y, String( inside_temp ));
    }

    void init_ui() {
      ui.setTargetFPS(5);

      ui.setActiveSymbol(activeSymbole);
      ui.setInactiveSymbol(inactiveSymbole);

      // TOP, LEFT, BOTTOM, RIGHT
      ui.setIndicatorPosition(BOTTOM);

      // Defines where the first frame is located in the bar.
      ui.setIndicatorDirection(LEFT_RIGHT);

      // You can change the transition that is used
      // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
      ui.setFrameAnimation(SLIDE_LEFT);

      ui.setFrames(frames, numberOfFrames);

      ui.setOverlays(overlays, numberOfOverlays);

      ui.init();

    }
#endif // ESP32D1MINI_FIRMWARE

#endif
