#include "server.h"


//Funzione ausliaria per parsing messaggi
int parser(char mess[], int l){
    if (l < LENGTH_HEADER) return -1;
    if (strncmp(mess, REGIS_HEADER, LENGTH_HEADER) == 0) return 1;
    if (strncmp(mess, CONNE_HEADER, LENGTH_HEADER) == 0) return 2;
    if (strncmp(mess, FRIE_HEADER, LENGTH_HEADER) == 0) return 3;
    if (strncmp(mess, MESS_HEADER, LENGTH_HEADER) == 0) return 4;
    if (strncmp(mess, FLOO_HEADER, LENGTH_HEADER) == 0) return 5;
    if (strncmp(mess, LIST_HEADER, LENGTH_HEADER) == 0) return 6;
    if (strncmp(mess, CONSU_HEADER, LENGTH_HEADER) == 0) return 7;
    if (strncmp(mess, IQUIT_HEADER, LENGTH_HEADER) == 0) return 8;
    return -1;
}

//Funzione ausiliaria per mandare i messaggi al client
void sendTCP(char *type, int size,  int sock){
    write(sock, type, size);
}

void regis(int sock, char* buffer){
    char id[LENGTH_ID + 1];
    char password[20];
    char port[LENGTH_UDP_PORT + 1];
    if (sscanf(buffer, "REGIS %s %s %s", id, password, port) != 3)
        return;
    
    pthread_mutex_lock(&usersListMutex);
    if (registeredUsers>=MAX_USERS) { //controllo se c'è spazio
        pthread_mutex_unlock(&usersListMutex);
        sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, sock); // Server pieno
        return;
    }
    
    for (int i = 0; i < registeredUsers; i++) { //controllo nome doppio
        if (strcmp(users[i].ID, id) == 0) {
            pthread_mutex_unlock(&usersListMutex);
            sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, sock); // Utente già esistente
            return;
        }
    }

    int i = registeredUsers;
    strncpy(users[i].ID, id, LENGTH_ID);
    users[i].password = (uint16_t)atoi(password); //
    strncpy(users[i].udpPort, port, LENGTH_UDP_PORT);
    users[i].socketTCP = sock;
    users[i].pendingMessages = NULL;
    users[i].pendingCount = 0;
    socklen_t len = sizeof(users[i].clientAddr);
    getpeername(sock, (struct sockaddr*)&users[i].clientAddr, &len);
    pthread_mutex_init(&users[i].userMutex, NULL);
    registeredUsers++;
    pthread_mutex_unlock(&usersListMutex);
    printf("[REGIS] Utente %s registrato con successo (Indice: %d)\n", id, i);
    sendTCP(FORMAT_WELCO,LENGTH_HEADER+LENGTH_END_SYMBOL , sock);
}

void conne(char* mess){

}
    

void frie(char* mess){
    
}

void mess(){
    
}

void* client_handler(void* socket_desc){
    int* sock=(int *)socket_desc;
    while (1){
        char buf[500];
        int r=read(*sock, buf, 500);
        if (r==0){
            printf("[LOG] CONNESSIONE PERSA\n");
            return NULL;
        }
        buf[r]='\0';
        printf("[LOG] MESSAGGIO RICEVUTO: ");
        for (int i=0; i<r; i++){
            if (buf[i]=='\0')
                printf("0");
            else
                printf("%c", buf[i]);
        }
        printf("\n[LOG] Inizio parsing messaggio\n");
        int p=parser(buf, r);
        switch (p)
        {
            case 1:
            printf("[PARSER] messaggio REGIS ricevuto\n");
            regis(*sock,buf);
            break;

            case 2:
            //TODO CONNE
            printf("[PARSER] messaggio CONNE ricevuto\n");
            /*if (conne(buf) == -1){
                return NULL;
            }*/
            break;

            case 3:
            //TODO FRIE
            printf("[PARSER] messaggio FRIE? ricevuto\n");
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
            //TODO LIST
            printf("[PARSER] messaggio LIST? ricevuto\n");
            break;
            
            case 7:
            //TODO CONSU
            printf("[PARSER] messaggio CONSU ricevuto\n");
            break;

            case 8:
            //TODO IQUIT
            printf("[PARSER] messaggio IQUIT ricevuto\n");
            break;
    
        default:
            printf("[PARSER] Messaggio non valido\n");
            sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, *sock);
            close(*sock);
            return NULL;
        }
    }
}
