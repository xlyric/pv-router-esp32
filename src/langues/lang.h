#ifndef LANG
#define LANG

// Définissez la langue ici. Par exemple, LANG_FR pour le français, LANG_EN pour l'anglais.
//#define LANG_FR
//#define LANG_EN
//#define LANG_UA


#if defined(LANG_FR)
    #include "lang_fr.h"
#elif defined(LANG_EN)
    #include "lang_en.h"
#elif defined(LANG_UA)
    #include "lang_ua.h"
#else
    #error "Langue non définie ou non prise en charge"
    #include "lang_fr.h"
#endif

#endif
