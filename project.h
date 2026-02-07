#ifndef PROJECT_H
#define PROJECT_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define LENGTH_ID 8 //Lunghezza (in byte) dell' ID
#define MAX_UDP_PORT 9999 //Numero massimo di porta UDP
#define LENGTH_UDP_PORT 4 //Lunghezza (in byte) della porta UDP
#define MAX_PSWD 65535 //Numero massimo per password
#define END_SYMBOL "+++" //Simbolo terminale per messaggi TCP
#define LENGTH_END_SYMBOL 3 //Lunghezza simbolo terminale per messaggi TCP
#define LENGTH_HEADER 6 //Lunghezza instestazioni
#define REGIS_HEADER "REGIS " //Intestazione per la registrazione
#define LENGTH_REGIS 25 //Lunghezza messaggio registrazione
#define CONNE_HEADER "CONNE " //Intestazione per la connessione
#define LENGTH_CONNE 20 //Lunghezza messaggio connessione
#define FRIE_HEADER "FRIE? "
#define LENGTH_FRIE 17 //Lunghezza messaggio amicizia

#endif