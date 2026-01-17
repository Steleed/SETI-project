#ifndef CLIENT_H
#define CLIENT_H

#include <stdbool.h>

// Struct identificativo client
typedef struct{
    char ID[9]; //Nome identificativo
    char PORT[5];  //Porta UDP
    int MDP; //Password
} client_id;

//Controlla se l'utente ha inserito il numero giusto di argomenti
bool check_args(int, char*[]);

//Controlla se l'utente ha inserito la password nei limiti consentiti 
bool check_MPD(const int);



#endif