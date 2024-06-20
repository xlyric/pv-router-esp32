#ifndef TASK_MDNS
#define TASK_MDNS

#include <ESPmDNS.h>
#include <WiFi.h>
#include "../config/enums.h"

extern Config config; 
extern Logs Logging;

void mdns_discovery(void * parameter) // NOSONAR
{

    for(;;){
        if (WiFi.status() == WL_CONNECTED && ( strcmp(config.dimmer,"") == 0 || strcmp(config.dimmer,"none") == 0 ) ) {
            /// recherche d'un dimmer 
            int nrOfServices = MDNS.queryService("http", "tcp");
            if (nrOfServices == 0) {
                Serial.println("No services were found.");
            } 
            
            else {
                
                Serial.print("Number of services found: ");
                Serial.println(nrOfServices);

                    for (int i = 0; i < nrOfServices; i=i+1) {
                        if (MDNS.port(i) == 1308 ){
                            String name_dimmer = MDNS.hostname(i)+".local";
                            name_dimmer.toCharArray(config.dimmer, name_dimmer.length()+1);
                            Serial.print("Dimmer found: ");
                            Serial.println(config.dimmer);
                            //Logging.Set_log_init("Dimmer found: ");
                            //Logging.Set_log_init(config.dimmer);
                            //Logging.Set_log_init("\r\n");
                            //sauvegarde de la configuration
                            //config.saveConfiguration();
                            break;
                        }
                    }

            }
           
        }
         vTaskDelay(10000 / portTICK_PERIOD_MS);
    }     
}



#endif