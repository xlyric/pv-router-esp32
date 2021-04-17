#ifndef DIMMER_FUNCTIONS
#define DIMMER_FUNCTIONS

#include <WiFi.h>
#include "../config/enums.h"
#include "../config/config.h"

extern DisplayValues gDisplayValues;

//***********************************
//************* Fonction aservissement autonome
//***********************************

void dimmer(int commande){
gDisplayValues.change = 0; 

  // 0 -> linky ; 1-> injection  ; 2-> stabilisé

  // si grosse puissance instantanée sur le réseau, coupure du dimmer. ( ici 350w environ ) 
  if ( gDisplayValues.watt >= 350 && gDisplayValues.dimmer != 0 )  {
    gDisplayValues.dimmer = 0 ;  
    gDisplayValues.change = 1 ;
    } 
  
  /// si gros mode linky  on reduit la puissance par extrapolation ( valeur de puissance supérieur à config.delta + 10 )
  else if (commande == 0 && gDisplayValues.dimmer != 0 && gDisplayValues.watt >= (config.delta+10) ) {
    gDisplayValues.dimmer += -2*((gDisplayValues.watt-config.delta)/(50*config.resistance/1000)) ; 
    gDisplayValues.change = 1; 
    } 
  
    /// si petit mode linky on reduit la puissance 
  else if (commande == 0 && gDisplayValues.dimmer != 0 ) {
    gDisplayValues.dimmer += -1 ; 
    gDisplayValues.change = 1; 
    }  
  
    /// si grosse injection on augmente la puissance par extrapolation
  else if (commande == 1 && gDisplayValues.watt <= (config.deltaneg-10) ) {   
    gDisplayValues.dimmer += 2*abs(gDisplayValues.watt/(50*config.resistance/1000)) ; 
    gDisplayValues.change = 1 ; 
    } 
  
    /// si injection legère on augmente la puissance doucement
  else if (commande == 1  ) { 
    gDisplayValues.dimmer += 1 ; 
    gDisplayValues.change = 1 ; 
    }
  
    /// test puissance de sécurité 
  if ( gDisplayValues.dimmer >= config.num_fuse ) {
    gDisplayValues.dimmer = config.num_fuse; 
    gDisplayValues.change = 1 ; 
    }

  if ( gDisplayValues.dimmer <= 0 && gDisplayValues.dimmer != 0 ) {
    gDisplayValues.dimmer = 0; 
    gDisplayValues.change = 1 ; 
    }
    /// mode stabilisé, envoie de gDisplayValues.watt de temps en temps
  if (commande == 2  ) { 
    gDisplayValues.change = 1 ; 
    } 

    gDisplayValues.security ++ ;

    if ( gDisplayValues.security >= 5 ) { if ( gDisplayValues.dimmer <= 0 ) {gDisplayValues.dimmer = 0; gDisplayValues.change = 1 ; gDisplayValues.security = 0;  }} 


}

#endif