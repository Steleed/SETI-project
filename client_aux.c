#include "project.h"
#include <stdio.h>
#include <stdlib.h>

bool check_args(int argc, char* argv[]){
    return (argc == 3 && strlen(argv[1])==LENGTH_ID && strlen(argv[2])==LENGTH_UDP_PORT);
}

bool check_MPD(const int p){
    return (p>=0 && p<=65535);
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
    char *mess=malloc(LENGTH_HEADER+LENGTH_ID+1+LENGTH_UDP_PORT+1+2+LENGTH_END_SYMBOL); //+1 finale per '\0'
    uint16_t littleEndian=htole16(id->MDP);
    printf("MDP in little-endian: %02X %02X\n", littleEndian & 0xFF, (littleEndian >> 8) & 0xFF);
    sprintf(mess, "%s%s %s ", REGIS_HEADER, id->ID, id->PORT);
    int i=strlen(mess);
    mess[i]=(littleEndian >> 8) & 255;
    mess[i++]=littleEndian & 255;
    memcpy(mess+i+1, END_SYMBOL, 3);
    //TODO mettere la password in little-endian
    
    return mess;
}

void registration(int fd, const client_id *id){
    char *mess=build_message_reg(id);
    write(fd, mess, MAX_REGIS_HEADER);
    char buf[10];
    int r=read(fd, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);
}

char *build_message_conn(const client_id *id){
    char *mess=malloc(LENGTH_HEADER+LENGTH_ID+1+2/*+mdp*/+LENGTH_END_SYMBOL+1); //+1 finale per '\0'
    int i;
    for (i=0; i<LENGTH_HEADER; i++){
        mess[i]=CONNE_HEADER[i];
    }
    for (int c=0; c<LENGTH_ID; c++){
        mess[i]=id->ID[c];
        i++;
    }
    mess[i]=' ';
    i++;
    mess[i]=id->MDP & 255 /*1111 1111*/;
    i++;
    mess[i]=(id->MDP >> 8) & 255 /*1111 1111*/;
    i++;
    for (int c=0; c<LENGTH_END_SYMBOL; c++){
        mess[i]=END_SYMBOL[c];
        i++;
    }
    mess[i]='\0';
    return mess;
}

void connection(int fd, const client_id *id){
    //TODO mandare il messaggio di connesione
    char *mess=build_message_conn(id);
    /*char buf[10];
    int r=read(fd, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);*/
}