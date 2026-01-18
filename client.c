#include <stdlib.h>
#include <stdio.h>
#include "client.h"


int main(int argc, char* argv[]){
    //Creazione struct identificatore cliente
    client_id *id=malloc(sizeof(client_id));
    if (!check_args(argc, argv, id)){
        fprintf(stderr, "Errore argomenti\n\n\t-i: IMMETTI IL TUO ID (8 caratteri)\n\n\t-p: IMMETTI LA TUA PORTA UDP (4 caratteri, inferiore a 9999)\n");
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
    

    //Struct + socket per inviare messaggi sulla porta TCP
    struct sockaddr_in address_sock_tcp;
    address_sock_tcp.sin_family=AF_INET;
    address_sock_tcp.sin_port=htons(6769);
    //TODO prendere l'ip del server
    inet_aton("127.0.0.1", &address_sock_tcp.sin_addr);
    id->fdTCP=socket(PF_INET, SOCK_STREAM, 0);

    puts("Connessione al server");
    sleep(1.5);
    if (connect(id->fdTCP, (struct sockaddr *)&address_sock_tcp, sizeof(address_sock_tcp))==EOF){
        perror("Errore di connessione");
        free(id);
        return EXIT_FAILURE;
    }
    else{
        puts("Connessione stabilita");
        sleep(1);
        system("clear");
    }

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
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "ERRORE! Numero digitato fuori dal range consentito.\n");
            return EXIT_FAILURE;
        }
    
    id->fdUDP=socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address_sock_udp;
    address_sock_udp.sin_family=AF_INET;
    address_sock_udp.sin_port=htons(atoi(id->PORT));
    address_sock_udp.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(id->fdUDP,(struct sockaddr *)&address_sock_udp,sizeof(struct sockaddr_in)) == EOF){
        perror("Errore binding udp");
        free(id);
        return EXIT_FAILURE;
    }

    pthread_t th1;
    pthread_create(&th1,NULL,udp_listen,id);
 
    sleep(2);
    system("clear");
    int choice;
    while (1){
        if (id->notifications>0){
            printf("Hai %d notifiche\n", id->notifications);
            //!Mutex?
            id->notifications=0;
        }
        sleep(2);
        system("clear");
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