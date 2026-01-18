#ifndef CLIENT_H
#define CLIENT_H

#include "project.h"

// Struct identificativo client
typedef struct{
    char ID[9]; //Nome identificativo
    char PORT[5];  //Porta UDP
    uint16_t MDP; //Password in little-endian
    int fdTCP; //Socket TCP
    int fdUDP; //Socket TCP
    int notifications; //Numero notifiche udp
} client_id;


//Controlla se l'utente ha inserito il numero giusto di argomenti
bool check_args(int, char*[], client_id*);

//Controlla se l'utente ha inserito la password nei limiti consentiti 
bool check_MPD(const int*);

//Stampa il messaggio di avvio e chiede all'utente di selezionare un'opzione
//Restituisce l'opzione (numero intero) selezionata
int print_intro();

//Stampa il menu e chiede all'utente di selezionare un'opzione
//Restituisce l'opzione (numero intero) selezionata
int print_menu();

//Costruisce il messaggio di registrazione REGIS␣id␣port␣mdp+++
char* build_message_reg(const client_id *);

//Invia richiesta REGIS al server
void registration(const client_id *);

//Costruisce il messaggio di registrazione REGIS␣id␣port␣mdp+++
char *build_message_conn(const client_id *);

//Invia richiesta REGIS al server CONNE
void connection(const client_id *);

//Thread per ascoltare su porta UDP
void* udp_listen(void*);



#endif