#include "client.h"
#include <stdio.h>
#include <stdlib.h>

bool check_args(int argc, char* argv[], client_id* id){
    if (argc != 5)  return false;
    if (strcmp(argv[1],"-i") == 0){
        if (strlen(argv[2]) != LENGTH_ID)
            return false;
        strcpy(id->ID, argv[2]); //Identificativo client
        if (strcmp(argv[3],"-p") != 0 || strlen(argv[4]) != LENGTH_UDP_PORT)
            return false;
        strcpy(id->PORT, argv[4]); //Porta UDP client
        return true;
    }
    else if (strcmp(argv[1],"-p")==0){
        if (strlen(argv[2]) != LENGTH_UDP_PORT)
            return false;
        strcpy(id->PORT, argv[3]); //Porta UDP client
        if (strcmp(argv[3],"-i") != 0 || strlen(argv[4]) != LENGTH_ID)
            return false;
        strcpy(id->PORT, argv[4]); //Porta UDP client
        return true;
    }
    else
        return false;
}

bool check_MPD(const int *p){
    return (*p>=0 && *p<=65535);
}

int tcpSock(client_id* id){
    struct sockaddr_in address_sock_tcp;
    address_sock_tcp.sin_family=AF_INET;
    address_sock_tcp.sin_port=htons(6769);
    //TODO prendere l'ip del server
    inet_aton("127.0.0.1", &address_sock_tcp.sin_addr);
    id->fdTCP=socket(PF_INET, SOCK_STREAM, 0);
    if (id->fdTCP == EOF){
        perror("Errore socket");
        free(id);
        return -1;
    }

    puts("Connessione al server");
    sleep(1.5);
    if (connect(id->fdTCP, (struct sockaddr *)&address_sock_tcp, sizeof(address_sock_tcp))==EOF){
        perror("Errore di connessione");
        free(id);
        return -1;
    }
    else{
        puts("Connessione stabilita");
        sleep(1);
        system("clear");
    }
    return 0;
}

int udpSock(client_id* id){
    id->fdUDP=socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address_sock_udp;
    address_sock_udp.sin_family=AF_INET;
    address_sock_udp.sin_port=htons(atoi(id->PORT));
    address_sock_udp.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(id->fdUDP,(struct sockaddr *)&address_sock_udp,sizeof(struct sockaddr_in)) == EOF){
        perror("Errore binding udp");
        free(id);
        return -1;
    }
    return 0;
}


int print_intro(){
    puts("Benvenuto su IPortbook!\nSe è la tua prima volta, premi 1 e registrati, altrimenti digita 2 per conneterti o 3 per uscire\n");
    int n;
    scanf("%d", &n);
    return n;
}

int print_menu(){
    puts("Scegli una tra le seguenti opzioni e digita il numero a essa associato\n1. Richiedi amicizia\n2. Manda messaggio\n");
    int n;
    scanf("%d", &n);
    return n;
}

//Funzione ausiliaria per mandare messaggio tcp al server
void sendTcp(const client_id* id, char* mess, char type[]){
    if (strcmp(type, REGIS_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_REGIS);
    }
    else if (strcmp(type, CONNE_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_CONNE);
    }
    else if (strcmp(type, FRIE_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_FRIE);
    }
    char buf[10];
    int r=read(id->fdTCP, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);
}

//Funzione ausiliaria per costurire messaggio REGIS
char* build_message_reg(const client_id *id){
    char *mess=malloc(LENGTH_HEADER+LENGTH_ID+1+LENGTH_UDP_PORT+1+2+LENGTH_END_SYMBOL);
    printf("DEBUG: MDP in little-endian = 0x%02x 0x%02x\n", id->MDP & 255, (id->MDP >> 8) & 255);
    sprintf(mess, "%s%s %s ", REGIS_HEADER, id->ID, id->PORT);
    int i=strlen(mess);
    mess[i]=(id->MDP >> 8) & 255;
    mess[i++]=id->MDP & 255;
    memcpy(mess+i+1, END_SYMBOL, 3);

    return mess;
}

void registration(const client_id *id){
    char *mess=build_message_reg(id);
    sendTcp(id, mess, REGIS_HEADER);
}

//Funzione ausiliaria per costurire messaggio CONNE
char *build_message_conn(const client_id *id){
    char *mess=malloc(LENGTH_HEADER+LENGTH_ID+1+2+LENGTH_END_SYMBOL);
    sprintf(mess, "%s%s ", CONNE_HEADER, id->ID);
    int i=strlen(mess);
    mess[i]=(id->MDP >> 8) & 255;
    mess[i++]=id->MDP & 255;
    memcpy(mess+i+1, END_SYMBOL, 3);
    return mess;
}

void connection(const client_id *id){
    char *mess=build_message_conn(id);
    sendTcp(id, mess, CONNE_HEADER);
}

//Funzione ausiliaria per costurire messaggio FRIE?
char *build_message_frie(){
    char *mess=malloc(LENGTH_FRIE);
    char buf[100];
    printf("Inserisci l'ID dell'utente con cui desideri stringere amicizia: ");
    scanf("%s", buf);
    if (strlen(buf) != LENGTH_ID){
        free(mess);
        return NULL;
    }
    char frie_id[LENGTH_ID+1];
    strncpy(frie_id, buf, LENGTH_ID);
    sprintf(mess, "%s%s", FRIE_HEADER, frie_id);
    memcpy(mess+LENGTH_HEADER+LENGTH_ID, END_SYMBOL, 3);
    return mess;
}

void friend(const client_id* id){
    char *mess=build_message_frie();
    if (mess == NULL){
        fprintf(stderr, "Errore! ID inserito troppo corto/lungo");
        return;
    }
    sendTcp(id, mess, FRIE_HEADER);
}

void* udp_listen(void* ptr){
    client_id *id = (client_id *)ptr;
    char buf[4];
    while(1){
        int rec=recv(id->fdUDP,buf,3,0);
        buf[rec]='\0';
        //!Mutex?
        pthread_mutex_lock(&id->mtx);
        id->notifications++;
        pthread_mutex_unlock(&id->mtx);
    }
}
