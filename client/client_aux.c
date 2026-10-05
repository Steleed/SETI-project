#include "client.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

bool check_args(int argc, char* argv[], client_id* id){ //verifica degli argomenti
    bool has_id = false;
    bool has_port = false;
    bool has_server_ip = false;
    bool has_clear_option = false;
    for (int i = 1; i < argc;){
        if ((strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--clear") == 0) && !has_clear_option){
            clear_screen_enabled = true;
            has_clear_option = true;
            i++;
            continue;
        }
        if (i + 1 >= argc)
            return false;

        if (strcmp(argv[i], "-i") == 0 && !has_id){
            if (strlen(argv[i + 1]) != LENGTH_ID)
                return false;
            strcpy(id->ID, argv[i + 1]);
            has_id = true;
            i += 2;
        }
        else if (strcmp(argv[i], "-p") == 0 && !has_port){
            if (strlen(argv[i + 1]) != LENGTH_UDP_PORT)
                return false;
            strcpy(id->PORT, argv[i + 1]);
            has_port = true;
            i += 2;
        }
        else if (strcmp(argv[i], "-s") == 0 && !has_server_ip){
            struct in_addr server_address;
            if (inet_pton(AF_INET, argv[i + 1], &server_address) != 1)
                return false;
            strcpy(ip_server, argv[i + 1]);
            has_server_ip = true;
            i += 2;
        }
        else{
            return false;
        }
    }

    return has_id && has_port;
}

void clear_screen(void){
    if (clear_screen_enabled)
        system("clear");
}

bool check_MPD(const int *p){
    return (*p>=0 && *p<=65535); //controllo range password
}

int tcpSock(client_id* id){ //inizializzazione socket tcp e connessione al server
    const int connect_timeout_ms = 3000;
    struct sockaddr_in address_sock_tcp;
    address_sock_tcp.sin_family=AF_INET;
    address_sock_tcp.sin_port=htons(6769);

    if (inet_pton(AF_INET, ip_server, &address_sock_tcp.sin_addr) != 1){
        fprintf(stderr, "Indirizzo IPv4 del server non valido: %s\n", ip_server);
        free(id);
        return -1;
    }

    id->fdTCP=socket(PF_INET, SOCK_STREAM, 0);
    if (id->fdTCP < 0){
        perror("Errore socket");
        free(id);
        return -1;
    }

    puts("Connessione al server");
    sleep(1.5);
    if (connect(id->fdTCP, (struct sockaddr *)&address_sock_tcp, sizeof(address_sock_tcp))==EOF){
        perror("Errore di connessione");
        close(id->fdTCP);
        free(id);
        return -1;
    }
    else{
        puts("Connessione stabilita");
        sleep(1);
        clear_screen();
    }
    inet_ntop(AF_INET, &address_sock_tcp.sin_addr, ip_server, sizeof(ip_server));
    return 0;
}

int udpSock(client_id* id){ //inizializzazione socket udp per notifiche
    id->fdUDP=socket(PF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in address_sock_udp;
    address_sock_udp.sin_family=AF_INET;
    address_sock_udp.sin_port=htons(atoi(id->PORT));
    address_sock_udp.sin_addr.s_addr=htonl(INADDR_ANY);

    if(bind(id->fdUDP,(struct sockaddr *)&address_sock_udp,sizeof(struct sockaddr_in)) == EOF){
        perror("Errore binding udp");
        close(id->fdTCP);
        close(id->fdUDP);
        free(id);
        return -1;
    }
    return 0;
}


int print_intro(){
    puts("Benvenuto su IPortbook!\nSe è la tua prima volta, premi 1 e registrati, altrimenti digita 2 per conneterti o 3 per uscire\n");
    int n;
    char* tmp=malloc(100*sizeof(char));
    printf("> ");
    fflush(stdout);
    fgets(tmp, 100, stdin);
    n=atoi(tmp);
    free(tmp);
    return n;
}

int print_menu(bool *first_round){
    puts("Scegli una tra le seguenti opzioni e digita il numero a essa associato\n1. Richiedi amicizia\n2. Manda messaggio\n3. Manda flood\n4. Visualizza elenco utenti\n5. Consulta le notifiche\n6. Disconnettiti\n");
    if (*first_round){
        puts("\nMentre eri offline avresti potuto ricevere nuove notifiche! Premi 5 per consultare.\n\n");
        *first_round = false;
    }
    int n;
    char* tmp=malloc(100*sizeof(char));
    printf("> ");
    fflush(stdout);
    fgets(tmp, 100, stdin);
    n=atoi(tmp);
    free(tmp);
    clear_screen();
    return n;
}

void read_all(client_id *id, char *buf, int length){
    int total = 0;
    int r;
    while (total < length) {
        char c;
        r = read(id->fdTCP, &c, 1);
        if (r <= 0){
            fprintf(id->log, "[LOG]: Errore in lettura\n");
            close(id->fdTCP);
            close(id->fdUDP);
            fclose(id->log);
            free(id);
            exit(EXIT_FAILURE);
        }
        buf[total++] = c;
        buf[total] = '\0';
    }
}

//Funzione ausiliaria per mandare messaggio tcp al server
void sendTcp(client_id* id, char* mess){
    if (strncmp(mess, REGIS_HEADER, LENGTH_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_REGIS);
    }
    else if (strncmp(mess, CONNE_HEADER, LENGTH_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_CONNE);
    }
    else if (strncmp(mess, FRIE_HEADER, LENGTH_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_FRIE);
    }
    else if (strncmp(mess, MESS_HEADER, LENGTH_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_MESS+id->messLength);
    }
    else if (strncmp(mess, FLOO_HEADER, LENGTH_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_FLOO+id->messLength);
    }
    else if (strncmp(mess, LIST_HEADER, LENGTH_HEADER) == 0){
        write(id->fdTCP, mess, LENGTH_LIST);
        read_list(id, id->fdTCP);
        return;
    }
    else if (strncmp(mess, IQUIT_HEADER, LENGTH_HEADER) ==0 ){
        write(id->fdTCP, mess, LENGTH_IQUIT);
    }
    else{//Consu
        write(id->fdTCP, mess, LENGTH_CONSU);
        read_consu(id);
        return;
    }

    char buf[LENGTH_HEADER+LENGTH_END_SYMBOL+1];
    //int r=read(id->fdTCP, buf, LENGTH_HEADER+LENGTH_END_SYMBOL); //lettura risposta server
    //buf[r]='\0';
    read_all(id, buf, LENGTH_HEADER + LENGTH_END_SYMBOL);
    //printf("[LOG]%s\n", buf);
    fprintf(id->log, "[LOG]: %s\n", buf); //Scrittura log su file

    if (strcmp(buf, FORMAT_GOBYE) == 0){ 
        close(id->fdUDP);
        close(id->fdTCP);
        fclose(id->log);
        free(id);
        free(mess);
        puts("GOODBYE");
        exit(EXIT_SUCCESS);
    }//feedback
    if (strcmp(buf, FORMAT_OKFRIE) == 0){
        printf("Richiesta d'amicizia inviata con successo\n");
        return;
    }
    if (strcmp(buf, FORMAT_NOFRIE) == 0){
        fprintf(stderr, "Richiesta d'amicizia non inviata, l'utente potrebbe non esistere o essere gia tuo amico\n");
        return;
    }
    if (strcmp(buf, FORMAT_OKMESS) == 0){
        printf("Messaggio inviato con successo\n");
        return;
    }
    if (strcmp(buf, FORMAT_NOMESS) == 0){
        fprintf(stderr, "Messaggio non inviato, il destinatario potrebbe non esistere o non essere tuo amico\n");
        return;
    }
}

//Funzione ausiliaria per costurire messaggio REGIS
char* build_message_reg(client_id *id){
    char *mess=malloc(LENGTH_REGIS+1);
    sprintf(mess, FORMAT_REGIS, id->ID, id->PORT, id->MDP & 255, (id->MDP >> 8) & 255);
    //i 2 byte della password inviati separatamente
    return mess;
}

void registration(client_id *id){
    char *mess=build_message_reg(id);
    sendTcp(id, mess);
    printf("Registrazione avvenuta con successo!\n");
    free(mess);
}

//Funzione ausiliaria per costurire messaggio CONNE
char *build_message_conn(const client_id *id){
    char *mess=malloc(LENGTH_REGIS+1);
    sprintf(mess, FORMAT_CONNE, id->ID, id->MDP & 255, (id->MDP >> 8) & 255);

    return mess;
}

void connection(client_id *id){
    char *mess=build_message_conn(id);
    sendTcp(id, mess);
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
    sendTcp(id, mess);
    free(mess);
    printf("Premi INVIO per continuare...");
    getchar();
}

//Funzione ausiliaria per leggere il messaggio scritto in input
void read_mess(char* str, size_t capacity){
    clear_screen();
    printf("Inserisci il messaggio da inviare: ");
    if (fgets(str, capacity, stdin) == NULL){
        str[0] = '\0';
        return;
    }
    if (strchr(str, '\n') == NULL) { //Scarta la parte eccedente della riga
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
    read_mess(str, sizeof(str));
    id->messLength=strlen(str);
    char* mess=build_message_mess(str, id->messLength);
    if (mess == NULL){
        fprintf(stderr, "Errore! ID inserito troppo corto/lungo");
        return;
    }
    sendTcp(id, mess);
    free(mess);
    printf("Premi INVIO per continuare...");
    getchar();
}

//Funzione ausiliaria per costruire ,essaggio FLOO?
char* build_message_floo(const char* str, int length){
    char* mess=malloc(LENGTH_FLOO+length+1);
    sprintf(mess, FORMAT_FLOO, str);
    return mess;
}

void floo(client_id* id){
    char str[MAX_MESS+1];
    read_mess(str, sizeof(str));
    id->messLength=strlen(str);
    char* mess=build_message_floo(str, id->messLength);
    sendTcp(id, mess);
    free(mess);
    printf("Messaggio di flooding inviato\n");
    printf("Premi INVIO per continuare...");
    getchar();
}

void read_list(client_id *id, int fd){
    char buf[LENGTH_RLIST+1];
    read_all(id, buf, LENGTH_RLIST);
    int num_users;
    //printf("[LOG]%s\n", buf);
    fprintf(id->log, "[LOG]: %s\n", buf);
    sscanf(buf, FORMAT_RLIST, &num_users); //lettura numero utenti
    printf("Lettura lista di %d utenti:\n", num_users);
    char usr[LENGTH_LINUM+1];
    char id_usr[LENGTH_ID+1];
    for (int i=0;i<num_users;i++){
        read_all(id, usr, LENGTH_LINUM); //lettura utenti
        //printf("[LOG]%s\n", usr);
        fprintf(id->log, "[LOG]: %s\n", usr);
        sscanf(usr, FORMAT_LINUM, id_usr);
        printf("%s\n", id_usr);
        
    }
    printf("\nPremi INVIO per continuare...");
    getchar();
}

//Funzione ausiliaria per costruire messaggio LIST?
char* build_message_list(){
    char* mess=malloc(LENGTH_LIST+1);
    strcpy(mess, FORMAT_LIST);
    return mess;
}

void list(client_id* id){
    char* mess=build_message_list();
    sendTcp(id, mess);
    free(mess);
}

//Funzione ausiliaria per accettare/rifiutare la richiesta d'amicizia
void friend_request(client_id *id){
    char* tmp=malloc(100*sizeof(char));
    int p;
    do{
        fgets(tmp, 100, stdin);
        p=atoi(tmp);
    }
    while (p != 1 && p != 2);
    free(tmp);
    if (p == 1){
        write(id->fdTCP, FORMAT_OKIRF, LENGTH_HEADER+LENGTH_END_SYMBOL);
    }
    else{
        write(id->fdTCP, FORMAT_NOKRF, LENGTH_HEADER+LENGTH_END_SYMBOL);
    }
    char ack[LENGTH_HEADER+LENGTH_END_SYMBOL+1];
    read_all(id, ack, LENGTH_HEADER+LENGTH_END_SYMBOL);
    fprintf(id->log, "[LOG]: %s\n", ack);
}

//Funzione ausiliaria per costruire messaggio consu
char* build_message_consu(){
    char* mess=malloc(LENGTH_CONSU+1);
    strcpy(mess, FORMAT_CONSU);
    return mess;
}

void consu(client_id* id){
    char* mess=build_message_consu();
    sendTcp(id, mess);
    free(mess);
}

void read_consu(client_id *id){
    //TODO eliminare struttura dati notifiche e fare in modo di riceverle da offline
    char buf[MAX_BUF+1];  //Massima lunghezza del buffer, per evitare messaggi che non terminano mai con +++
    int total = 0;
    int r;
    while (total < MAX_BUF - 1) {
        char c;
        r = read(id->fdTCP, &c, 1);
        if (r == 0){
            clear_screen();
            fprintf(id->log, "[LOG]: Connessione persa\n");
            close(id->fdTCP);
            close(id->fdUDP);
            fclose(id->log);
            free(id);
            exit(EXIT_FAILURE);
        }
        buf[total++] = c;
        buf[total] = '\0';
        if (total >= LENGTH_END_SYMBOL && strcmp(buf + total - LENGTH_END_SYMBOL, END_SYMBOL) == 0) {
            fprintf(id->log, "[LOG]: CONSU RICEVUTO: %s\n", buf);
            break;
        }
    }
    char type_notification[LENGTH_HEADER+1];
    strncpy(type_notification, buf, LENGTH_HEADER);
    type_notification[LENGTH_HEADER] = '\0';


    if (strcmp(type_notification, FLUX_MESS_HEADER) == 0){
        strtok(buf, " ");
        char *ID=strtok(NULL, " ");
        char *mess=strtok(NULL, "+");
        printf("Messaggio da parte di %s:\n%s", ID, mess);
    }
    else if (strcmp(type_notification, FLUX_FLOO_HEADER) == 0){
        strtok(buf, " ");
        char *ID=strtok(NULL, " ");
        char *mess=strtok(NULL, "+");
        printf("Messaggio di inondazione da parte di %s:\n%s", ID, mess);
    }
    else if (strcmp(type_notification, FLUX_FRIE_HEADER) ==0){
        strtok(buf, " ");
        char *ID=strtok(NULL, "+");
        printf("Richiesta d'amicizia da parte dell'utente %s.\n1: Accetta\n2: Rifiuta\n\n", ID);
        friend_request(id);
    }
    else if (strcmp(type_notification, FRIEN_HEADER) ==0){
        strtok(buf, " ");
        char *ID=strtok(NULL, "+");
        printf("L'utente %s ha accettato la tua richiesta d'amicizia\n", ID);
    }
    else if (strcmp(type_notification, NOFRI_HEADER) ==0){
        strtok(buf, " ");
        char *ID=strtok(NULL, "+");
        printf("L'utente %s ha rifiutato la tua richiesta d'amicizia\n", ID);
    }
    else if (strcmp(buf, FORMAT_NOCON) ==0){
        printf("Non ci sono flussi da cosnultare\n");
    }
    else {
        fprintf(stderr, "Messaggio sconosciuto\n");
    }

    pthread_mutex_lock(&id->mtx);
    if (id->num_notifications >0)
        id->num_notifications--;
    pthread_mutex_unlock(&id->mtx);
    printf("\nPremi INVIO per continuare...");
    getchar();
}

//Funzione ausiliaria per costruire messaggio iquit
char* build_message_iquit(){
    char* mess=malloc(LENGTH_IQUIT+1);
    strcpy(mess, FORMAT_IQUIT);
    return mess;
}

void iquit(client_id* id){
    char* mess=build_message_iquit();
    sendTcp(id, mess);
}

void manage_udp_notification(int *type){
    switch (*type)
    {
    case 0:
        printf("Nuova richiesta d'amicizia\n");
        break;

    case 1 || 2:
        printf("Nuova notifica amicizia\n");
        break;
    
    case 3:
        printf("Nuovo messaggio da leggere\n");
        break;

    case 4:
        printf("Nuovo messaggio di inondazione da leggere\n");
        break;
    default:
        fprintf(stderr, "Numero notifica sconosciuto");
        break;
    }
}

void* udp_listen(void* ptr){
    client_id *id = (client_id *)ptr;
    char buf[LENGTH_UDP_NOT+1];
    struct sockaddr_in server;
    socklen_t a=sizeof(server);
    while(1){
        int rec=recvfrom(id->fdUDP,buf,LENGTH_UDP_NOT,0,(struct sockaddr *)&server,&a); //aspetta un pacchetto udp
        if (rec == -1){
            return NULL;
        }
        buf[rec]='\0';
        char sender_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server.sin_addr, sender_ip, sizeof(sender_ip));
        if (strcmp(ip_server, sender_ip) == 0){//Controllo che sia stato il server a inviare
            int type = buf[0] - '0';
            u_int8_t b1 = buf[1];
            u_int8_t b2 = buf[2];
            pthread_mutex_lock(&id->mtx);
            id->num_notifications = (b2 << 8) | b1;
            pthread_mutex_unlock(&id->mtx);
            manage_udp_notification(&type);
        }
    }
}
