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
  //int tableau[144][144]; 
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
  
  startMillis = micros();   //temps debut de la boucle

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
 
   rt_loop( startMillis, wait_time*loop ) ; // attent le prochain top temps de mesure ( 277us * loop )
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


void injection2(){ 

int temp_read , temp_porteuse ;
  double sigma_read = 0 ; 
  //String porteuse = "" ;
  double zero =0; 
  double positive = 0 ; 
  int zero_count = 0; 
  int loop = 0;  int inter=0;
  
  int wait_time = 277 ; //  --> 1/50Hz -> /2 -> 10ms --> 18 mesures --> 555 us 
  unsigned long startMillis,stopMillis;
  int injection = 0; 
  int nbmesure=72 ;  /// nombre de mesure par ondulation
  int nombre_cycle = 4 ;  /// nombre de cycle pour affiner la mesure
  int freqmesure = nbmesure*(nombre_cycle+1) ;  // 18 mesure pour la 1/2 ondulation
  int tableau[freqmesure];
  int porteuse[freqmesure];

front();  ///synchro porteuse.
delay (15);
startMillis = micros();   // 0ms 


while (loop < freqmesure ) {
    tableau[loop] =  analogRead(ADC_INPUT);
    porteuse[loop] =  analogRead(ADC_PORTEUSE);
    sigma_read += tableau[loop]; 
    
/*    if (temp_porteuse == 0 ) { // 2eme 1/2 mesure  10ms 
      zero += temp_read * temp_read ; 
      zero_count ++; 
      injection = 1 ;
    }
    else {  // 1ere 1/2 mesure  10ms 
      if ( injection == 1 ) { injection =2 ; break ; }
      positive += temp_read * temp_read ;
      inter=loop;
    }
*/
/*  if ( loop < 37 ) {
     positive += temp_read * temp_read ;
     inter=loop;
  }
  else { 
       zero += temp_read * temp_read ; 
      zero_count ++; 
   //   injection = 1 ;
  }
*/
 
   loop ++; 
 
   rt_loop( startMillis, wait_time*loop ) ; // attend le prochain top temps de mesure ( 277us * loop )
   
  }
// 20ms total 72 mesures 
stopMillis = micros();  

// début des calculs 
sigma_read = ( sigma_read / ( loop +1 ) ) ; 
int start=0; 

//Serial.println("tableau");

for(int i = 0; i < loop; i++)
{
  if ( start == 0 ) {  if ( porteuse[i] > 0 ) { 
    start = i ; 
    //Serial.println(String(tableau[i])); }
    }   
  //else { 
  //Serial.println(String(tableau[i])); 
  } 

}

int phi = config.cosphi ;
if (phi > start ) { phi = start ; }


for (int j = 0 ; j < nombre_cycle   ; j++) 
{
  for(int i = (start - phi ) ; i < ( start + nbmesure - phi )  ; i++)
  {
    if ( i <  ( start + ( nbmesure / 2 ) - phi ) ) {
    temp_read = (tableau[i+(j*nbmesure)]-sigma_read) ; 
    positive += temp_read  ; 
    }
    else {
    temp_read = (sigma_read-tableau[i+(j*nbmesure)]) ; 
    positive += temp_read ; 
    }
  }
}

positive = positive / ( FACTEURPUISSANCE * nombre_cycle ) ; 



Serial.println("fin tableau");
Serial.println("temp");
Serial.println(wait_time*loop);
Serial.println(stopMillis-startMillis);
Serial.println("inter");
Serial.println(inter*wait_time);
Serial.println("middle");
Serial.println(loop);
Serial.println(sigma_read);
Serial.println(start);
//zero = sqrt(zero / float(zero_count)); 
// positive = sqrt(positive / float( loop - zero_count )) ;
Serial.println(int(positive)) ;
gDisplayValues.watt = int(( positive )) ; 
if ( config.polarity == true ) { gDisplayValues.watt = - gDisplayValues.watt ; }

}

#endif