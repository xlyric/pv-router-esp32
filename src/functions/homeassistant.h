#ifndef HOMEASSISTANT
#define HOMEASSISTANT

#include <Arduino.h>
#include <PubSubClient.h>

extern HA device_dimmer; 
extern HA device_routeur; 
extern HA device_grid;
extern HA device_inject;
extern HA compteur_inject;
extern HA compteur_grid;
extern HA switch_1;
extern HA switch_2;
extern HA temperature_HA;
extern HA power_factor;
extern HA power_vrms;
extern HA power_irms;
extern HA power_apparent;
extern HA device_alarm_temp;


void init_HA_sensor(){

        device_dimmer.Set_name("Consigne");
        device_dimmer.Set_object_id("dimmer");
        device_dimmer.Set_unit_of_meas("%");
        device_dimmer.Set_stat_cla("measurement");
        device_dimmer.Set_dev_cla("power_factor"); // fix is using native unit of measurement '%' which is not a valid unit for the device class ('power') it is using
        device_dimmer.Set_icon("mdi:percent");
        device_dimmer.Set_entity_type("sensor");
        device_dimmer.Set_retain_flag(true);
        // device_dimmer.Set_expire_after(true);
        
        device_routeur.Set_name("Puissance");
        device_routeur.Set_object_id("power");
        device_routeur.Set_unit_of_meas("W");
        device_routeur.Set_stat_cla("measurement");
        device_routeur.Set_dev_cla("power");
        device_routeur.Set_entity_type("sensor");
        device_routeur.Set_retain_flag(true);
        // device_routeur.Set_expire_after(true);

        device_grid.Set_name("Soutirage");
        device_grid.Set_object_id("grid");
        device_grid.Set_unit_of_meas("W");
        device_grid.Set_stat_cla("measurement");
        device_grid.Set_dev_cla("power");
        device_grid.Set_entity_type("sensor");
        device_grid.Set_retain_flag(true);
        // device_grid.Set_expire_after(true);


        device_inject.Set_name("Injection");
        device_inject.Set_object_id("inject");
        device_inject.Set_unit_of_meas("W");
        device_inject.Set_stat_cla("measurement");
        device_inject.Set_dev_cla("power");
        device_inject.Set_entity_type("sensor");
        device_inject.Set_retain_flag(true);
        // device_inject.Set_expire_after(true);

        compteur_grid.Set_name("Compteur soutirage");
        compteur_grid.Set_object_id("grid_Wh");
        compteur_grid.Set_unit_of_meas("Wh");
        compteur_grid.Set_stat_cla("total_increasing");
        compteur_grid.Set_dev_cla("energy");
        compteur_grid.Set_entity_type("sensor");
        compteur_grid.Set_retain_flag(true);
        // compteur_grid.Set_expire_after(true);


        compteur_inject.Set_name("Compteur injection");
        compteur_inject.Set_object_id("inject_Wh");
        compteur_inject.Set_unit_of_meas("Wh");
        compteur_inject.Set_stat_cla("total_increasing");
        compteur_inject.Set_dev_cla("energy");      
        compteur_inject.Set_entity_type("sensor");
        compteur_inject.Set_retain_flag(true);
        // compteur_inject.Set_expire_after(true);
            
        temperature_HA.Set_name("Température");
        temperature_HA.Set_object_id("Dallas");
        temperature_HA.Set_dev_cla("temperature"); 
        temperature_HA.Set_unit_of_meas("°C");
        temperature_HA.Set_stat_cla("measurement");
        temperature_HA.Set_entity_type("sensor");
        temperature_HA.Set_retain_flag(true);
        // temperature_HA.Set_expire_after(true);


        switch_1.Set_name("Relais 1");
        switch_1.Set_object_id("Switch1");
        switch_1.Set_entity_type("switch");
        switch_1.Set_retain_flag(true);
        // switch_1.Set_expire_after(true);

        switch_2.Set_name("Relais 2");
        switch_2.Set_object_id("Switch2");
        switch_2.Set_entity_type("switch");
        switch_2.Set_retain_flag(true);
        // switch_2.Set_expire_after(true);

        power_factor.Set_name("Facteur de puissance");
        power_factor.Set_object_id("PowerFactor");
        power_factor.Set_unit_of_meas("");
        power_factor.Set_stat_cla("measurement");
        power_factor.Set_dev_cla("power_factor");
        power_factor.Set_entity_type("sensor");
        power_factor.Set_retain_flag(true);
        // power_factor.Set_expire_after(true);

        power_vrms.Set_name("Tension RMS");
        power_vrms.Set_object_id("Vrms");
        power_vrms.Set_unit_of_meas("V");
        power_vrms.Set_stat_cla("measurement");
        power_vrms.Set_dev_cla("voltage");
        power_vrms.Set_entity_type("sensor");
        power_vrms.Set_retain_flag(true);
        // power_vrms.Set_expire_after(true);

        power_irms.Set_name("Intensité RMS");
        power_irms.Set_object_id("Irms");
        power_irms.Set_unit_of_meas("A");
        power_irms.Set_stat_cla("measurement");
        power_irms.Set_dev_cla("current");
        power_irms.Set_entity_type("sensor");
        power_irms.Set_retain_flag(true);
        // power_irms.Set_expire_after(true);

        power_apparent.Set_name("Puissance apparente");
        power_apparent.Set_object_id("ApparentPower");
        power_apparent.Set_unit_of_meas("VA");
        power_apparent.Set_stat_cla("measurement");
        power_apparent.Set_dev_cla("apparent_power");
        power_apparent.Set_entity_type("sensor");
        power_apparent.Set_retain_flag(true);
        // power_apparent.Set_expire_after(true);

        device_alarm_temp.Set_name("Surchauffe");
        device_alarm_temp.Set_object_id("alarm_temp");
        device_alarm_temp.Set_entity_type("binary_sensor");
        device_alarm_temp.Set_dev_cla("problem");
        device_alarm_temp.Set_retain_flag(true);
        // device_alarm_temp.Set_expire_after(true);




        client.setBufferSize(1024);
        device_routeur.discovery();
        device_dimmer.discovery();
        device_grid.discovery();
        device_inject.discovery();

        compteur_inject.discovery();
        compteur_inject.send(String("0"));

        compteur_grid.discovery();
        compteur_grid.send(String("0"));

        // temperature_HA.discovery();
        switch_1.discovery();
        switch_1.send(String(0));
 
        switch_2.discovery(); 
        switch_2.send(String(0));

        power_factor.discovery();
        power_vrms.discovery();
        power_irms.discovery();
        power_apparent.discovery();




}


#endif