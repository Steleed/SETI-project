#include <stdlib.h>
#include <stdio.h>
#include "project.h"

int main(int argc, char* argv[]){
    //Creazione struct identificatore cliente
    client_id *id=malloc(sizeof(client_id));
    if (!check_args(argc, argv)){
        fprintf(stderr, "Errore argomenti\n\n\t-i: IMMETTI IL TUO ID\n\n\t-p: IMMETTI LA TUA PORTA UDP\n");
        return EXIT_FAILURE;
    }
    //TODO check -i -p
    strcpy(id->ID, argv[1]); //Identificativo client
    strcpy(id->PORT, argv[2]); //Porta UDP client
    int p;
    do {
        printf("Inserisci la password ((un numero compreso tra 0 e 65535): ");
        scanf("%d", &p);
    }
    while (!check_MPD(p));
    id->MDP=p; //Password client
    system("clear");
    

    //Struct + socket per inviare messaggi sulla porta TCP
    struct sockaddr_in address_sock_tcp;
    address_sock_tcp.sin_family=AF_INET;
    address_sock_tcp.sin_port=htons(6769);
    //TODO prendere l'ip del server
    inet_aton("127.0.0.1", &address_sock_tcp.sin_addr);
    int sock1=socket(PF_INET, SOCK_STREAM, 0);

    puts("Connessione al server");
    sleep(1.5);
    if (connect(sock1, (struct sockaddr *)&address_sock_tcp, sizeof(address_sock_tcp))==EOF){
        perror("Errore di connessione");
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
            registration(sock1, id);
            break;
        case 2:
            //TODO registrazione
            connection(sock1, id);
            break;
        case 3:
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "ERRORE! Numero digitato fuori dal range consentito.\n");
            return EXIT_FAILURE;
        }
    
    /*sleep(2);
    system("clear");*/
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