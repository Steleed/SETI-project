#include "server.h"

//Funzione ausiliaria per il parsing dei messaggi ricevuti
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

int find_user_index(char* id){
    pthread_mutex_lock(&usersListMutex);
    for (int i=0; i<MAX_USERS; i++){
        if (strcmp(users[i].ID, id) == 0){
            pthread_mutex_unlock(&usersListMutex);
            return i;
        }
    }
    pthread_mutex_unlock(&usersListMutex);
    return -1;
}

int regis(int sock, char* buffer){
    char id[LENGTH_ID + 1];
    uint16_t password;
    char port[LENGTH_UDP_PORT + 1];
    strncpy(id, buffer + 6, LENGTH_ID);
    id[LENGTH_ID] = '\0';
    strncpy(port, buffer + 15, LENGTH_UDP_PORT);
    port[LENGTH_UDP_PORT] = '\0';
    unsigned char p1 = (unsigned char)buffer[20];
    unsigned char p2 = (unsigned char)buffer[21];
    password = (uint16_t)p1 | ((uint16_t)p2 << 8);
    pthread_mutex_lock(&usersListMutex);
    if (registeredUsers>=MAX_USERS) { //controllo se c'è spazio
        pthread_mutex_unlock(&usersListMutex);
        sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, sock); // Server pieno
        return -1;
    }
    
    for (int i = 0; i < registeredUsers; i++) { //controllo nome doppio
        if (strcmp(users[i].ID, id) == 0) {
            pthread_mutex_unlock(&usersListMutex);
            sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, sock); // Utente già esistente
            return -1;
        }
    }

    int i = registeredUsers;
    strncpy(users[i].ID, id, LENGTH_ID);
    users[i].password=password; 
    strncpy(users[i].udpPort, port, LENGTH_UDP_PORT);
    users[i].socketTCP=sock;
    users[i].pendingMessages=NULL;
    users[i].pendingCount=0;

    socklen_t len = sizeof(users[i].clientAddr);
    getpeername(sock, (struct sockaddr*)&users[i].clientAddr, &len);
    users[i].clientAddr.sin_port = htons(atoi(port));
    pthread_mutex_init(&users[i].userMutex, NULL);
    registeredUsers++;
    pthread_mutex_unlock(&usersListMutex);
    printf("[REGIS] Utente %s registrato con successo (Indice: %d)\n", id, i);
    sendTCP(FORMAT_WELCO,LENGTH_HEADER+LENGTH_END_SYMBOL , sock);
    return i;
}

int conne(int sock, char* buffer){
    char id[LENGTH_ID+1];
    uint16_t password;
    memcpy(id,buffer+6,LENGTH_ID); 
    id[LENGTH_ID] = '\0';
    unsigned char p1 = (unsigned char)buffer[15];    
    unsigned char p2 = (unsigned char)buffer[16];
    password=(uint16_t)p1 | ((uint16_t)p2<<8);
    pthread_mutex_lock(&usersListMutex);
    int found=-1;
    for (int i=0;i<registeredUsers;++i) { //cerca l'utente
        if (strcmp(users[i].ID,id)==0) {
            found=i;
            break;
        }
    }
    if(found==-1) //utente non trovato
    {
        pthread_mutex_unlock(&usersListMutex);
        sendTCP(FORMAT_GOBYE,LENGTH_HEADER+LENGTH_END_SYMBOL,sock);
        return -1; 
    }
    if(users[found].password!=password) //password sbagliata
    {
        pthread_mutex_unlock(&usersListMutex);
        sendTCP(FORMAT_GOBYE,LENGTH_HEADER+LENGTH_END_SYMBOL,sock);
        return -1;
    }
    pthread_mutex_lock(&users[found].userMutex);
    users[found].socketTCP = sock;
    struct sockaddr_in tmp_addr;
    socklen_t l = sizeof(tmp_addr);
    if (getpeername(sock, (struct sockaddr*)&tmp_addr, &l) == 0) {
        users[found].clientAddr.sin_addr = tmp_addr.sin_addr;
    }
    pthread_mutex_unlock(&users[found].userMutex);
    pthread_mutex_unlock(&usersListMutex);
    printf("[CONNE] Utente %s riconnesso (Socket %d)\n", id, sock);
    sendTCP(FORMAT_HELLO,LENGTH_HEADER+LENGTH_END_SYMBOL, sock);
    return found;
}
    
void send_udp_notification(int userIndex){
    pthread_mutex_lock(&users[userIndex].userMutex);
    sendto(socketUDP, users[userIndex].pendingMessages->content, strlen(users[userIndex].pendingMessages->content), 
        0, (struct sockaddr *)&users[userIndex].clientAddr, (socklen_t)sizeof(struct sockaddr_in));
    pthread_mutex_unlock(&users[userIndex].userMutex);  
}


int frie(char* buffer, int index){
    if (index == -1)  return -1;
    char friend_id[LENGTH_ID+1];
    if (sscanf(buffer, FORMAT_FRIE, friend_id) != 1){
        sendTCP(FORMAT_NOFRIE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    int friend_index=find_user_index(friend_id);
    if (friend_index==-1 || friend_index == index){ //Controllo se friend_index esiste e che non stia cercando di richiedere amicizia a se stesso
        sendTCP(FORMAT_NOFRIE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    sendTCP(FORMAT_OKFRIE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
    send_udp_notification(index);
    //TODO inserimento flusso al destinatario
}

void mess(){
    
}

void list(int sock){
    char buf[20];
    pthread_mutex_lock(&usersListMutex);
    sprintf(buf,"RLIST %03d+++",registeredUsers);
    sendTCP(buf, strlen(buf), sock);
    for(int i=0;i<registeredUsers;++i){
        sprintf(buf,"LINUM %s+++", users[i].ID);
        sendTCP(buf, strlen(buf), sock);
    }
    pthread_mutex_unlock(&usersListMutex);
    printf("[LIST] Inviata lista di %d utenti\n", registeredUsers);
}

void iquit(int sock,int index){
    sendTCP(FORMAT_GOBYE,LENGTH_HEADER+LENGTH_END_SYMBOL,sock);
    if(index!=-1)
    {
        pthread_mutex_lock(&users[index].userMutex);
        users[index].socketTCP = -1;
        pthread_mutex_unlock(&users[index].userMutex);
        printf("[IQUIT] Utente %s disconnesso correttamente.\n", users[index].ID);
    }
    else
        printf("[IQUIT] Connessione anonima chiusa.\n");
}