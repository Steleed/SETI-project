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
    int sock=socket(PF_INET, SOCK_STREAM, 0);
    if (bind(sock, (struct sockaddr *)&address_sock, sizeof(struct sockaddr_in))==EOF){
        perror("errore bind");
        return EXIT_FAILURE;
    }
    if (listen(sock, 0)==EOF){
        perror("errore server");
        return EXIT_FAILURE;
    }  
    struct sockaddr_in caller;
    socklen_t size=sizeof(caller);

    //Socket UDP
    socketUDP=socket(PF_INET, SOCK_DGRAM, 0);
    
    while(1){
        int *sockCaller=(int *)malloc(sizeof(int));
        *sockCaller=accept(sock, (struct sockaddr *)&caller, &size);
        if (*sockCaller >= 0){
            printf("[LOG] nuova connessione (%s)\n", inet_ntoa(caller.sin_addr));
            pthread_t t1;
            pthread_create(&t1, NULL, client_handler, sockCaller);
            pthread_detach(t1); 
        }
    }
    return EXIT_SUCCESS;
}

void* client_handler(void* socket_desc){
    int* sock=(int *)socket_desc;
    int p;
    int index=-1; //Indice restituito dalle funzioni regis/conne
    while (1){
        char buf[500];
        int r=read(*sock, buf, 500);
        if (r==0){
            printf("[LOG] CONNESSIONE PERSA\n");
            free(sock);
            return NULL;
        }
        buf[r]='\0';
        printf("[LOG] MESSAGGIO RICEVUTO: ");
        for (int i=0; i<r; i++){
            if (buf[i] >= 32 && buf[i] <= 126)
                printf("%c", buf[i]);
            else 
                printf("\\x%02X", (unsigned char)buf[i]);
        }
        printf("\n[LOG] Inizio parsing messaggio\n");
        p=parser(buf, r);
        switch (p)
        {
            case 1:
            printf("[PARSER] messaggio REGIS ricevuto\n");
            index=regis(*sock,buf);
            if (index == -1){
                fprintf(stderr, "[REGIS] registrazione fallita\n");
                close(*sock);
                free(sock);
                return NULL;
            }
            break;

            case 2:
            printf("[PARSER] messaggio CONNE ricevuto\n");
            index=conne(*sock, buf);
            if (index == -1){
                fprintf(stderr, "[CONNE] connessione fallita");
                close(*sock);
                free(sock);
                return NULL;
            }
            break;

            case 3:
            //TODO FRIE
            printf("[PARSER] messaggio FRIE? ricevuto\n");
            if (frie(buf, index) == -1){
                fprintf(stderr, "[FRIE?] richiesta di amicizia fallita");
            }
            break;

            case 4:
            //TODO MESS
            printf("[PARSER] messaggio MESS? ricevuto\n");
            break;

            case 5:
            //TODO FLOO
            printf("[PARSER] messaggio FLOO? ricevuto\n");
            break;

            case 6:
            printf("[PARSER] messaggio LIST? ricevuto\n");
            list(*sock);
            break;
            break;
            
            case 7:
            //TODO CONSU
            printf("[PARSER] messaggio CONSU ricevuto\n");
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
            return NULL;
        }
    }
}