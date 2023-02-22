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
unsigned long startMillis,stopMillis;

// void front() {
//   int Watchdog=0 ;  
//   gDisplayValues.porteuse = true; 
//   int margin = 0; /// 0 marge de bruit de fond  en lecture ( si pb ) 
//   // recherche demi ondulation
//   while ( analogRead(ADC_PORTEUSE) > margin  ) { 
//   	delayMicroseconds (3);
//     Watchdog++;
//     if ( Watchdog > 2500  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ? "); gDisplayValues.porteuse = false; break; } 

// 	}

//   Watchdog=0 ; 
//   // recherche d'un 0
  
//   while ( analogRead(ADC_PORTEUSE) == margin  ) {
//   delayMicroseconds (3);  
//     Watchdog++;
//     if ( Watchdog > 2500  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ou inversée ? "); gDisplayValues.porteuse = false ; break;}  
// 	}
    
// }




// ***********************************
// ** boucle de syncho temps
// ***********************************

void rt_loop (unsigned long startmicro, unsigned long timer )
{

while ( micros() <= ( startmicro + timer ) )  /// a vérifier si < permetrait encore d'affiner la mesure 
    {
    delayMicroseconds(3);  // J'ai lu qu'en dessous de 3, les délais ne sont pas respecté.
    }

 
}

// ***********************************
// recherche d'un niveau d'injection
// ***********************************
int middle = 0;
int middle_count = 0; 
int start = 0 ;
#ifdef OLD
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
#endif

const int nbmesure = 72 ; /// nombre de mesure par ondulation
const int nombre_cycle  = 4 ; /// nombre de cycle pour affiner la mesure
const int freqmesure = nbmesure*(nombre_cycle+1) ;  // nombre total de mesures
int tableau[freqmesure]; // mesure ADC Ampères
int porteuse[freqmesure]; // mesure ADC Volts
int middle_debug ; 
int positive_debug ; 

//float constante_voltage = 4.33; 



// void injection2(){ 

// int temp_read ; 
// // int temp_porteuse ;
//   double sigma_read = 0 ; 
//   //double voltage = 0; // test de calcul de voltage
//   //String porteuse = "" ;
//   int zero =0; 
//   double positive = 0 ; 
//   //int zero_count = 0; 
//   int loop = 0;  int inter=0;
  
//   int wait_time = 277 ; //  --> 1/50Hz -> /2 -> 10ms --> 18 mesures --> 555 us 
//   unsigned long startMillis,stopMillis;
//   //int injection = 0; 
//  // int nbmesure=72 ;  /// nombre de mesure par ondulation
//  // int nombre_cycle = 4 ;  
//  // int freqmesure = nbmesure*(nombre_cycle+1) ;  // nombre total de mesures


// front();  ///synchro porteuse.
// delay (15);
// startMillis = micros();   // 0ms 

// ///// construction du tableau de mesures  /////
// while (loop < freqmesure ) {
//     tableau[loop] =  analogRead(ADC_INPUT);
//     porteuse[loop] =  analogRead(ADC_PORTEUSE);
//     sigma_read += tableau[loop]; 
//     // voltage += porteuse[loop]; // test de calcul de voltage
    
// /*    if (temp_porteuse == 0 ) { // 2eme 1/2 mesure  10ms 
//       zero += temp_read * temp_read ; 
//       zero_count ++; 
//       injection = 1 ;
//     }
//     else {  // 1ere 1/2 mesure  10ms 
//       if ( injection == 1 ) { injection =2 ; break ; }
//       positive += temp_read * temp_read ;
//       inter=loop;
//     }
// */
// /*  if ( loop < 37 ) {
//      positive += temp_read * temp_read ;
//      inter=loop;
//   }
//   else { 
//        zero += temp_read * temp_read ; 
//       zero_count ++; 
//    //   injection = 1 ;
//   }
// */
 
//    loop ++; 
 
//    rt_loop( startMillis, wait_time*loop ) ; // attend le prochain top temps de mesure ( 277us * loop )
   
//   }
//   ///// fin  construction du tableau de mesures  /////

// // 20ms * nombre de cycles -> total 72 mesures * nb cycles
// stopMillis = micros();  

// // début des calculs 

// sigma_read = ( sigma_read / ( loop +1 ) ) ;  /// calcul du sigma ( la valeur moyenne ), ne marche bien que dans le cas d'une ondulation symétrique. A voir si nouvelle méthode de calcul. ( valeur théorique : 2047 -> vcc/2)
// // voltage = ( voltage / (loop + 1 )) ; // test de calcul de voltage
// int start=0; 

// // int voltage_read = int(voltage/constante_voltage) ; // test de calcul de voltage ... pas de réel variation 

// //// test de calcul de voltage
// /*Serial.println("voltage");
// Serial.println(voltage);
// Serial.println(voltage_read);*/



// for(int i = 0; i < loop; i++)
// {
//   if ( start == 0 ) {  if ( porteuse[i] > 0 ) { 
//     start = i ; 
//      //if ( voltage_read < porteuse[i] ) {voltage_read = porteuse[i] ; }
//     //Serial.println(String(tableau[i])); }
//     }   
//   //else { 
//   //Serial.println(String(tableau[i])); 
//   } 

// }
// ///Serial.println (voltage_read);

// // int phi = config.cosphi ;
// int phi = 5; //config.cosphi ;
// if (phi > start ) { phi = start ; }


