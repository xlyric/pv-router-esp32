#ifndef UNIFIED_DIMMER_FUNCTIONS
#define UNIFIED_DIMMER_FUNCTIONS

// le but de cette fonction est de centraliser les commandes de dimmer  ( robotdyn et SSR ) de façon uniforme 

//***********************************
//************* LIBRAIRIES ESP
//***********************************
#include <Arduino.h>

//***********************************
//************* PROGRAMME PVROUTER
//***********************************
#include "../config/enums.h"

//***********************************
//************* Variables externes
//***********************************
extern Config config; 
extern DisplayValues gDisplayValues;
extern Logs logging;
extern dimmerLamp dimmer1;
extern dimmerLamp dimmer2;
extern dimmerLamp dimmer3;


// @brief  structure pour uniformiser les commandes de puissances entre robotdyn et SSR
//***********************************
//************* Structure gestion_puissance
//***********************************
struct gestion_puissance {
  private: unsigned long last_time = millis();

  public:float power;

  // setter
  void set_power(float unified_power,String reason="") {   /// le string reason est principalement utilisé pour le debug
    last_time = millis();
    if ( gDisplayValues.temperature > config.tmax ) { unified_power = 0; } /// si la température est supérieur à la température max on coupe tout
    else if ( unified_power > config.localfuse )  { unified_power = config.localfuse; } /// si puissance demandée supérieur à ma puissance max reglé alors puissance max
  
    this->power = unified_power;

    // pour le dimmer robotdyn et SSR Random 
  
    // On transforme la puissance totale à envoyer aux dimmers en watts pour mieux les répartir entre les 3 SSR
    // Meilleure précision en float 
    config.calcul_charge();
    float tmp_pwr_watt = 0;
    if ( int(unified_power) >= 0 ) { tmp_pwr_watt = unified_power * config.charge / 100;  }
    
    int dimmer1_pwr = 0;
    int dimmer2_pwr = 0;
    int dimmer3_pwr = 0;

    // Calcul de la puissance à envoyer à chaque dimmer
    if (tmp_pwr_watt <= config.charge1){ // Un seul dimmer à fournir
      if ( tmp_pwr_watt > 0 ) { dimmer1_pwr = tmp_pwr_watt * 100 / config.charge1 ;  }
      else
      { dimmer1_pwr = 0; }
      dimmer2_pwr = 0;
      dimmer3_pwr = 0;
    }  
    #ifdef outputPin2
    else if (tmp_pwr_watt <= (config.charge1+config.charge2)){ // 2 dimmers à fournir
      if (config.charge1 != 0) { dimmer1_pwr = 100; } // Permet d'avoir le dimmer1 configuré à 0 dans l'interface web
      if (config.charge2 != 0) { dimmer2_pwr = (tmp_pwr_watt - config.charge1 ) * 100 / config.charge2 ;} // Permet d'avoir le dimmer2 configuré à 0 dans l'interface web
      dimmer3_pwr = 0;
    }
    else { // Les 3 dimmers à fournir
      if (config.charge1 != 0) { dimmer1_pwr = 100; } // Permet d'avoir le dimmer1 configuré à 0 dans l'interface web
      if (config.charge2 != 0) { dimmer2_pwr = 100; } // Permet d'avoir le dimmer2 configuré à 0 dans l'interface web
      if (config.charge3 != 0) { dimmer3_pwr = (tmp_pwr_watt - (config.charge1 + config.charge2) ) * 100 / config.charge3; } else { dimmer3_pwr = 0;};
    }
    #endif 
    
    // Application de la puissance à chaque dimmer
    // Dimmer1
    if ( dimmer1_pwr != dimmer1.getPower() ) {  /// si puissance demandée différente de la puissance actuelle
     if (dimmer1_pwr == 0 && dimmer1.getState()==1) {  /// si puissance demandée = 0 et dimmer allumé alors on éteint
      dimmer1.setPower(0);
      dimmer1.setState(OFF);
      logging.Set_log_init("Dimmer1 Off "+ reason +"\r\n");
      delay(50);
     }
     else if (dimmer1_pwr != 0 && dimmer1.getState()==0) {  /// si puissance demandée différente de 0 et dimmer éteint alors on allume
      dimmer1.setState(ON);
      logging.Set_log_init("Dimmer1 On\r\n");
      delay(50);
      dimmer1.setPower(dimmer1_pwr);
     }
     else { dimmer1.setPower(dimmer1_pwr); }
    }
 
    #ifdef outputPin2
    // Dimmer2
    if ( dimmer2_pwr != dimmer2.getPower() ) {   /// si puissance demandée différente de la puissance actuelle
      if (dimmer2_pwr == 0 && dimmer2.getState()==1) {
      dimmer2.setPower(0);
      dimmer2.setState(OFF);
      logging.Set_log_init("Dimmer2 Off\r\n");
      delay(50);
      }     
      else if (dimmer2_pwr != 0 && dimmer2.getState()==0) {  /// si puissance demandée différente de 0 et dimmer éteint alors on allume
        dimmer2.setState(ON);
        logging.Set_log_init("Dimmer2 On\r\n");
        delay(50);
        dimmer2.setPower(dimmer2_pwr);
      }
      else { 
        dimmer2.setPower(dimmer2_pwr); 
      }
    }    
    // Dimmer3
    if ( dimmer3_pwr != dimmer3.getPower() ) {  
      if (dimmer3_pwr == 0 && dimmer3.getState()==1) {  /// si puissance demandée = 0 et dimmer allumé alors on éteint
      dimmer3.setPower(0);
      dimmer3.setState(OFF);
      logging.Set_log_init("Dimmer3 Off\r\n");
      delay(50);
      }     
      else if (dimmer3_pwr != 0 && dimmer3.getState()==0) {  /// si puissance demandée différente de 0 et dimmer éteint alors on allume
        dimmer3.setState(ON);
        logging.Set_log_init("Dimmer3 On\r\n");
        delay(50);
        dimmer3.setPower(dimmer3_pwr);
      }
      else { 
        dimmer3.setPower(dimmer3_pwr); 
      }
    }  
    #endif   
    
    logging.Set_log_init("dimmer 1: " + String(dimmer1_pwr) + "%\r\n" );
    #ifdef outputPin2
      logging.Set_log_init("dimmer 2: " + String(dimmer2_pwr) + "%\r\n" );
      logging.Set_log_init("dimmer 3: " + String(dimmer3_pwr) + "%\r\n" );
    #endif
}

