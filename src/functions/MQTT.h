#ifndef MQTT_SENSORS
#define MQTT_SENSORS

#include <Arduino.h>
#include <PubSubClient.h>

extern Mqtt configmqtt; 

extern MQTT device_dimmer;
extern MQTT device_routeur; 
extern MQTT device_grid;
extern MQTT device_inject;
extern MQTT compteur_inject;
extern MQTT compteur_grid;
extern MQTT switch_1;
extern MQTT switch_2;
extern MQTT temperature;
extern MQTT device_alarm_temp;

#ifdef HARDWARE_MOD
        extern MQTT power_factor;
        extern MQTT power_vrms;
        extern MQTT power_irms;
        extern MQTT power_apparent;
#endif

void init_MQTT_sensor(){
        device_dimmer.Set_object_id("dimmer");
        device_dimmer.Set_retain_flag(true);

        device_routeur.Set_object_id("power");
        device_routeur.Set_retain_flag(true);

        device_grid.Set_object_id("grid");
        device_grid.Set_retain_flag(true);

        device_inject.Set_object_id("inject");
        device_inject.Set_retain_flag(true);

        compteur_grid.Set_object_id("grid_Wh");
        compteur_grid.Set_retain_flag(true);

        compteur_inject.Set_object_id("inject_Wh");
        compteur_inject.Set_retain_flag(true);

        temperature.Set_object_id("temperature");
        temperature.Set_retain_flag(true);

        switch_1.Set_object_id("Switch1");
        switch_1.Set_retain_flag(true);

        switch_2.Set_object_id("Switch2");
        switch_2.Set_retain_flag(true);

        device_alarm_temp.Set_object_id("alarm_temp");
        device_alarm_temp.Set_retain_flag(true);

        compteur_inject.send(String("0"));
        compteur_grid.send(String("0"));
        // switch_1.send(String(0));
        // switch_2.send(String(0));

        #ifdef HARDWARE_MOD
                power_factor.Set_object_id("PowerFactor");
                power_vrms.Set_object_id("Vrms");
                power_irms.Set_object_id("Irms");
                power_apparent.Set_object_id("ApparentPower");
        #endif

}
void init_HA_sensor(){
        // init_MQTT_sensor();
        device_dimmer.Set_name("Consigne");
        // device_dimmer.Set_object_id("dimmer");
        device_dimmer.Set_unit_of_meas("%");
        device_dimmer.Set_stat_cla("measurement");
        device_dimmer.Set_dev_cla("power_factor"); // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
        device_dimmer.Set_icon("mdi:percent");
        device_dimmer.Set_entity_type("sensor");
        // device_dimmer.Set_retain_flag(true);
        // device_dimmer.Set_expire_after(true);
        
        device_routeur.Set_name("Puissance");
        // device_routeur.Set_object_id("power");
        device_routeur.Set_unit_of_meas("W");
        device_routeur.Set_stat_cla("measurement");
        device_routeur.Set_dev_cla("power");
        device_routeur.Set_entity_type("sensor");
        // device_routeur.Set_retain_flag(true);
        // device_routeur.Set_expire_after(true);

        device_grid.Set_name("Soutirage");
        // device_grid.Set_object_id("grid");
        device_grid.Set_unit_of_meas("W");
        device_grid.Set_stat_cla("measurement");
        device_grid.Set_dev_cla("power");
        device_grid.Set_entity_type("sensor");
        // device_grid.Set_retain_flag(true);
        // device_grid.Set_expire_after(true);


        device_inject.Set_name("Injection");
        // device_inject.Set_object_id("inject");
        device_inject.Set_unit_of_meas("W");
        device_inject.Set_stat_cla("measurement");
        device_inject.Set_dev_cla("power");
        device_inject.Set_entity_type("sensor");
        // device_inject.Set_retain_flag(true);
        // device_inject.Set_expire_after(true);

        compteur_grid.Set_name("Compteur soutirage");
        // compteur_grid.Set_object_id("grid_Wh");
        compteur_grid.Set_unit_of_meas("Wh");
        compteur_grid.Set_stat_cla("total_increasing");
        compteur_grid.Set_dev_cla("energy");
        compteur_grid.Set_entity_type("sensor");
        // compteur_grid.Set_retain_flag(true);
        // compteur_grid.Set_expire_after(true);

        compteur_inject.Set_name("Compteur injection");
        // compteur_inject.Set_object_id("inject_Wh");
        compteur_inject.Set_unit_of_meas("Wh");
        compteur_inject.Set_stat_cla("total_increasing");
        compteur_inject.Set_dev_cla("energy");      
        compteur_inject.Set_entity_type("sensor");
        // compteur_inject.Set_retain_flag(true);
        // compteur_inject.Set_expire_after(true);
            
        temperature.Set_name("Température");
        // temperature.Set_object_id("temperature");
        temperature.Set_dev_cla("temperature"); 
        temperature.Set_unit_of_meas("°C");
        temperature.Set_stat_cla("measurement");
        temperature.Set_entity_type("sensor");
        // temperature.Set_retain_flag(true);
        // temperature.Set_expire_after(true);

        switch_1.Set_name("Relais 1");
        // switch_1.Set_object_id("Switch1");
        switch_1.Set_entity_type("switch");
        switch_1.Set_retain(true);
        // switch_1.Set_expire_after(true);

        switch_2.Set_name("Relais 2");
        // switch_2.Set_object_id("Switch2");
        switch_2.Set_entity_type("switch");
        switch_2.Set_retain(true);
        // switch_2.Set_expire_after(true);

        device_alarm_temp.Set_name("Surchauffe");
        // device_alarm_temp.Set_object_id("alarm_temp");
        device_alarm_temp.Set_entity_type("binary_sensor");
        device_alarm_temp.Set_dev_cla("problem");
        // device_alarm_temp.Set_retain_flag(true);
        // device_alarm_temp.Set_expire_after(true);

        client.setBufferSize(1024);
        if (configmqtt.HA){
        device_routeur.HA_discovery();
        device_dimmer.HA_discovery();
        device_grid.HA_discovery();
        device_inject.HA_discovery();

        compteur_inject.HA_discovery();
        compteur_grid.HA_discovery();
        switch_1.HA_discovery(); 
        switch_2.HA_discovery(); 
        }
        #ifdef HARDWARE_MOD

                power_factor.Set_name("Facteur de puissance");
                // power_factor.Set_object_id("PowerFactor");
                power_factor.Set_unit_of_meas("");
                power_factor.Set_stat_cla("measurement");
                power_factor.Set_dev_cla("power_factor");
                power_factor.Set_entity_type("sensor");
                power_factor.Set_retain_flag(true);
                // power_factor.Set_expire_after(true);

                power_vrms.Set_name("Tension RMS");
                // power_vrms.Set_object_id("Vrms");
                power_vrms.Set_unit_of_meas("V");
                power_vrms.Set_stat_cla("measurement");
                power_vrms.Set_dev_cla("voltage");
                power_vrms.Set_entity_type("sensor");
                power_vrms.Set_retain_flag(true);
                // power_vrms.Set_expire_after(true);

                power_irms.Set_name("Intensité RMS");
                // power_irms.Set_object_id("Irms");
                power_irms.Set_unit_of_meas("A");
                power_irms.Set_stat_cla("measurement");
                power_irms.Set_dev_cla("current");
                power_irms.Set_entity_type("sensor");
                power_irms.Set_retain_flag(true);
                // power_irms.Set_expire_after(true);

                power_apparent.Set_name("Puissance apparente");
                // power_apparent.Set_object_id("ApparentPower");
                power_apparent.Set_unit_of_meas("VA");
                power_apparent.Set_stat_cla("measurement");
                power_apparent.Set_dev_cla("apparent_power");
                power_apparent.Set_entity_type("sensor");
                power_apparent.Set_retain_flag(true);
                // power_apparent.Set_expire_after(true);
        
        
                if (configmqtt.HA){
                        power_factor.HA_discovery();
                        power_vrms.HA_discovery();
                        power_irms.HA_discovery();
                        power_apparent.HA_discovery();
                }
                
        #endif

}


#endif