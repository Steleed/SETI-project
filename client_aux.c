#include "client.h"

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
        close(id->fdTCP);
        free(id);
        return -1;
    }
    return 0;
}


int print_intro(){
    puts("Benvenuto su IPortbook!\nSe è la tua prima volta, premi 1 e registrati, altrimenti digita 2 per conneterti o 3 per uscire\n");
    int n;
    char* tmp=malloc(100*sizeof(char));
    //scanf("%d", &n);
    fgets(tmp, 100, stdin);
    n=atoi(tmp);
    free(tmp);
    return n;
}

int print_menu(){
    puts("Scegli una tra le seguenti opzioni e digita il numero a essa associato\n1. Richiedi amicizia\n2. Manda messaggio\n3. Manda flood\n4. Visualizza elenco utenti\n5. Consulta le notifiche\n6. Disconnettiti\n");
    int n;
    //scanf("%d", &n);
    char* tmp=malloc(100*sizeof(char));
    fgets(tmp, 100, stdin);
    n=atoi(tmp);
    free(tmp);
    system("clear");
    return n;
}

//Funzione ausiliaria per mandare messaggio tcp al server
void sendTcp(client_id* id, char* mess, char type[]){
    if (strcmp(type, REGIS_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_REGIS);
    }
    else if (strcmp(type, CONNE_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_CONNE);
    }
    else if (strcmp(type, FRIE_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_FRIE);
    }
    else if (strcmp(type, MESS_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_MESS+id->messLength);
    }
    else if (strcmp(type, FLOO_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_FLOO+id->messLength);
    }
    else if (strcmp(type, LIST_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_LIST);
        read_list(id->fdTCP);
        return;
    }
    else if (strcmp(type, IQUIT_HEADER) ==0 ){
        write(id->fdTCP, mess, LENGTH_IQUIT);
    }
    else{
        //TODO CONSU
    }
    char buf[LENGTH_HEADER+LENGTH_END_SYMBOL+1];
    int r=read(id->fdTCP, buf, LENGTH_HEADER+LENGTH_END_SYMBOL);
    buf[r]='\0';
    printf("%s\n", buf);
    if (strcmp(buf, FORMAT_GOBYE) == 0){ 
        close(id->fdUDP);
        free(id);
        free(mess);
        exit(EXIT_SUCCESS);
    }
}

//Funzione ausiliaria per costurire messaggio REGIS
char* build_message_reg(client_id *id){
    /*char *mess=malloc(LENGTH_REGIS);
    int i=sprintf(mess, "%s %s %s ", REGIS_HEADER, id->ID, id->PORT);
    mess[i]=id->MDP & 255;
    i++;
    mess[i]=(id->MDP >> 8) & 255;
    printf("DEBUG: MDP in little-endian = 0x%02x 0x%02x\n", mess[i-1], mess[i]);
    memcpy(mess+i+1, END_SYMBOL, LENGTH_END_SYMBOL);*/
    char *mess=malloc(LENGTH_REGIS+1);
    sprintf(mess, FORMAT_REGIS, id->ID, id->PORT, id->MDP & 255, (id->MDP >> 8) & 255);

    return mess;
}

void registration(client_id *id){
    char *mess=build_message_reg(id);
    sendTcp(id, mess, REGIS_HEADER);
    free(mess);
}

//Funzione ausiliaria per costurire messaggio CONNE
char *build_message_conn(const client_id *id){
    /*char *mess=malloc(LENGTH_CONNE);
    int i=sprintf(mess, "%s %s ", CONNE_HEADER, id->ID);
    mess[i]=id->MDP & 255;
    i++;
    mess[i]=(id->MDP >> 8) & 255;
    memcpy(mess+i+1, END_SYMBOL, LENGTH_END_SYMBOL);*/
    char *mess=malloc(LENGTH_REGIS+1);
    sprintf(mess, FORMAT_CONNE, id->ID, id->MDP & 255, (id->MDP >> 8) & 255);

    return mess;
}

void connection(client_id *id){
    char *mess=build_message_conn(id);
    sendTcp(id, mess, CONNE_HEADER);
    free(mess);
}

