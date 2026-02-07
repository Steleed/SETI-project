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
    pthread_mutex_t mtx; //Mutex per le notifiche UDP
} client_id;


//Controlla se l'utente ha inserito il numero giusto di argomenti
bool check_args(int, char*[], client_id*);

//Controlla se l'utente ha inserito la password nei limiti consentiti 
bool check_MPD(const int*);

//Crea una socket tcp e si connette al server, ritorna -1 in caso di errori
int tcpSock(client_id*);

//Crea una socket udp per ricevere notifiche dal server, ritorna -1 in caso di errori
int udpSock(client_id*);

//Stampa il messaggio di avvio e chiede all'utente di selezionare un'opzione
//Restituisce l'opzione (numero intero) selezionata
int print_intro();

//Stampa il menu e chiede all'utente di selezionare un'opzione
//Restituisce l'opzione (numero intero) selezionata
int print_menu();

//Invia richiesta REGIS al server
void registration(const client_id *);

//Invia richiesta CONNE al server 
void connection(const client_id *);

//Invia richiesta FRIE? al server
void friend(const client_id *);

//Thread per ascoltare su porta UDP
void* udp_listen(void*);



#endif