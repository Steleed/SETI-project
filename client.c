#include "client.h"

char ip_server[INET_ADDRSTRLEN];

int main(int argc, char* argv[]){
    //Creazione struct identificatore cliente
    client_id *id=malloc(sizeof(client_id));
    if (!check_args(argc, argv, id)){
        fprintf(stderr, "Errore argomenti\n\n\t-i: IMMETTI IL TUO ID (8 caratteri)\n\n\t-p: IMMETTI LA TUA PORTA UDP (4 caratteri, inferiore a 9999,\n\t  completata con degli 0 all'inizio se necessario)\n");
        free(id);
        return EXIT_FAILURE;
    }
    int p;
    char *tmp=malloc(100*sizeof(char));
    do {
        printf("Inserisci la password ((un numero compreso tra 0 e 65535): ");
        fgets(tmp, 100, stdin);
        p=atoi(tmp);
    }
    while (!check_MPD(&p));
    free(tmp);
    id->MDP=htole16(p); //Password client
    id->num_notifications=0; //Numero di notifiche ricevute
    id->notifications=NULL; //Lista notifiche ricevute
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
            registration(id);
            break;
        case 2:
            connection(id);
            break;
        case 3:
            iquit(id);
            break;
        default:
            fprintf(stderr, "ERRORE! Numero digitato fuori dal range consentito.\n");
            free(id);
            return EXIT_FAILURE;
        }

    pthread_t th1;
    pthread_create(&th1,NULL,udp_listen,id);
    pthread_detach(th1);
 
    int choice;
    while (1){    
        sleep(1);
        system("clear");        
        //Controllo notifiche udp
        pthread_mutex_lock(&id->mtx);
        if (id->num_notifications>0){
            printf("Hai %d notifiche\n", id->num_notifications);
        }
        pthread_mutex_unlock(&id->mtx);
        sleep(1);
        system("clear");

        choice=print_menu();
        switch (choice)
        {
        case 1: 
            friend(id);
            break;
        case 2:
            mess(id);
            break;
        case 3:
            floo(id);
            break;
        case 4:
            list(id);
            break;
        case 5:
            consu(id);
            break;
        case 6:
            iquit(id);
            break;
        default:
            puts("ERRORE! Numero fuori dai limiti.\nRitenta fra poco");
            break;
        }
    }
    return EXIT_SUCCESS;
}