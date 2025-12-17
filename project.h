#ifndef PROJECT_H
#define PROJECT_H

#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Struct identificativo client
typedef struct{
    char ID[9]; //Nome identificativo
    char PORT[5];  //Porta UDP
    uint16_t MDP; //Password
} client_id;

#define LENGTH_ID 8 //Lunghezza (in byte) dell' ID
#define MAX_UDP_PORT 9999 //Numero massimo di porta UDP
#define LENGTH_UDP_PORT 4 //Lunghezza (in byte) della porta UDP
#define MAX_PSWD 65535 //Numero massimo per password
#define END_SYMBOL "+++" //Simbolo terminale per messaggi TCP
#define LENGTH_END_SYMBOL 3 //Lunghezza simbolo terminale per messaggi TCP
#define LENGTH_HEADER 6 //Lunghezza instestazioni
#define REGIS_HEADER "REGIS " //Intestazione per la registrazione
#define CONNE_HEADER "CONNE " //Intestazione per la connessione

#endif