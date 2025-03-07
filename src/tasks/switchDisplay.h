#ifndef TASK_SWITCH_DISPLAY
#define TASK_SWITCH_DISPLAY

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>
#ifdef  TTGO
  #include <TFT_eSPI.h>
#endif

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/config.h"
#include "../functions/display.h"

//***********************************
//************* Variables externes
//***********************************
#ifdef  TTGO  
  extern TFT_eSPI display;
#endif
extern DisplayValues gDisplayValues;
extern Config config; 
extern Memory task_mem; 

//***********************************
//************* Constante
//***********************************
#define TFT_PIN 4 // NOSONAR

//***********************************
//************* switchDisplay
//***********************************
void switchDisplay(void * parameter) { // NOSONAR
  int timer = millis();
  int timeroptions ;

  for(;;) {
    if (digitalRead(TFT_PIN)==HIGH && config.ScreenTime !=0 ) { 
        if ( millis() > timer + config.ScreenTime*1000 ) {
          digitalWrite(TFT_PIN,LOW);
        }
    }

    // réinit du compteur si bouton option pressé
    if ( gDisplayValues.nextbutton ) {
      gDisplayValues.nextbutton = false; 
      digitalWrite(TFT_PIN,HIGH);
      call_display(); 
      timeroptions = millis();  
      timer = millis();
    }

    // si option = 0 --> on/off écran 
    if (gDisplayValues.option == 0 ) {
      if (gDisplayValues.screenbutton || gDisplayValues.screenstate == HIGH ) { // if right button is pressed or HTTP call 
        if (digitalRead(TFT_PIN)==HIGH) {             // and the status flag is LOW
          gDisplayValues.screenstate = LOW ;              
          logging.Set_log_init("Oled Off\r\n",true);
          digitalWrite(TFT_PIN,LOW);     // and turn Off the OLED
        }                           
        else {                        // otherwise...      
          Serial.println("button left/bottom pressed");
          gDisplayValues.screenstate = LOW ;
            
          logging.Set_log_init("Oled On\r\n",true);
          digitalWrite(TFT_PIN,HIGH);      // and turn On  the OLED
          if (config.ScreenTime !=0 ) {
            timer = millis();
          }
        }
        gDisplayValues.screenbutton = false; 
      }
    }
    // sinon différentes options présentes 
    else {
      if (gDisplayValues.screenbutton) {
        // valider l'option sélectionnée et effectuer l'action correspondante
        switch (gDisplayValues.option) {
          case 1:
            // flip screen
            config.flip = !config.flip; 
            #ifdef  TTGO
              if (config.flip) 
                display.setRotation(3);
              else 
                display.setRotation(1);
              gDisplayValues.option = 0 ;
              call_display(); 
              logging.Set_log_init(config.saveConfiguration(),true);  ///save configuration
            #endif
            break;
          case 2:
            // flip SCT013
            config.polarity = !config.polarity;
            config.sauve_polarity();
            gDisplayValues.option = 0 ;
            call_display(); 
            break;
          default:
            gDisplayValues.option = 0 ;
            call_display(); 
            break;
            // ajouter d'autres cas pour chaque option disponible
        } // switch
        gDisplayValues.screenbutton = false;
      }

     // si rien n'est fait pendant 10s retour à l'affichage d'origine ( et option != 0 )
      if (millis() > timeroptions + 10000 ) {   
        timeroptions = millis(); 
        gDisplayValues.option = 0 ; 
      }
    }
  
    task_mem.task_switchDisplay = uxTaskGetStackHighWaterMark(nullptr);
    vTaskDelay(pdMS_TO_TICKS(1000+(esp_random() % 61) - 30));  
  } 
} 
#endif
