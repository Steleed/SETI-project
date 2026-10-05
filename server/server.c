#include "server.h"

//Inizializzazione variabili globali
    Client users[MAX_USERS];
    int registeredUsers=0;
    int socketUDP;
    pthread_mutex_t usersListMutex = PTHREAD_MUTEX_INITIALIZER;

int main(){
    struct sockaddr_in address_sock;
    address_sock.sin_family=AF_INET;
    address_sock.sin_port=htons(6769);
    address_sock.sin_addr.s_addr=htonl(INADDR_ANY);
    int sock=socket(PF_INET, SOCK_STREAM, 0); //creazione socket tcp
    if (sock < 0){
        perror("errore socket");
        return EXIT_FAILURE;
    }
    int reuse_address = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address)) < 0){
        perror("errore setsockopt");
        close(sock);
        return EXIT_FAILURE;
    }
    if (bind(sock, (struct sockaddr *)&address_sock, sizeof(struct sockaddr_in)) < 0){
        perror("errore bind");
        close(sock);
        return EXIT_FAILURE;
    }
    if (listen(sock, 0) < 0){ //ascolto del client
        perror("errore server");
        close(sock);
        return EXIT_FAILURE;
    }  
    struct sockaddr_in caller;
    socklen_t size=sizeof(caller);

    //Socket UDP
    socketUDP=socket(PF_INET, SOCK_DGRAM, 0); //creazione socket udp per le notifiche
    
    while(1){
        int *sockCaller=(int *)malloc(sizeof(int));
        *sockCaller=accept(sock, (struct sockaddr *)&caller, &size); //creazione socket dedicato al client
        if (*sockCaller >= 0){
            printf("[LOG] nuova connessione (%s)\n", inet_ntoa(caller.sin_addr));
            pthread_t t1;
            pthread_create(&t1, NULL, client_handler, sockCaller); //creazone thread indipendente dedicato al client in client_handler
            pthread_detach(t1); 
        }
        else {
            perror("errore client");
            free(sockCaller);
        }
    }
    return EXIT_SUCCESS;
}

void* client_handler(void* socket_desc){
    int* sock=(int *)socket_desc;
    int p;
    int index=-1; //Indice restituito dalle funzioni regis/conne
    while (1){
        char buf[MAX_BUF+1];  //Massima lunghezza del buffer, per evitare messaggi che non terminano mai con +++
        int total = 0;
        int r;
        while (total < MAX_BUF - 1) {
            char c;
            r = read(*sock, &c, 1); //Per gestire meglio il terminatore (+++) preferiamo leggere un byte (char) per volta
            if (r == 0){
                printf("[LOG] Connessione persa\n");
                close(*sock);
                free(sock);
                return NULL;
            }
            buf[total++] = c;
            buf[total] = '\0';
            if (total >= LENGTH_END_SYMBOL && strcmp(buf + total - LENGTH_END_SYMBOL, END_SYMBOL) == 0) {
                printf("[LOG] MESSAGGIO RICEVUTO: ");
                break;
            }
        }
        for (int i=0; i<total; i++){
            if (buf[i] >= 32 && buf[i] <= 126)
                printf("%c", buf[i]);
            else 
                printf("\\x%02X", (unsigned char)buf[i]);
        }
        printf("\n[LOG] Inizio parsing messaggio\n");
        p=parser(buf, total);
        switch (p)
        {
            case 1:
            printf("[PARSER] messaggio REGIS ricevuto\n");
            index=regis(*sock,buf,total);
            if (index == -1){
                fprintf(stderr, "[REGIS] registrazione fallita\n");
                close(*sock);
                free(sock);
                return NULL;
            }
            break;

            case 2:
            printf("[PARSER] messaggio CONNE ricevuto\n");
            index=conne(*sock, buf, total);
            if (index == -1){
                fprintf(stderr, "[CONNE] connessione fallita\n");
                close(*sock);
                free(sock);
                return NULL;
            }
            break;

            case 3:
            printf("[PARSER] messaggio FRIE? ricevuto\n");
            if (frie(buf, index, total) == -1){
                fprintf(stderr, "[FRIE?] richiesta di amicizia fallita\n");
            }
            break;

            case 4:
            printf("[PARSER] messaggio MESS? ricevuto\n");
            if (mess(buf, index) == -1){
                fprintf(stderr, "[MESS?] invio messaggio fallito\n");
            }
            break;

            case 5:
            printf("[PARSER] messaggio FLOO? ricevuto\n");
            if (floo(buf,index) == -1){
                fprintf(stderr, "[FLOO?] invio messaggio flooding fallito\n");
                /*close(*sock);
                free(sock);*/
            }
            break;

            case 6:
            printf("[PARSER] messaggio LIST? ricevuto\n");
            list(index);
            break;
            break;
            
            case 7:
            printf("[PARSER] messaggio CONSU ricevuto\n");
            consu(*sock,index);
            break;

            case 8:
            printf("[PARSER] messaggio IQUIT ricevuto\n");
            iquit(*sock,index);
            close(*sock);
            free(sock);
            return NULL;
    
        default:
            printf("[PARSER] Messaggio non valido\n");
            sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, *sock);
            close(*sock);
            free(sock);
            return NULL;
        }
    }
}