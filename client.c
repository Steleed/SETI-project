#include <stdlib.h>
#include <stdio.h>
#include "project.h"

int main(){
    //Struct + socket per inviare messaggi sulla porta TCP
    struct sockaddr_in address_sock_tcp;
    address_sock_tcp.sin_family=AF_INET;
    address_sock_tcp.sin_port=htons(6769);
    //TODO prendere l'ip del server
    int sock1=socket(PF_INET, SOCK_STREAM, 0);

    //Creazione struct identificatore cliente
    client_id *id=malloc(sizeof(client_id));
    strncpy(id->ID, "A.Malesa", 8); //Identificativo client
    id->ID[8] = '\0';
    strncpy(id->PORT, "5000", 4); //Porta UDP client
    int p;
    do {
        scanf("Inserisci la password ((un numero compreso tra 0 e 65535)", &p);
    }
    while (!check_MPD(p));
    id->MDP=p; //Password client
    

    /*if (connect(sock, (struct sockaddr *)&address_sock_tcp, sizeof(address_sock_tcp))==EOF){
        perror("Errore di connessione");
        return EXIT_FAILURE;
    }*/

    int start=print_intro();
    switch (start)
        {
        case 1: 
            //TODO registrazione
            registration(sock1, id);
            break;
        case 2:
            //TODO registrazione
            connection(sock1, id);
            break;
        default:
            puts("ERRORE! Numero digitato fuori dal range consentito.");
            return EXIT_FAILURE;
        }

    int choice;
    while (1){
        choice=print_menu();
        switch (choice)
        {
        case 1: 
            //TODO amiciaiza
            continue;
        case 2:
            //TODO messaggio
            continue;
        default:
            puts("ERRORE! Numero fuori dai limiti.\nRitenta fra poco");
            sleep(2);
            continue;
        }
    }
    return EXIT_SUCCESS;
}