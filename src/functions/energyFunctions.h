#ifndef ENERGY_FUNCTIONS
#define ENERGY_FUNCTIONS

#include "../config/enums.h"
#include "../config/config.h"

extern DisplayValues gDisplayValues;
extern Config config; 

// ***********************************
// ** recherche du point 0. temps 20 ms max ... 
// ***********************************
void front() {
  int Watchdog=0 ;  
  gDisplayValues.porteuse = true; 
  int margin = 0; /// 0 marge de bruit de fond  en lecture ( si pb ) 
  // recherche demi ondulation
  while ( analogRead(ADC_PORTEUSE) > margin  ) { 
  	delayMicroseconds (25);
    Watchdog++;
    if ( Watchdog > 500  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ? "); gDisplayValues.porteuse = false; break; } 

	}

  Watchdog=0 ; 
  // recherche d'un 0
  while ( analogRead(ADC_PORTEUSE) == margin  ) {
	delayMicroseconds (5);
    Watchdog++;
    if ( Watchdog > 1500  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ou inversée ? "); gDisplayValues.porteuse = false ; break;}  
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
int middle = 0;
int middle_count = 0; 

void injection(){
  int injection = 0; 
  //int margin = 1893;
  int temp_read , temp_porteuse ;
  int sigma_read = 0 ; 
  String porteuse = "" ;
  double zero =0; 
  double positive = 0 ; 
  int zero_count = 0; 
  int loop = 0; 
  int freqmesure = 144;  // 18 mesure pour la 1/2 ondulation
  int wait_time = 277 ; //  --> 1/50Hz -> /2 -> 10ms --> 18 mesures --> 555 us 
  unsigned long startMillis;
  
  front();  ///synchro porteuse.
  
  startMillis = micros();

  while (loop < freqmesure ) {
    temp_read =  analogRead(ADC_INPUT);
    temp_porteuse =  analogRead(ADC_PORTEUSE);
    sigma_read += temp_read; 

    if (temp_porteuse == 0 ) {
      zero += temp_read * temp_read ; 
      zero_count ++; 
      injection = 1 ;
    }
    else {
      if ( injection == 1 ) { injection =2 ; break ; }
      positive += temp_read * temp_read ;

    }



    //porteuse += String(temp_porteuse) + " " +  String(temp_read) + "-"; 
 
      //filtre bruit de fond 
   /* if ( temp_read > middle + BRUIT || temp_read < middle - BRUIT ) {
      if ( temp_read > middle )  { margin += ( temp_read - middle ) ; } 
      if ( temp_read < middle )  { margin += ( middle - temp_read ) ; }
    }*/ 
    

    ///detection injection 
   /* if ( temp_tension == 0 )  {  
     injection += temp - middle ;
    }*/
   if ( injection == 2  || loop > 500) { break ; }
   loop ++; 
 
   rt_loop( startMillis, wait_time*loop ) ; 
  }

  zero = sqrt(zero / float(zero_count)); 
  positive = sqrt(positive / float( loop - zero_count )) ; 
  //serial_print(positive) ; serial_print("  ") ; serial_print (zero) ; serial_print("  ") ; serial_println ( (zero + positive) /2  ) ;
  gDisplayValues.watt = int(( positive - zero)*3.2) ; 
  if ( config.polarity == true ) { gDisplayValues.watt = - gDisplayValues.watt ; }
  

  //injection = sigma_read / (loop  ); 
  //if (injection >= ADC_MIDDLE ) 
  //if (positive >= zero ) {gDisplayValues.injection = false ; }
  //else {gDisplayValues.injection = true ;  serial_print(porteuse) ; serial_print("  ") ; serial_print (injection) ; serial_print("  ") ; serial_println (loop) ;}
 
  


}




#endif