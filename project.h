#ifndef PROJECT_H
#define PROJECT_H

#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#define LENGTH_ID 8 //Lunghezza (in byte) dell' ID
#define MAX_UDP_PORT 9999 //Numero massimo di porta UDP
#define LENGTH_UDP_PORT 4 //Lunghezza (in byte) della porta UDP
#define LENGTH_UDP_NOT 3 //Lunghezza (in byte) della notifica udp
#define FORMAT_UDP_NOT "%d%c%c" //Formato notifica UDP
#define MAX_PSWD 65535 //Numero massimo per password
#define MAX_USERS 100 //Numero massimo utenti

#define WELCO_HEADER "WELCO" //Header messaggio welcome
#define FORMAT_WELCO "WELCO+++" //Formato messaggio welcome
#define GOBYE_HEADER "GOBYE" //Header messaggio goodbye
#define FORMAT_GOBYE "GOBYE+++" //Formato messaggio goodbye
#define FORMAT_HELLO "HELLO+++" //formato messaggio hello

#define END_SYMBOL "+++" //Simbolo terminale per messaggi TCP
#define LENGTH_END_SYMBOL 3 //Lunghezza simbolo terminale per messaggi TCP

#define LENGTH_HEADER 5 //Lunghezza instestazioni

#define REGIS_HEADER "REGIS" //Intestazione per la registrazione
#define LENGTH_REGIS 25 //Lunghezza messaggio registrazione
#define FORMAT_REGIS "REGIS %8.8s %4.4s %c%c+++"  //Formato messaggio registrazione

#define CONNE_HEADER "CONNE" //Intestazione per la connessione
#define LENGTH_CONNE 20 //Lunghezza messaggio connessione
#define FORMAT_CONNE "CONNE %8.8s %c%c+++" //Formato messaggio connessione

#define FRIE_HEADER "FRIE?" //Intestazione per richiesta amicizia
#define LENGTH_FRIE 17 //Lunghezza messaggio amicizia
#define FORMAT_FRIE "FRIE? %8.8s+++" //Formato messaggio richiesta amicizia
#define FORMAT_OKFRIE "FRIE>+++" //Formato conferma invio richiesta amicizia
#define FORMAT_NOFRIE "FRIE<+++" //Formato errore invio richiesta  amicizia
#define FRIE_NOT_UDP 0 //Numero notifica richiesta amicizia

#define MAX_MESS 200 //Massima lunghezza messaggio da scrivere
#define MESS_HEADER "MESS?" //Intestazione per invio messaggio a un amico
#define LENGTH_MESS 18 //Lunghezza messaggio invio messaggio ad un amico senza contare la lunghezza del messaggio interno
#define FORMAT_MESS "MESS? %8.8s %s+++"
#define FORMAT_OKMESS "MESS>+++" //Formato conferma invio messaggio
#define FORMAT_NOMESS "MESS<+++" //Formato errore invio messaggio

#define FLOO_HEADER "FLOO?" //Intestazione per flooding
#define LENGTH_FLOO 9 //Lunghezza messaggio flooading senza contare la lunghezza del messaggio interno
#define FORMAT_FLOO "FLOO? %s+++" //Formato messaggio flooding

#define LIST_HEADER "LIST?" //Intestazione per lista utenti
#define LENGTH_LIST 8 //Lunghezza messaggio per richiesta lista utenti
#define FORMAT_LIST "LIST?+++" //Formato messaggio richiesta lista utenti
#define RLIST_HEADER "RLIST " //Intestazione risposta lista utenti
#define LENGTH_RLIST 12 //Lunghezza massima messaggio RLIST
#define FORMAT_RLIST "RLIST %3d+++" //Formato messaggio RLIST
#define LENGTH_LINUM 17 //Lunghezza messaggio LINUM

#define CONSU_HEADER "CONSU" //Intestazione messaggio consultazione notifiche

#define IQUIT_HEADER "IQUIT"  //Intestazione messaggio di disconnessione
#define LENGTH_IQUIT 8 //Lunghezza messaggio disconnessione
#define FORMAT_IQUIT "IQUIT+++" //Formato messaggio disconnessione


#endif