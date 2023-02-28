#ifndef TASK_MEASURE_ELECTRICITY
#define TASK_MEASURE_ELECTRICITY

#include <Arduino.h>

#include "../config/config.h"
#include "../config/enums.h"
// #include "mqtt-home-assistant.h"
#include "functions/energyFunctions.h"
#include "functions/dimmerFunction.h"
#include "functions/drawFunctions.h"
#include "functions/enphaseFunction.h"
#include "functions/froniusFunction.h"

extern DisplayValues gDisplayValues;
extern Configmodule configmodule; 
extern Logs Logging;
extern MQTT device_routeur; 
extern MQTT device_grid; 
extern MQTT device_inject; 
extern MQTT compteur_inject;
extern MQTT compteur_grid;
extern MQTT temperature;
extern MQTT device_alarm_temp;
#ifdef HARDWARE_MOD
      extern MQTT power_factor;
      extern MQTT power_vrms;
      extern MQTT power_irms;
      extern MQTT power_apparent;
#endif

int slowlog = TEMPOLOG - 1 ; 
long beforetime; 
#define timemilli 3.6e+6 
float WHtempgrid=0;
float WHtempinject=0;

int Pow_mqtt_send = 0;

void measureElectricity(void * parameter)
{
    for(;;){
    //  serial_println("[ENERGY] Measuring...");
       /// vérification qu'une autre task ne va pas fausser les valeurs
      long start = millis();
      
      
      if ( configmodule.pilote == false ) {
            #ifndef HARDWARE_MOD
                  injection2();
            #else
                  injection3();
            #endif
            if ( gDisplayValues.porteuse == false ) {
                  gDisplayValues.watt =0 ; 
                  slowlog ++; 
                  if (slowlog == TEMPOLOG) { logging.start += "--> No sinus, check 12AC power \r\n"; slowlog =0 ; }

            }
            serial_println(int(gDisplayValues.watt)) ;

      }
      else{
            gDisplayValues.porteuse = true;
      }
     

if (!AP) {
      if (configmodule.enphase_present ) {
            Enphase_get();
            if ( configmodule.pilote ) { 
                  //// inversion des valeurs pour enphase piloteur
                  int tempo = gDisplayValues.watt; 
                  gDisplayValues.watt = gDisplayValues.Fronius_conso ;
                  gDisplayValues.Fronius_conso = tempo; 
                  }
            }

      if (configmodule.Fronius_present ){
            Fronius_get();
            }           


            #if WIFI_ACTIVE == true
                  Pow_mqtt_send ++ ;
                  if ( Pow_mqtt_send >= 5 ) {
                  long timemesure = start-beforetime;
                  float wattheure = (timemesure * abs(gDisplayValues.watt) / timemilli) ;  

                  if (configmqtt.DOMOTICZ) {Mqtt_send_DOMOTICZ(String(config.IDX), String(int(gDisplayValues.watt)));  }
                  if ((configmqtt.HA) || ( configmqtt.JEEDOM)) {
                        device_routeur.send(String(int(gDisplayValues.watt)));
                        #ifdef HARDWARE_MOD
                              power_apparent.send(String(int(PVA)));
                              power_vrms.send(String(int(Vrms)));
                              power_irms.send(String(Irms));
                              power_factor.send(String(PowerFactor));
                        #endif
                  }
                  // send if injection
                  if (gDisplayValues.watt < 0 ){
                  if (configmqtt.DOMOTICZ) {
                        Mqtt_send_DOMOTICZ(String(config.IDX), String(int(-gDisplayValues.watt)),"injection");
                        Mqtt_send_DOMOTICZ(String(config.IDX), String("0") ,"grid");
                  }
                  if ((configmqtt.HA) || ( configmqtt.JEEDOM)) {
                        device_inject.send(String(int(-gDisplayValues.watt)));
                        device_grid.send(String("0"));
                        WHtempinject += wattheure; 
                        compteur_inject.send(String(WHtempinject));
                  }
                  
                  
                  // if (configmqtt.HA)compteur_grid.send(String("0"));
                  }
                  else {
                        if (configmqtt.DOMOTICZ) {
                              Mqtt_send_DOMOTICZ(String(config.IDX), String("0"),"injection");
                              Mqtt_send_DOMOTICZ(String(config.IDX), String(int(gDisplayValues.watt)),"grid");
                              if (discovery_temp) {Mqtt_send_DOMOTICZ(String("temperature"), String(gDisplayValues.temperature) );}
                        }
                  }
                  if ((configmqtt.HA) || (configmqtt.JEEDOM)) {
                        device_grid.send(String(int(gDisplayValues.watt)));
                        device_inject.send(String("0"));
                        WHtempgrid += wattheure;
                        compteur_grid.send(String(WHtempgrid));
                        //maj 202030209
                        if (discovery_temp) {
                              temperature.send(String(gDisplayValues.temperature));
                              device_alarm_temp.send(stringbool(security));
                        }
                  }
                  beforetime = start; 
                  Pow_mqtt_send = 0 ;
                  }             
            #endif
}

long end = millis();

      // Schedule the task to run again in 1 second (while
      // taking into account how long measurement took)
      if (configmodule.enphase_present && configmodule.pilote) {
            vTaskDelay(3000 / portTICK_PERIOD_MS);
      }
      else
      {      
            vTaskDelay((1500-(end-start)) / portTICK_PERIOD_MS);
      }

    }    
}

#endif
