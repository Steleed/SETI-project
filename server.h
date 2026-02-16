#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdbool.h>
#include <netinet/in.h> // Serve per struct sockaddr_in
#include "project.h"    // Include le costanti comuni (LENGTH_ID, ecc.)

#define MAX_USERS 100   // Limite massimo imposto dalle specifiche

// --- STRUTTURE DATI ---

// 1. Nodo per la lista dei messaggi in attesa (Coda)
//    Questa è la "cassetta della posta" di ogni utente.
typedef struct MessageNode {
    char senderID[LENGTH_ID + 1]; // Chi ha mandato il messaggio
    char content[MAX_MESS + 1];   // Il testo del messaggio (o nullo se non serve)
    char type;                    // Tipo notifica: '3'=Messaggio, '1'=Amicizia OK, '0'=Richiesta Amicizia, ecc.
    struct MessageNode *next;     // Puntatore al prossimo messaggio
} MessageNode;

// 2. Struttura Utente (Il registro completo)
typedef struct {
    // Dati di Registrazione
    char ID[LENGTH_ID + 1];            
    uint16_t password;                 
    char udpPort[LENGTH_UDP_PORT + 1]; 
    
    // Dati di Connessione
    struct sockaddr_in clientAddr; // Importante: Ci serve per sapere l'IP a cui mandare le notifiche UDP!
    int socketTCP;                 // > 0 se connesso, -1 se offline
    bool isOnline;                 
    
    // Gestione Amicizie
    char friends[MAX_USERS][LENGTH_ID + 1]; // Lista degli ID degli amici
    int friendsCount;
    
    // Gestione Messaggi (I Flussi)
    MessageNode *pendingMessages;  // Testa della lista concatenata (Coda)
    int pendingCount;              // Numero messaggi in coda (per la notifica UDP)

    // Gestione Stato "Bloccato" (Per specifica CONSU su richiesta amicizia)
    // Se true, l'utente DEVE rispondere alla richiesta prima di fare altro.
    bool isBlocked;                
    char blockedByUserID[LENGTH_ID + 1]; // L'ID di chi ha mandato la richiesta che blocca

    // Thread Safety
    pthread_mutex_t userMutex;     // Protegge i dati di QUESTO utente (es. coda messaggi)
} Client;

// --- VARIABILI GLOBALI (Definite poi in server.c) ---
extern Client users[MAX_USERS];        // Il database in memoria
extern int registeredUsers;            // Quanti utenti ci sono ora
extern pthread_mutex_t usersListMutex; // Protegge l'aggiunta di nuovi utenti (registro globale)

// --- PROTOTIPI FUNZIONI ---

// Inizializza tutte le strutture dati all'avvio del server
void init_server_structures();

// La funzione principale eseguita dal Thread per ogni client
void* client_handler(void* socket_desc);

// Funzione per inviare la notifica UDP (il "BIP") a un utente
void send_udp_notification(int userIndex);

// Funzione per cercare un utente nell'array (restituisce l'indice o -1)
int find_user_index(char* id);

#endif