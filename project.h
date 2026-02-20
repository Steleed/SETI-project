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
#define FLUX_FRIE_HEADER "EIRF>" //Intestazione flusso richiesta amicizia
#define FORMAT_FLUX_FRIE "EIRF> %8s+++" //Formato flusso richiesta amicizia
#define FRIEN_HEADER "FRIEN" //Intestazione flusso accettazione richiesta amicizia
#define FORMAT_FRIEN "FRIEN %s+++" //Formato flusso accettazione richiesta amicizia
#define NOFRI_HEADER "NOFRI" //Intestazione flusso rifiuto richiesta amicizia
#define FORMAT_NOFRI "NOFRI %s+++" //Formato flusso rifiuto richiesta amicizia
#define OKIRF_HEADER "OKIRF" //Intestazione accettazione richiesta amicizia
#define FORMAT_OKIRF "OKIRF+++" //Formato accettazione richiesta amicizia
#define NOKRF_HEADER "NOKRF" //Intestazione rifiuto richiesta amicizia
#define FORMAT_NOKRF "NOKRF+++" //Formato rifiuto richiesta amicizia


#define MAX_MESS 200 //Massima lunghezza messaggio da scrivere
#define MESS_HEADER "MESS?" //Intestazione per invio messaggio a un amico
#define LENGTH_MESS 18 //Lunghezza messaggio invio messaggio ad un amico senza contare la lunghezza del messaggio interno
#define FORMAT_MESS "MESS? %8.8s %s+++"
#define FORMAT_OKMESS "MESS>+++" //Formato conferma invio messaggio
#define FORMAT_NOMESS "MESS<+++" //Formato errore invio messaggio
#define FLUX_MESS_HEADER "SSEM>" //Intestazione flusso invio messaggio
#define FORMAT_FLUX_MESS "SSEM> %8s %200s+++" //Formato flusso invio messaggio

#define FLOO_HEADER "FLOO?" //Intestazione per flooding
#define LENGTH_FLOO 9 //Lunghezza messaggio flooading senza contare la lunghezza del messaggio interno
#define FORMAT_FLOO "FLOO? %s+++" //Formato messaggio flooding
#define FLUX_FLOO_HEADER "OOLF>" //Intestazione flusso flooding
#define FORMAT_FLUX_FLOO "SSEM> %8s %200s+++" //Formato flusso flooding

#define LIST_HEADER "LIST?" //Intestazione per lista utenti
#define LENGTH_LIST 8 //Lunghezza messaggio per richiesta lista utenti
#define FORMAT_LIST "LIST?+++" //Formato messaggio richiesta lista utenti
#define RLIST_HEADER "RLIST " //Intestazione risposta lista utenti
#define LENGTH_RLIST 12 //Lunghezza massima messaggio RLIST
#define FORMAT_RLIST "RLIST %03d+++" //Formato messaggio RLIST
#define FORMAT_LINUM "LINUM %8s+++" //Formato messsaggio LINUM
#define LENGTH_LINUM 17 //Lunghezza messaggio LINUM

#define CONSU_HEADER "CONSU" //Intestazione messaggio consultazione notifiche
#define LENGTH_CONSU 8 //Lunghezza messaggio CONSU
#define FORMAT_CONSU "CONSU+++" //Formato messaggio consultazione
#define FORMAT_NOCON "NOCON++" //Formato per segnalare che non ci sono flussi

#define IQUIT_HEADER "IQUIT"  //Intestazione messaggio di disconnessione
#define LENGTH_IQUIT 8 //Lunghezza messaggio disconnessione
#define FORMAT_IQUIT "IQUIT+++" //Formato messaggio disconnessione


#endif