//Funzione ausiliaria per costurire messaggio FRIE?
char *build_message_frie(){
    char *mess=malloc(LENGTH_FRIE+1);
    char *buf=malloc(100*sizeof(char));
    printf("Inserisci l'ID dell'utente con cui desideri stringere amicizia: ");
    //scanf("%s", buf);
    fgets(buf, 100, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) != LENGTH_ID){
        free(mess);
        free(buf);
        return NULL;
    }
    /*char frie_id[LENGTH_ID+1];
    strncpy(frie_id, buf, LENGTH_ID);
    sprintf(mess, "%s%s", FRIE_HEADER, frie_id);
    memcpy(mess+LENGTH_HEADER+LENGTH_ID, END_SYMBOL, LENGTH_END_SYMBOL);*/
    sprintf(mess, FORMAT_FRIE, buf);
    free(buf);
    return mess;
}

void friend(client_id* id){
    char *mess=build_message_frie();
    if (mess == NULL){
        fprintf(stderr, "Errore! ID inserito troppo corto/lungo");
        return;
    }
    sendTcp(id, mess, FRIE_HEADER);
    free(mess);
}

//Funzione ausiliaria per leggere il messaggio scritto in input
void read_mess(char* str){
    system("clear");
    printf("Inserisci il messaggio da inviare: ");
    fgets(str, 500, stdin);
    if (strchr(str, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    str[strcspn(str, "\n")] = '\0';
}

//Funzione ausiliaria per costruire messaggio MESS?
char* build_message_mess(const char* str, int length){
    char* mess=malloc(LENGTH_MESS+length+1);
    char *buf=malloc(100*sizeof(char));
    printf("Inserisci l'ID dell'utente a cui vuoi mandare il messaggio: ");
    //scanf("%s", buf);
    fgets(buf, 100, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    if (strlen(buf) != LENGTH_ID){
        free(mess);
        free(buf);
        return NULL;
    }
    sprintf(mess, FORMAT_MESS, buf, str);
    free(buf);
    return mess;
}

void mess(client_id* id){
    char str[500];
    read_mess(str);
    id->messLength=strlen(str);
    char* mess=build_message_mess(str, id->messLength);
    if (mess == NULL){
        fprintf(stderr, "Errore! ID inserito troppo corto/lungo");
        return;
    }
    sendTcp(id, mess, MESS_HEADER);
    free(mess);
}

//Funzione ausiliaria per costruire ,essaggio FLOO?
char* build_message_floo(const char* str, int length){
    char* mess=malloc(LENGTH_FLOO+length+1);
    /*sprintf(mess, "%s%s", FLOO_HEADER, str);
    memcpy(mess+LENGTH_HEADER+length, END_SYMBOL, 3);*/
    sprintf(mess, FORMAT_FLOO, str);
    return mess;
}

void floo(client_id* id){
    char str[200];
    read_mess(str);
    id->messLength=strlen(str);
    char* mess=build_message_floo(str, id->messLength);
    sendTcp(id, mess, FLOO_HEADER);
    free(mess);
}

void read_list(int fd){
    char buf[LENGTH_RLIST+1];
        int r=read(fd, buf, LENGTH_RLIST);
        buf[r]='\0';
        int num_users;
        printf("%s\n", buf);
        sscanf(buf, FORMAT_RLIST, &num_users);
        printf("DEBUG: numero utenti = %d\n", num_users);
        char usr[LENGTH_LINUM+1];
        for (int i=0;i<num_users;i++){
            r=read(fd, usr, LENGTH_LINUM);
            usr[r]='\0';
            printf("%s\n", usr);
            //sleep(1);
        }
}

//Funzione ausiliaria per costruire messaggio LIST?
char* build_message_list(){
    char* mess=malloc(LENGTH_LIST+1);
    strcpy(mess, FORMAT_LIST);
    return mess;
}

void list(client_id* id){
    char* mess=build_message_list();
    sendTcp(id, mess, LIST_HEADER);
    free(mess);
}

char* build_message_iquit(){
    char* mess=malloc(LENGTH_IQUIT+1);
    strcpy(mess, FORMAT_IQUIT);
    return mess;
}

void iquit(client_id* id){
    char* mess=build_message_iquit();
    sendTcp(id, mess, IQUIT_HEADER);
}

void* udp_listen(void* ptr){
    client_id *id = (client_id *)ptr;
    char buf[LENGTH_UDP_NOT+1];
    struct sockaddr_in server;
    socklen_t a=sizeof(server);
    while(1){
        int rec=recvfrom(id->fdUDP,buf,LENGTH_UDP_NOT,0,(struct sockaddr *)&server,&a);
        buf[rec]='\0';
        //!Mutex?
        pthread_mutex_lock(&id->mtx);
        id->notifications++;
        pthread_mutex_unlock(&id->mtx);
    }
}
