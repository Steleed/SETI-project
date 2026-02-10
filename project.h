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
#define LENGTH_UDP_NOT 3 //Lunghezza (in byte) della notifica udp
#define MAX_PSWD 65535 //Numero massimo per password
#define END_SYMBOL "+++" //Simbolo terminale per messaggi TCP
#define LENGTH_END_SYMBOL 3 //Lunghezza simbolo terminale per messaggi TCP
#define LENGTH_HEADER 6 //Lunghezza instestazioni
#define REGIS_HEADER "REGIS " //Intestazione per la registrazione
#define LENGTH_REGIS 25 //Lunghezza messaggio registrazione
#define CONNE_HEADER "CONNE " //Intestazione per la connessione
#define LENGTH_CONNE 20 //Lunghezza messaggio connessione
#define FRIE_HEADER "FRIE? " //Intestazione per richiesta amicizia
#define LENGTH_FRIE 17 //Lunghezza messaggio amicizia
#define MAX_MESS 200 //Massima lunghezza messaggio da scrivere
#define MESS_HEADER "MESS? " //Intestazione per invio messaggio a un amico
#define LENGTH_MESS 18 //Lunghezza messaggio invio messaggio ad un amico senza contare la lunghezza del messaggio interno
#define FLOO_HEADER "FLOO? " //Intestazione per flooding
#define LENGTH_FLOO 9 //Lunghezza messaggio flooading senza contare la lunghezza del messaggio interno
#define LIST_HEADER "LIST?" //Intestazione per lista utenti
#define LENGTH_LIST 8 //Lunghezza messaggio per richiesta lista utenti
#define IQUIT_HEADER "IQUIT"  //Intestazione messaggio di disconnessione
#define LENGTH_IQUIT 8 //Lunghezza messaggio disconnessione

#define GOBYE_HEADER "GOBYE" //Header messaggio goodbye

#define LENGTH_RLIST 12 //Lunghezza massima messaggio di risposta a LIST?
#define FORMAT_RLIST "RLIST %3d+++" //Formato messaggio RLIST

#define LENGTH_LINUM 17 //Lunghezza messaggio LINUM

#endif