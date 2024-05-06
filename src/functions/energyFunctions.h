#ifndef ENERGY_FUNCTIONS
#define ENERGY_FUNCTIONS

#include "../config/enums.h"
#include "../config/config.h"

extern DisplayValues gDisplayValues;
extern Config config; 
extern Logs logging;


// ***********************************
// ** recherche du point 0. temps 20 ms max ... 
// ***********************************


void front() {
  int Watchdog=0 ;  
  gDisplayValues.porteuse = true; 
  int margin = 0; /// 0 marge de bruit de fond  en lecture ( si pb ) 
  // recherche demi ondulation
  while ( adc1_get_raw((adc1_channel_t)5) > margin  ) { 
  	delayMicroseconds (3);
    Watchdog++;
    if ( Watchdog > 2500  ) {  Serial.println(NO_SYNC); gDisplayValues.porteuse = false; break; } 

	}

  Watchdog=0 ; 
  // recherche d'un 0
  
  while ( adc1_get_raw((adc1_channel_t)5) == margin  ) {
  delayMicroseconds (3);  
    Watchdog++;
    if ( Watchdog > 2500  ) {  Serial.println(NO_SYNC); gDisplayValues.porteuse = false ; break;}  
	}
    
}

// ***********************************
// ** boucle de syncho temps
// ***********************************

void rt_loop (unsigned long startmicro, unsigned long timer )
{

while ( micros() < ( startmicro + timer ) )  /// a vérifier si < permetrait encore d'affiner la mesure 
    {
    delayMicroseconds(3);  // J'ai lu qu'en dessous de 3, les délais ne sont pas respecté.
    }

 
}

// ***********************************
// recherche d'un niveau d'injection
// ***********************************
int middle = 0;
int middle_count = 0; 

constexpr const int nbmesure = 72 ; /// nombre de mesure par ondulation
constexpr const int nombre_cycle  = 8 ; /// nombre de cycle pour affiner la mesure
const int freqmesure = nbmesure*(nombre_cycle+1) ;  // nombre total de mesures
int tableau[freqmesure]; // mesure ADC Ampères // NOSONAR
int porteuse[freqmesure]; // mesure ADC Volts // NOSONAR
int middle_debug ; 
int positive_debug ; 

void injection2(){ 

double temp_read ; 

  double sigma_read = 0 ; 

  int zero =0; 
  double positive = 0 ; 

  int loop = 0;  int inter=0;
  
  double wait_time = 277.77 ; //  --> 1/50Hz -> /2 -> 10ms --> 18 mesures --> 555 us 
  unsigned long startMillis; 
  unsigned long stopMillis;

front();  ///synchro porteuse.
delay (15);
startMillis = micros();   // 0ms 

///// construction du tableau de mesures  /////
while (loop < freqmesure  ) {
    tableau[loop] =  adc1_get_raw((adc1_channel_t)4);
    porteuse[loop] =  adc1_get_raw((adc1_channel_t)5);
    sigma_read += tableau[loop]; 
 
 
   loop ++; 
 
   rt_loop( startMillis, wait_time*loop ) ; // attend le prochain top temps de mesure ( 277us * loop )
   
  }
  ///// fin  construction du tableau de mesures  /////

// 20ms * nombre de cycles plus un certain nombre et le tout sur 8 cycles --> 160ms interessante pour 180 ms collectée 
stopMillis = micros();  

// début des calculs 

sigma_read = ( sigma_read / loop ) ;  /// calcul du sigma ( la valeur moyenne ), ne marche bien que dans le cas d'une ondulation symétrique. A voir si nouvelle méthode de calcul. ( valeur théorique : 2047 -> vcc/2)

int start=0; 


/// synchronisation sur la porteuse 
for(int i = 0; i < loop; i++)
{
  if ( start == 0 && porteuse[i] > 0 && i >= 1) { 
    start = (i-1) ; 
  } 
}   /// stable sur la carte din entre 18 et 21 


int phi = config.cosphi ;
if (phi > start ) { phi = start ; }
// 14 mesures de retard pour phi de 0.90 ( .451rad / (2Pi/72 ) ) 
// 8 pour cosphi ) 

for (int j = 0 ; j < nombre_cycle   ; j++) 
{
  for(int i = (start - phi ) ; i < ( start + nbmesure - phi )  ; i++)
  {
    if ( i <  ( start + ( nbmesure / 2 ) - phi ) ) {
    temp_read = (tableau[i+(j*nbmesure)]-sigma_read) ; 
    positive += temp_read  ; 
        if ( tableau[i+(j*nbmesure)] < 5 ) { zero ++ ;}    //détection des zeros ( prise débranchée )
    }
    else {
    temp_read = (sigma_read-tableau[i+(j*nbmesure)]) ; 
    positive += temp_read ; 
        if ( tableau[i+(j*nbmesure)] < 5 ) { zero ++ ;}  //détection des zeros ( prise débranchée )
    }
  }
}



/// A vide j'ai 20 ou -20 environ en fonction de comment est connecté la prise.



// info int base_offset = 15; offset du à la sonde et au montage ( composante continue mal filtrée) pour offset = 0 il faut mettre un condensateur de 10µF en //parallèle sur la sonde  (testé aussi avec 3.3µF)

int base_offset = 0; // ( testé sur 3 sondes différentes à vide )  --> la base d'offset change de sens en fonction de la phase de la prise
positive = ( ( positive * config.voltage  ) / ( FACTEURPUISSANCE * nombre_cycle * 230  ) )   ;  
/// correction pour l'offset en fonction de comment est branchée la pince


logging.clean_log_init();
if ( zero > 75 ) { 
  if (logging.sct) {    
    logging.Set_log_init("--> SCT013 Prob not connected  ?\r\n");
    logging.sct = false; }
}



bool nolog =false; 
if (nolog) {
Serial.println("fin tableau");

Serial.println("middle");

Serial.println(sigma_read);
Serial.println(start);
Serial.println(int(positive)) ;
Serial.println("valeur 20");
Serial.println(tableau[19]);
Serial.println(tableau[20]);
Serial.println(tableau[21]);

}
middle_debug= sigma_read; 



// correction sonde SCT
if ( config.polarity == true ) { 
  positive = ( ( positive + base_offset ) * config.SCT_13 / 30 + config.offset )  ; 
  }
else {
positive = -( ( positive - base_offset )* config.SCT_13 / 30 + config.offset )  ;
}

gDisplayValues.watt = int( positive ) ; 

}

#endif