// for (int j = 0 ; j < nombre_cycle   ; j++) 
// {
//   for(int i = (start - phi ) ; i < ( start + nbmesure - phi )  ; i++)
//   {
//     if ( i <  ( start + ( nbmesure / 2 ) - phi ) ) {
//     temp_read = (tableau[i+(j*nbmesure)]-sigma_read) ; 
//     positive += temp_read  ; 
//         if ( tableau[i+(j*nbmesure)] < 5 ) { zero ++ ;}    //détection des zeros ( prise débranchée )
//     }
//     else {
//     temp_read = (sigma_read-tableau[i+(j*nbmesure)]) ; 
//     positive += temp_read ; 
//         if ( tableau[i+(j*nbmesure)] < 5 ) { zero ++ ;}  //détection des zeros ( prise débranchée )
//     }
//   }
// }


// //Serial.println(int(positive/10000*voltage_read)) ;// test de calcul de voltage



// //positive = ( positive / ( FACTEURPUISSANCE * nombre_cycle * 230 / config.voltage ) ) + config.offset ; 
// positive = ( ( positive * config.voltage ) / ( 10.5 * nombre_cycle * 230 ) ) + config.offset ; 


// if ( zero > 75 ) { 
//   if (logging.sct) { logging.start += "--> SCT013 Prob not connected  ?\r\n" ; logging.sct = false; }
// }
// //logging.start += "zero detected : " + String(zero) +   "\r\n" ;


// bool nolog =false; 
// if (nolog) {
// Serial.println("fin tableau");
// Serial.println("temp");
// Serial.println(wait_time*loop);
// Serial.println(stopMillis-startMillis);
// Serial.println("inter");
// Serial.println(inter*wait_time);
// Serial.println("middle");
// Serial.println(loop);
// Serial.println(sigma_read);
// Serial.println(start);
// Serial.println(int(positive)) ;
// }
// middle_debug= sigma_read; 

// //zero = sqrt(zero / float(zero_count)); 
// // positive = sqrt(positive / float( loop - zero_count )) ;

// gDisplayValues.watt = int(( positive )) ; 
// if ( config.polarity == true ) { gDisplayValues.watt = - gDisplayValues.watt ; }




// }
int startV;
double Vrms;
double Irms;

void frontmod(){
  int Watchdog = 0;
      int temp_value0 = analogRead(ADC_MIDDLE);  //Mean value. Should be at 3.3v/2
      value0 = value0 + PHASECAL * (temp_value0 - value0); // Lissage des valeurs

      startV = analogRead(ADC_PORTEUSE);

  while ( startV< (value0*0.95 )) { 
      delayMicroseconds (5);
      startV = analogRead(ADC_PORTEUSE);
      Watchdog++;
      if ( Watchdog > 2000  ) {  Serial.print("Attention pas de porteuse, alimentation 12v AC ou pont redresseur débranché ? "); gDisplayValues.porteuse = false; break; } 

    }
    startMillis = micros();   // 0ms 
    while(1)                                   //the while loop...
    {
      // if ((startV < (ADC_COUNTS*0.50)) && (startV > (ADC_COUNTS*0.45))) 
      if ((startV < value0) && (startV > (value0*0.95))) 

      {
    //   logging.start +="porteuse OK \r\n" ;

        gDisplayValues.porteuse = true; 
        break;  //check its within range
      }
      startV = analogRead(ADC_PORTEUSE);                    //using the voltage waveform

      if ((millis()-startMillis)>timeout) {Serial.print("Attention pas milieu trouvé, alimentation 12v AC ou pont redresseur débranché ? ");  break;}
    }
}

void injection3(){ 
double temp_read;
double temp_tension;
unsigned int crossCount = 0;                             //Used to measure number of times threshold is crossed.
unsigned int numberOfSamples = 0;                        //This is now incremented

int wait_time = 277 ; //  --> 1/50Hz -> /2 -> 10ms --> 18 mesures --> 555 us 
unsigned long startMillis;


bool lastVCross; 
bool checkVCross;                  //Used to measure number of times threshold is crossed.

Irms = 0;
Vrms = 0;
PW = 0;
frontmod();

startMillis = micros();   // 0ms 
while ((crossCount <= (nombre_cycle*2))) // && ((millis()-startMillis)<timeout))
{
  numberOfSamples++;                       //Count number of times looped.
///// construction du tableau de mesures  /////

    temp_read =  analogRead(ADC_INPUT) - value0;
    temp_tension = analogRead(ADC_PORTEUSE) - value0;
    
    if (crossCount>0) // On oublie volontairement le 1er cycle
    {
      temp_read = temp_read / config.facteur ; 
      Irms += sq(temp_read);

      temp_tension = temp_tension / config.voltage;
      Vrms += sq(temp_tension); // carré des mesures

      PW += temp_tension * temp_read;
    }

    lastVCross = checkVCross;
    if (temp_tension >= 0) {
      checkVCross = true;}
    else {checkVCross = false;}
    if (numberOfSamples==1) lastVCross = checkVCross;
    
    if (lastVCross != checkVCross) {
      crossCount++;
      if (crossCount ==1) {
        numberOfSamples = 0;
        startMillis = micros();   // 0ms 
      }
    }
    rt_loop( startMillis, (wait_time * numberOfSamples) ) ; // attend le prochain top temps de mesure ( 277us * loop )
  }
  ///// fin  construction du tableau de mesures  /////

Vrms = Vrms / numberOfSamples;
Vrms = sqrt(Vrms);
Vrms = VrmsOLD + PHASECAL * (Vrms - VrmsOLD); // Lissage des valeurs
VrmsOLD = Vrms;

Irms = Irms / numberOfSamples;
Irms = sqrt(Irms);

PVA = floor(Vrms * Irms);

PW =  (PW/numberOfSamples) + config.offset ; 

PowerFactor = floor(100 * abs(PW) / PVA) / 100;

// positive = PW ; 

middle_debug= value0; 

gDisplayValues.watt = int( PW ) ; 
if ( config.polarity == true ) { gDisplayValues.watt = - gDisplayValues.watt ; }

}


#endif