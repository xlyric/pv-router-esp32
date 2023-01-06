#ifndef DRAW_FUNCTIONS
#define DRAW_FUNCTIONS

#include <WiFi.h>

#if NTP
  #include <NTPClient.h>
#endif

#include "../config/enums.h"
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
extern unsigned char measureIndex;

#if NTP
  extern NTPClient timeClient;
#endif


  void drawTime(){
    #ifdef  DEVKIT1
        if (!AP) {
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 0,timeClient.getFormattedTime());
      }
      #ifdef  TTGO
        display.setCursor(0, 0, 2);
        display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextSize(1);
        
          display.println(timeClient.getFormattedTime());

      #endif
    #endif
  }


void drawIP(){
  #ifdef  DEVKIT1
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 0,gDisplayValues.IP);
  #endif
  #ifdef  TTGO
    display.setCursor(120, 0, 2);
    display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextSize(1);
    display.println(gDisplayValues.IP);
  #endif
}
/*
void drawSignalStrength(){
  #ifdef  DEVKIT1
  const byte X = 51;
  const byte X_SPACING = 2;

  // Draw the four base rectangles
  display.setColor(WHITE);
  display.fillRect(X, 8-2, X+1,8-2 ); // Bar 1
  display.fillRect(X + X_SPACING, 8-2 ,X + X_SPACING +2, 8-2); // Bar 2
  display.fillRect(X + X_SPACING*2, 8-2,X + X_SPACING*2 +3, 8-2 ); // Bar 3
  display.fillRect(X + X_SPACING*3, 8-2,X + X_SPACING*3 +4 , 8-2); // Bar 4

  // Draw bar 2
  if(gDisplayValues.wifi_strength > -70){
    display.fillRect(X+X_SPACING, 8-4, 1, 4);
  }

  // Draw bar 3
  if(gDisplayValues.wifi_strength > -60){
    display.fillRect(X+X_SPACING*2, 8-6, 1, 6);
  }

  // Draw bar 4
  if(gDisplayValues.wifi_strength >= -50){
    display.fillRect(X+X_SPACING*3, 8-8, 1, 8);
  }
  #endif
}

void drawMeasurementProgress(){
  #ifdef  DEVKIT1
  const byte Y = SCREEN_WIDTH - 20;
  display.drawRect(0, Y, measureIndex*2, 2);
  #endif
}
*/
/**
 * The screen that is displayed when the ESP has just booted
 * and is connecting to WiFi & AWS.
 */
void drawBootscreen(){
  #ifdef  DEVKIT1
  byte X = 14;
  byte Y = 70;
  byte WIDTH = 6;
  byte MAX_HEIGHT = 35;
  byte HEIGHT_STEP = 10;
  byte X_SPACING = 10;

  display.fillRect(X              , Y, WIDTH, MAX_HEIGHT - HEIGHT_STEP*3);
  display.fillRect(X + X_SPACING  , Y - HEIGHT_STEP, WIDTH, MAX_HEIGHT - HEIGHT_STEP*2);
  display.fillRect(X + X_SPACING*2, Y - HEIGHT_STEP*2, WIDTH, MAX_HEIGHT - HEIGHT_STEP);
  display.fillRect(X + X_SPACING*3, Y - HEIGHT_STEP*3, WIDTH, MAX_HEIGHT);

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, Y + MAX_HEIGHT / 2 ,"Connecting" );
  

  if(gDisplayValues.currentState == CONNECTING_WIFI){
    display.drawString(0, Y + MAX_HEIGHT / 2 ,"   WiFi" );
    
  }

  if(gDisplayValues.currentState == CONNECTING_AWS){
    display.drawString(0, Y + MAX_HEIGHT / 2 ,"   AWS" );
    
  }
 #endif

}
 
/**
 * Draw the current amps & watts in the middle of the display.
 */
/*
void drawAmpsWatts(){
  const int width = 85 ; 
  const int height = 30 ; 
  const int startY = 18;

  String watts = String(gDisplayValues.watt, 0);
  String lblWatts = "Watt";
  
  // Calculate how wide (pixels) the text will be once rendered.
  // Each character = 6 pixels, with font size 2, that is 12 pixels.
  // -1 because of the spacing between letters (last one doesn't)
  int widthWatts = watts.length() * 12 - 1;
  /// clean rect
  display.setColor(BLACK);
  display.fillRect(width, height, widthWatts, startY);
  /// write Data
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(width,height, String(watts) + " W"); 
}
*/
/**
 * Draw the current amps & watts in the middle of the display.
 */
/*
void drawDimmer(){
  const int width = 85 ; 
  const int height = 50 ; 
  const int startY = 18;

  String watts = String(gDisplayValues.dimmer);
  
  // Calculate how wide (pixels) the text will be once rendered.
  // Each character = 6 pixels, with font size 2, that is 12 pixels.
  // -1 because of the spacing between letters (last one doesn't)
  int widthWatts = watts.length() * 12 - 1;
  /// clean rect
  display.setColor(BLACK);
  display.fillRect(width, height, widthWatts, startY);
  /// write Data
  display.setColor(WHITE);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(width,height, String(watts) ); 
}
*/

/**
 * Affichage de texte de format 16 
 */

void drawtext16(int width,int height, String text ){
  
  #ifdef  DEVKIT1  
  const int startY = 18;
  // Calculate how wide (pixels) the text will be once rendered.
  // Each character = 6 pixels, with font size 2, that is 12 pixels.
  // -1 because of the spacing between letters (last one doesn't)
    int widthtext = text.length() * 12 - 1;
  /// clean rect
    display.setColor(BLACK);
    display.fillRect(width, height, widthtext, startY);
  /// write Data
    display.setColor(WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(width,height, String(text) ); 
  #endif
  
  #ifdef  TTGO
    display.setCursor(width, height, 2); display.setTextSize(2);
    display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextFont(2);
    display.print(String(text));
  #endif

}

void drawtext10(int width,int height, String text  ){

  #ifdef  DEVKIT1
    const int startY = 12;
    
  // Calculate how wide (pixels) the text will be once rendered.
  // Each character = 6 pixels, with font size 2, that is 12 pixels.
  // -1 because of the spacing between letters (last one doesn't)
    int widthtext = text.length() * 6 - 1;
  /// clean rect
    display.setColor(BLACK);
    display.fillRect(width, height, widthtext, startY);
  /// write Data
    display.setColor(WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(width,height, String(text) ); 
  #endif

  #ifdef  TTGO
    display.setTextFont(3);
    display.setCursor(width, height, 2);
    display.setTextColor(TFT_WHITE,TFT_BLACK);  display.setTextSize(2);
    display.print(String(text));
  #endif

}

void drawtext16TTGO(int width,int height, String text, int color ){
#ifdef  TTGO  
    display.setCursor(width, height, 2); display.setTextSize(2);
    display.setTextColor(color,TFT_BLACK);  display.setTextFont(4);
    display.print(String(text));
#endif
}

void drawtext10TTGO(int width,int height, String text, int color ){
#ifdef  TTGO  
    display.setCursor(width, height, 2); 
    display.setTextColor(color,TFT_BLACK);    display.setTextSize(2);
    display.print(String(text));
#endif
}

#endif