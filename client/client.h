#ifndef CLIENT_H
#define CLIENT_H

#include "project.h"

// Struct identificativo client
typedef struct{
    char ID[LENGTH_ID+1]; //Nome identificativo
    char PORT[LENGTH_UDP_PORT+1];  //Porta UDP
    uint16_t MDP; //Password in little-endian
    int fdTCP; //Socket TCP
    int fdUDP; //Socket UCP
    u_int16_t num_notifications; //Numero notifiche udp
    int messLength; //Lunghezza messaggio da inviare ad un amico/flood
    pthread_mutex_t mtx; //Mutex per le notifiche UDP
    FILE *log; //File dove viene scritto il loge per ricezione messaggi
} client_id;

//Indirizzo ip del server
extern char ip_server[INET_ADDRSTRLEN];

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
int print_menu(bool *);

//Invia richiesta REGIS al server
void registration(client_id *);

//Invia richiesta CONNE al server 
void connection(client_id *);

//Legge i messaggi TCP inviati dal server
void read_all(client_id *, char *, int);

//Invia richiesta FRIE? al server
void friend(client_id *);

//Invia richiesta MESS? al server
void mess(client_id*);

//Invia richiesta FLOO? al server
void floo(client_id*);

//Invia richiesta LIST? al server
void list(client_id*);

//Legge la lista di utenti inviata dal server
void read_list(client_id *, int);

//Invia richiesta IQUIT al server
void iquit(client_id *);

//Invia richiesta CONSU al server
void consu(client_id*);

//Legge il flusso inviato dal server dopo CONSU
void read_consu(client_id*);

//Thread per ascoltare su porta UDP
void* udp_listen(void*);



#endif