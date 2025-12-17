#include "project.h"
#include <stdio.h>
#include <stdlib.h>

bool check_MPD(const int *p){
    return (p>=0 && p<=65535);
}

int print_intro(){
    puts("Benvenuto nel menu!\nScegli un'opzione tra le seguenti e digita il numero a essa associato.\nSe è la tua prima volta, premi 1 e registrati, altrimenti digita 2 per conneterti\n");
    int n;
    scanf("%d", &n);
    return n;
}

int print_menu(){
    puts("\n1. Richiedi amicizia\n2. Manda messaggio");
    int n;
    scanf("%d", &n);
    return n;
}

void registration(int fd, const struct client_id *id){
    char *mess=malloc(LENGTH_HEADER+LENGTH_ID+1+LENGTH_UDP_PORT+1+2/*+mdp*/+LENGTH_END_SYMBOL+1); //+1 finale per '\0'
    char byte_0=id->mdp & 11111111/*0xFF*/;
    char byte_1=(id->mdp >> 8) & 11111111/*0xFF*/;
    //TODO riempire mess e capire come usare mpd
    write(fd, mess, strlen(mess));
    char buf[10];
    int r=read(fd, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);
}

void connection(int fd, const struct client_id *id){
    //TODO mandare il messaggio di connesione
    char buf[10];
    int r=read(fd, buf, 9);
    buf[r]='\0';
    printf("%s\n", buf);
}