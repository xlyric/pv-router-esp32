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
extern HA temperature_HA;

void init_HA_sensor(){

        device_dimmer.Set_name("dimmer");
        device_dimmer.Set_unit_of_meas("%");
        device_dimmer.Set_stat_cla("measurement");
        device_dimmer.Set_dev_cla("power");
        
        device_routeur.Set_name("power");
        device_routeur.Set_unit_of_meas("W");
        device_routeur.Set_stat_cla("measurement");
        device_routeur.Set_dev_cla("power");

        device_grid.Set_name("grid");
        device_grid.Set_unit_of_meas("W");
        device_grid.Set_stat_cla("measurement");
        device_grid.Set_dev_cla("power");

        device_inject.Set_name("inject");
        device_inject.Set_unit_of_meas("W");
        device_inject.Set_stat_cla("measurement");
        device_inject.Set_dev_cla("power");

        compteur_grid.Set_name("grid_Wh");
        compteur_grid.Set_unit_of_meas("Wh");
        compteur_grid.Set_stat_cla("total_increasing");
        compteur_grid.Set_dev_cla("energy");

        compteur_inject.Set_name("inject_Wh");
        compteur_inject.Set_unit_of_meas("Wh");
        compteur_inject.Set_stat_cla("total_increasing");
        compteur_inject.Set_dev_cla("energy");      
                
        temperature_HA.Set_name("Dallas");
        temperature_HA.Set_dev_cla("temperature"); 
        temperature_HA.Set_unit_of_meas("°C");
        temperature_HA.Set_stat_cla("measurement");

        switch_1.Set_name("Switch1");
        switch_1.Set_dev_cla("switch"); 

        client.setBufferSize(1024);
        device_routeur.discovery();
        device_dimmer.discovery();
        device_grid.discovery();
        device_inject.discovery();
        compteur_inject.discovery();
        compteur_grid.discovery();
        temperature_HA.discovery();
        switch_1.discovery();

}


#endif