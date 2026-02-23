#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h> 
#include "project.h"      

//STRUTTURE DATI
//Coda messaggi
typedef struct FluxNode {
    char senderID[LENGTH_ID+1]; //Chi ha mandato il messaggio
    char content[500];   //Il testo del messaggio 
    char type[LENGTH_UDP_NOT+1]; //Tipo notifica: '3'=Messaggio, '1'=Amicizia OK, '0'=Richiesta Amicizia, ecc.
    struct FluxNode *next;     //Puntatore al prossimo messaggio
} FluxNode;

//Amici
typedef struct Friends{
    char friendID[LENGTH_ID+1]; //ID dell'amico
    int friend_index; //Index dell'amico dentro il database del server
    struct Friends* next; //Puntatore al prossimo amico
} Friends;

//Struttura Utente 
typedef struct {
    //Dati di Registrazione
    char ID[LENGTH_ID+1];            
    uint16_t password;                 
    char udpPort[LENGTH_UDP_PORT+1]; 
    struct sockaddr_in clientAddr; //IP utente
    int socketTCP;                 //> 0 se connesso, -1 se offline               
    //char friends[MAX_USERS][LENGTH_ID + 1]; //Lista degli ID degli amici
    Friends* friends;              //Testa della lista degli amici
    int friendsCount;              //Numero amici
    //Gestione Messaggi 
    FluxNode *pendingFluxes;  //Testa della lista dei flussi in attesa
    u_int16_t pendingCount;     //Numero flussi in coda 
    pthread_mutex_t userMutex;
} Client;

//VARIABILI GLOBALI
extern Client users[MAX_USERS];        //Il database in memoria
extern int registeredUsers;            //Quanti utenti ci sono ora
extern int socketUDP;                  //Socket UDP per mandare notifiche agli utenti
extern pthread_mutex_t usersListMutex; //Protegge l'aggiunta di nuovi utenti 

//FUNZIONI

//Inizializza tutte le strutture dati all'avvio del server
void init_server_structures();

//Funzione principale eseguita dal Thread per ogni client
void* client_handler(void* socket_desc);

//Funzione per fare il parsing dei messaggi ricevuti
int parser(char*, int);

//Funzione per mandare messaggi di risposta all'utente
void sendTCP(char *type, int size,  int sock);

//Funzione per inviare la notifica UDP al client
void send_udp_notification(int userIndex);

//Funzione per cercare un utente nell'array (restituisce l'indice o -1)
int find_user_index(char* id);

//Funzione per verificare la registrazione (restituisce l'indice o -1)
int regis(int, char*);

//Funzione per verificare la connessione (restituisce l'indice o -1)
int conne(int, char*);

//Funzione per verificare la richiesta d'amicizia
int frie(char*, int);

//Funzione per verificare l'invio di un messaggio a un amico
int mess(char* , int);

//Funzione che ritorna la lista di tutti gli utenti registrati
void list(int);

//Funzione per la disconnessione utente
void iquit(int, int);

//Funzione per consultazione notifiche
void consu(int, int);

//Funzione per registrare le amicizie
void add_friend(int, int);

//Funzione notifica richiesta d'amicizia
void insert_new_flux_frien(int, char*, char);
#endif