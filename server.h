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