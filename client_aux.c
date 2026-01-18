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

int print_intro(){
    puts("Benvenuto su IPortbook!\nSe è la tua prima volta, premi 1 e registrati, altrimenti digita 2 per conneterti o 3 per uscire\n");
    int n;
    scanf("%d", &n);
    return n;
}

int print_menu(){
    puts("Scegli una tra le seguenti opzioni e digita il numero a essa associato\n1. Richiedi amicizia\n2. Manda messaggio");
    int n;
    scanf("%d", &n);
    return n;
}

char* build_message_reg(const client_id *id){
    char *mess=malloc(LENGTH_HEADER+LENGTH_ID+1+LENGTH_UDP_PORT+1+2+LENGTH_END_SYMBOL);
    printf("MDP in little-endian: %02X %02X\n", id->MDP & 0xFF, (id->MDP >> 8) & 0xFF);
    sprintf(mess, "%s%s %s ", REGIS_HEADER, id->ID, id->PORT);
    int i=strlen(mess);
    mess[i]=(id->MDP >> 8) & 255;
    mess[i++]=id->MDP & 255;
    memcpy(mess+i+1, END_SYMBOL, 3);
    
    return mess;
}

void registration(const client_id *id){
    char *mess=build_message_reg(id);
    write(id->fdTCP, mess, LENGTH_REGIS);
    char buf[10];
    int r=read(id->fdTCP, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);
}

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
    write(id->fdTCP, mess, LENGTH_CONNE);
    char buf[10];
    int r=read(id->fdTCP, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);
}

void* udp_listen(void* ptr){
    client_id *id = (client_id *)ptr;
    char buf[4];
    while(1){
        int rec=recv(id->fdUDP,buf,3,0);
        buf[rec]='\0';
        //!Mutex?
        id->notifications++;
    }
}
