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
    else{//Consu
        write(id->fdTCP, mess, LENGTH_CONSU);
        read_consu(id);
        return;
    }
    char buf[LENGTH_HEADER+LENGTH_END_SYMBOL+1];
    int r=read(id->fdTCP, buf, LENGTH_HEADER+LENGTH_END_SYMBOL);
    buf[r]='\0';
    printf("[LOG]%s\n", buf);
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
    printf("Registrazione avvenuta con successo!\n");
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
    printf("Sei connesso!\n");
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
    printf("[LOG}%s\n", buf);
    sscanf(buf, FORMAT_RLIST, &num_users);
    printf("DEBUG: numero utenti = %d\n", num_users);
    printf("Lettura lista di %d utenti:\n", num_users);
    char usr[LENGTH_LINUM+1];
    char id[LENGTH_ID+1];
    for (int i=0;i<num_users;i++){
        r=read(fd, usr, LENGTH_LINUM);
        usr[r]='\0';
        printf("[LOG]%s\n", usr);
        sscanf(usr, FORMAT_LINUM, id);
        printf("%s\n", id);
        sleep(1);
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

void friend_request(int fd){
    char* tmp=malloc(100*sizeof(char));
    int p;
    do{
        fgets(tmp, 100, stdin);
        p=atoi(tmp);
    }
    while (p != 1 && p != 2);
    free(tmp);
    if (p == 1){
        write(fd, FORMAT_OKIRF, LENGTH_HEADER+LENGTH_END_SYMBOL);
    }
    else{
        write(fd, FORMAT_NOKRF, LENGTH_HEADER+LENGTH_END_SYMBOL);
    }
    char ack[LENGTH_HEADER+LENGTH_END_SYMBOL+1];
    int r=read(fd, ack, LENGTH_HEADER+LENGTH_END_SYMBOL);
    ack[r]='\0';
    printf("[LOG]%s\n", ack);
}


char* build_message_consu(){
    char* mess=malloc(LENGTH_CONSU+1);
    strcpy(mess, FORMAT_CONSU);
    return mess;
}

void consu(client_id* id){
    char* mess=build_message_consu();
    sendTcp(id, mess, CONSU_HEADER);
}

void read_consu(client_id *id){
    char buf[503];
    int r=read(id->fdTCP, buf, 503);
    buf[r]='\0';
    printf("[LOG]%s\n", buf);
    pthread_mutex_lock(&id->mtx);
    if (id->notifications == 0){
        printf("Non ci sono flussi da consultare\n");
        pthread_mutex_unlock(&id->mtx);
        return;
    }
    switch (id->notifications->udp_notification_type)
    {
    case 0: {
        //TODO flux frie
        char ID[LENGTH_ID+1];
        sscanf(buf, FORMAT_FLUX_FRIE, ID);
        printf("Richiesta d'amicizia da parte dell'utente %s.\n1: Accetta\n2: Rifiuta", ID);
        friend_request(id->fdTCP);
        break;
    }

    case 1: {
        char id[LENGTH_ID+1];
        sscanf(buf, FORMAT_FRIEN, id);
        printf("L'utente %s ha accettato la tua richiesta d'amicizia\n", id);
        break;
    }

    case 2: {
        char id[LENGTH_ID+1];
        sscanf(buf, FORMAT_NOFRI, id);
        printf("L'utente %s ha rifiutato la tua richiesta d'amicizia\n", id);        
        break;
    }
    
    case 3: {
        char mess[MAX_MESS+1];
        char id[LENGTH_ID+1];
        sscanf(buf, FORMAT_FLUX_MESS, id, mess);
        printf("Messaggio da parte di %s:\n%s", id, mess);
        break;
    }
    
    case 4: {
        char floo[MAX_MESS+1];
        char id[LENGTH_ID+1];
        sscanf(buf, FORMAT_FLUX_MESS, id, floo);
        printf("Messaggio di flooding da parte di %s:\n%s", id, floo);
        break;
    }
    
    default: 
        printf("Comportamento indefinito\n");
        break;
    }
    //Cancello notifica consultata
    Notifications* aux=id->notifications;
    id->notifications=id->notifications->next;
    id->num_notifications--;
    free(aux);
    pthread_mutex_unlock(&id->mtx);
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

//Funzione ausiliaria per inserire la notifica nella struttura dati
void insert_notification(client_id* id, char *buf){
    pthread_mutex_lock(&id->mtx);
    int type = buf[0]-'0'; //Tipo notifica udp
    u_int16_t n; //Numero notifiche
    char b1=buf[1];
    char b2=buf[2];
    n = (b2 << 8) | b1;
    id->num_notifications=(int)n;
    Notifications* new=malloc(sizeof(Notifications));
    new->udp_notification_type=type;
    new->next=NULL;
    if (id->notifications == NULL){
        id->notifications=new;
    }
    else{
        Notifications *aux=id->notifications;
        while (aux->next != NULL){
            aux=aux->next;
        }
        aux->next=new;
    }
    pthread_mutex_unlock(&id->mtx);
}

void* udp_listen(void* ptr){
    client_id *id = (client_id *)ptr;
    char buf[LENGTH_UDP_NOT+1];
    struct sockaddr_in server;
    socklen_t a=sizeof(server);
    while(1){
        int rec=recvfrom(id->fdUDP,buf,LENGTH_UDP_NOT,0,(struct sockaddr *)&server,&a);
        buf[rec]='\0';
        insert_notification(id, buf);
    }
}
