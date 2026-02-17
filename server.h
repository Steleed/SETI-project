#ifndef SERVER_H
#define SERVER_H
#include <netinet/in.h> 
#include "project.h"      

//STRUTTURE DATI
//Coda messaggi
typedef struct MessageNode {
    char senderID[LENGTH_ID+1]; //Chi ha mandato il messaggio
    char content[MAX_MESS+1];   //Il testo del messaggio 
    char type;                    //Tipo notifica: '3'=Messaggio, '1'=Amicizia OK, '0'=Richiesta Amicizia, ecc.
    struct MessageNode *next;     //Puntatore al prossimo messaggio
} MessageNode;

//Struttura Utente 
typedef struct {
    //Dati di Registrazione
    char ID[LENGTH_ID+1];            
    uint16_t password;                 
    char udpPort[LENGTH_UDP_PORT+1]; 
    struct sockaddr_in clientAddr; //IP utente
    int socketTCP;                 //> 0 se connesso, -1 se offline               
    char friends[MAX_USERS][LENGTH_ID + 1]; //Lista degli ID degli amici
    int friendsCount;              //Numero amici
    //Gestione Messaggi 
    MessageNode *pendingMessages;  //Testa della lista concatenata
    int pendingCount;              //Numero messaggi in coda 
    pthread_mutex_t userMutex;
} Client;

//VARIABILI GLOBALI
extern Client users[MAX_USERS];        //Il database in memoria
extern int registeredUsers;            //Quanti utenti ci sono ora
extern pthread_mutex_t usersListMutex; //Protegge l'aggiunta di nuovi utenti 

//FUNZIONI

//Inizializza tutte le strutture dati all'avvio del server
void init_server_structures();
//Funzione principale eseguita dal Thread per ogni client
void* client_handler(void* socket_desc);
//Funzione per inviare la notifica UDP al client
void send_udp_notification(int userIndex);
//Funzione per cercare un utente nell'array (restituisce l'indice o -1)
int find_user_index(char* id);
#endif