#include <stdlib.h>
#include <stdio.h>
#include "client.h"


int main(int argc, char* argv[]){
    //Creazione struct identificatore cliente
    client_id *id=malloc(sizeof(client_id));
    if (!check_args(argc, argv, id)){
        fprintf(stderr, "Errore argomenti\n\n\t-i: IMMETTI IL TUO ID (8 caratteri)\n\n\t-p: IMMETTI LA TUA PORTA UDP (4 caratteri, inferiore a 9999,\n\t  completata con degli 0 all'inizio se necessario)\n");
        free(id);
        return EXIT_FAILURE;
    }
    int p;
    do {
        printf("Inserisci la password ((un numero compreso tra 0 e 65535): ");
        scanf("%d", &p);
    }
    while (!check_MPD(&p));
    id->MDP=htole16(p); //Password client
    id->notifications=0;
    system("clear");
    

    //Socket + connessione per inviare messaggi sulla porta TCP
    if (tcpSock(id) == -1)  return EXIT_FAILURE;
    //Socket + binding per ricevere notifiche su porta UDP
    if (udpSock(id) == -1)  return EXIT_FAILURE;
    pthread_mutex_init(&id->mtx, NULL); //Inizializzazione mutex

    int start=print_intro();
    switch (start)
        {
        case 1: 
            //TODO registrazione
            registration(id);
            break;
        case 2:
            //TODO registrazione
            connection(id);
            break;
        case 3:
            free(id);
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "ERRORE! Numero digitato fuori dal range consentito.\n");
            free(id);
            return EXIT_FAILURE;
        }

    pthread_t th1;
    pthread_create(&th1,NULL,udp_listen,id);
 
    sleep(2);
    system("clear");
    int choice;
    while (1){            
        //!Mutex?
        //Controllo notifiche udp
        pthread_mutex_lock(&id->mtx);
        if (id->notifications>0){
            printf("Hai %d notifiche\n", id->notifications);
        }
        pthread_mutex_unlock(&id->mtx);
        sleep(2);
        system("clear");

        choice=print_menu();
        switch (choice)
        {
        case 1: 
            //TODO amiciaiza
            friend(id);
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