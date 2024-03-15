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
int get_dimmer_child_power (){
    http.begin("http://" + String(config.dimmer) + "/state");   
                int httpResponseCode = http.GET();
                int Ptotal_read;
        String dimmerstate = "0"; 
        
        if (httpResponseCode==200) {
            dimmerstate = http.getString(); // Modif RV - 20230302 - déplacé ici, était 2 lignes plus haut
            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, dimmerstate);
            if (error) {
                Serial.print(F("get_dimmer_child_power() failed: "));
                logging.Set_log_init("get_dimmer_child_power() failed: ",true);
                Serial.println(error.c_str());
                
                return 0 ; 
            }
            //int ptotal = doc["Ptotal"];
            
            Ptotal_read = doc["Ptotal"];
            return Ptotal_read;
  
            DEBUG_PRINTLN("routed : " + Ptotal_read);

        }
        else {
            return 0 ;
        }
     http.end();
}
extern Memory task_mem; 

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
   
    #ifndef POURCENTAGE
        int local_power = 0 ;
        int child_power = 0 ;
    // si dimmer local alors calcul de puissance routée 
    //if (config.dimmerlocal) { // Pas besoin de ce if, ça répond bien même si pas d'enfant configuté
        local_power =  dimmer_hard.getPower() * config.resistance/100; // watts
        //}
        //else { 
        //    local_power = 0 ; 
        //}
    
    // si dimmer distant alors calcul de puissance routée
    if ( strcmp(config.dimmer,"") != 0 && strcmp(config.dimmer,"none") != 0 ) {  
        child_power = get_dimmer_child_power();  // watts
        }

        gDisplayValues.puissance_route = local_power + child_power;

        /// Actualisation de gDisplayValues.dimmer à chaque requêtage du dimmer enfant
        gDisplayValues.dimmer = gDisplayValues.puissance_route * 100/config.resistance;

        #else
            gDisplayValues.puissance_route = config.resistance * gDisplayValues.dimmer/100; // watts
        #endif
    
#endif


    gDisplayValues.task = false;
    task_mem.task_updateDimmer = uxTaskGetStackHighWaterMark(NULL);
   // Sleep for 5 seconds, avant de refaire une analyse
    vTaskDelay(pdMS_TO_TICKS(4000));
    // 24/01/2023 changement de 5 à 4s 
  }
}
#endif