#ifndef TASK_DIMMER
#define TASK_DIMMER

    #include <Arduino.h>
    #include "../config/config.h"
    #include "../config/enums.h"
    #include "../functions/dimmerFunction.h"


extern HTTPClient http;

extern DisplayValues gDisplayValues;
extern  Config config;
extern dimmerLamp dimmer_hard; 


/**
 * Task: Modifier le dimmer en fonction de la production
 * 
 * récupère les informations, conso ou injection et fait varier le dimmer en conséquence
 * 
 */
void get_dimmer_child_power (){
    /// récupération de la puissance du dimmer enfant en http
        // si config.dimmer = none  alors pas de dimmer enfant
        if ( strcmp(config.dimmer,"none") == 0 ) { {
            gDisplayValues.puissance_route = 0;
            return;
        }

        String baseurl; 
        baseurl = "/state";
        http.begin(String(config.dimmer),80,baseurl);   
        
        int httpResponseCode = http.GET();
        String dimmerstate = "0"; 
        dimmerstate = http.getString();
        http.end();
        
        if (httpResponseCode==200) {
            DynamicJsonDocument doc(128);
            deserializeJson(doc, dimmerstate);
            //int ptotal = doc["Ptotal"];
            
            gDisplayValues.puissance_route = doc["Ptotal"];
  
            Serial.println(gDisplayValues.puissance_route);
        }
        else {
            gDisplayValues.puissance_route = 0;
        }
     

}


void updateDimmer(void * parameter){
  for (;;){
  gDisplayValues.task = true;
#if WIFI_ACTIVE == true
    dimmer();
    
    #if CLEAN
    /*
    /// si changement à faire
    if  (gDisplayValues.change != 0 ) {
        Serial.println(F("changement des valeurs dimmer-MQTT"));
        // envoie de l'information au dimmer et au serveur MQTT ( mosquito ou autre )
        dimmer_change(); 
    }*/ 
    #endif
   
    // si dimmer local alors calcul de puissance routée 
    if (config.dimmerlocal) {
       gDisplayValues.puissance_route = config.resistance * dimmer_hard.getPower()/100; 
    }
    // si dimmer distant alors calcul de puissance routée
    else  {  
        #ifndef POURCENTAGE
            get_dimmer_child_power ();
        #else
            gDisplayValues.puissance_route = config.resistance * gDisplayValues.dimmer/100;
        #endif
    }

    



#endif
    gDisplayValues.task = false;
   // Sleep for 5 seconds, avant de refaire une analyse
    vTaskDelay(pdMS_TO_TICKS(4000));
    // 24/01/2023 changement de 5 à 4s 
  }
}
#endif