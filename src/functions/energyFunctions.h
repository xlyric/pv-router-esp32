#ifndef ENERGY_FUNCTIONS
#define ENERGY_FUNCTIONS

#include "../config/enums.h"
#include "../config/config.h"

extern DisplayValues gDisplayValues;

// ***********************************
// ** recherche du point 0. temps 20 ms max ... 
// ***********************************
void front() {
  int Watchdog=0 ;  
  
  int margin = 0; /// 0 marge de bruit de fond  en lecture ( si pb ) 
  // recherche demi ondulation
  while ( analogRead(ADC_PORTEUSE) > margin  ) { 
	delayMicroseconds (25);
    Watchdog++;
    if ( Watchdog > 500  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ? "); break; } 
	}

  Watchdog=0 ; 
  // recherche d'un 0
  while ( analogRead(ADC_PORTEUSE) == margin  ) {
	delayMicroseconds (5);
    Watchdog++;
    if ( Watchdog > 1500  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ou inversée ? "); break;}  
	}
    
}

// ***********************************
// ** boucle de syncho temps
// ***********************************

void rt_loop (unsigned long startmicro, unsigned long timer )
{

while ( micros() <=  ( startmicro + timer ) )
    {
    delayMicroseconds(1);
    }

 
}

// ***********************************
// recherche d'un niveau d'injection
// ***********************************

void injection(){
  int injection = 0; 
  //int margin = 1893;
  int temp_read , temp_porteuse ;
  int sigma_read = 0 ; 
  String porteuse = "" ;
  int loop = 0; 
  int freqmesure = 15;  // 18 mesure pour la 1/2 ondulation
  int wait_time = 555 ; //  --> 1/50Hz -> /2 -> 10ms --> 18 mesures --> 555 us 
  unsigned long startMillis;
  
  front();  ///synchro porteuse.
  
  startMillis = micros();

  while (loop < freqmesure ) {
    temp_read =  analogRead(ADC_INPUT);
    temp_porteuse =  analogRead(ADC_PORTEUSE);
    sigma_read += temp_read; 
    porteuse += String(temp_porteuse) + " " ; 

      //filtre bruit de fond 
   /* if ( temp_read > middle + BRUIT || temp_read < middle - BRUIT ) {
      if ( temp_read > middle )  { margin += ( temp_read - middle ) ; } 
      if ( temp_read < middle )  { margin += ( middle - temp_read ) ; }
    }*/ 
    

    ///detection injection 
   /* if ( temp_tension == 0 )  {  
     injection += temp - middle ;
    }*/

   loop ++; 
 
   rt_loop( startMillis, wait_time*loop ) ; 
  }

  //unsigned long endtMillis;
  //endtMillis = micros();
 // serial_println ("loop time ");
 // serial_println (endtMillis -startMillis  ) ;

  injection = sigma_read / (loop  ); 
  if (injection >= ADC_MIDDLE ) {gDisplayValues.injection = false ; }
  else {gDisplayValues.injection = true ; }
 // serial_print(porteuse) ; serial_print("  ") ; 
  //serial_println (injection) ;
}


#endif