  //getter
  float get_power() {
    // pour le dimmer robotdyn et SSR Random
    config.calcul_charge();
    int power1 = dimmer1.getPower();

    #ifdef outputPin2
      int power2 = dimmer2.getPower();
      int power3 = dimmer3.getPower();
      power = ((float)(power1*config.charge1 + power2*config.charge2 + power3*config.charge3) / (float)config.charge) ;
    #else
      this->power = ((float)(power1*config.charge1 ) / (float)config.charge) ;
    #endif

    return this->power;
  }

  void dimmer_off(String reason="") {
      if (dimmer1.getState()) {
        dimmer1.setPower(0);
        dimmer1.setState(OFF);
        logging.Set_log_init("Dimmer Off "+ reason +" \r\n");
        delay(50);
      }
      #ifdef outputPin2
        if (dimmer2.getState()) {
          dimmer2.setPower(0);
          dimmer2.setState(OFF);
          logging.Set_log_init("Dimmer2 Off\r\n");
          delay(50);
        }
        if (dimmer3.getState()) {
          dimmer3.setPower(0);
          dimmer3.setState(OFF);
          logging.Set_log_init("Dimmer3 Off\r\n");
          delay(50);
        }
      #endif
  }

  // fonction de coupure automatique après un certain temps
  void auto_off(int delay_off) {
    if (( millis() - last_time >= delay_off*60*1000)  && (power > 0) ) {
      dimmer_off();
      Serial.println("dimmer auto off");
      last_time = millis();
    }
  }
}; // Structure gestion_puissance()

#endif
