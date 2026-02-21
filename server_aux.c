#include "server.h"

int parser(char mess[], int l){
    if (l < LENGTH_HEADER+LENGTH_END_SYMBOL) return -1;
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

void sendTCP(char *type, int size,  int sock){
    write(sock, type, size);
}

//Funzione ausiliaria per verificare che un utente friend_id sia amico di un utente users[index]
int is_friend(int index, char* friend_id){
    pthread_mutex_lock(&users[index].userMutex);
    Friends* aux=users[index].friends;
    while (aux != NULL){
        if (strcmp(aux->friendID, friend_id) == 0){
            pthread_mutex_unlock(&users[index].userMutex);
            return 0;
        }
        aux=aux->next;
    }
    pthread_mutex_unlock(&users[index].userMutex);
    return -1;
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
    users[i].pendingFluxes=NULL;
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

//Funzione ausiliaria per inserire un nuovo flusso a userIndex
void insert_new_flux(int userIndex, char* type, char* sender, char* mess){
    FluxNode* flux=malloc(sizeof(FluxNode));
    strcpy(flux->senderID, sender);
    flux->next=NULL;
    pthread_mutex_lock(&users[userIndex].userMutex);
    users[userIndex].pendingCount++;
    char b2=(users[userIndex].pendingCount >> 8) & 255;
    char b1=users[userIndex].pendingCount & 255;
    if (strcmp(type, FRIE_HEADER) == 0){
        sprintf(flux->type, FORMAT_UDP_NOT, FRIE_NOT_UDP, b1, b2);
        sprintf(flux->content, FORMAT_FLUX_FRIE, sender);
    }
    else if (strcmp(type, MESS_HEADER) == 0){
        sprintf(flux->type, FORMAT_UDP_NOT, FRIE_NOT_UDP, b1, b2);
        sprintf(flux->content, FORMAT_FLUX_MESS, sender, mess);
    }

    //! floo in un'altra funzione
    if (users[userIndex].pendingFluxes == NULL){
        users[userIndex].pendingFluxes=flux;
        printf("[DEBUG] Messaggio aggiunto al flusso = %s\n", users[userIndex].pendingFluxes->content);
        pthread_mutex_unlock(&users[userIndex].userMutex);
        return;
    }
    FluxNode* aux=users[userIndex].pendingFluxes;
    while (aux->next != NULL){
        aux=aux->next;
    }
    aux->next=flux;
    printf("[DEBUG] Messaggio aggiunto al flusso = %s", aux->next->content);
    pthread_mutex_unlock(&users[userIndex].userMutex);
}
    
void send_udp_notification(int userIndex){
    pthread_mutex_lock(&users[userIndex].userMutex);
    sendto(socketUDP, users[userIndex].pendingFluxes->type, LENGTH_UDP_NOT, 
        0, (struct sockaddr *)&users[userIndex].clientAddr, (socklen_t)sizeof(struct sockaddr_in));
    pthread_mutex_unlock(&users[userIndex].userMutex);  
}


int frie(char* buffer, int index){
    if (index == -1 || strlen(buffer) != LENGTH_FRIE || strncmp(buffer+LENGTH_FRIE-LENGTH_END_SYMBOL, END_SYMBOL, LENGTH_END_SYMBOL) != 0){
        sendTCP(FORMAT_NOFRIE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    char friend_id[LENGTH_ID+1];
    strncpy(friend_id, buffer+LENGTH_HEADER+1, LENGTH_ID);
    friend_id[LENGTH_ID] = '\0';
    int friend_index=find_user_index(friend_id);
    if (friend_index==-1 || friend_index == index || users[friend_index].socketTCP == -1 || is_friend(index, friend_id) == 0){ //Controllo se: friend_index esiste || non stia cercando di richiedere amicizia a se stesso || a un utente già amico
        sendTCP(FORMAT_NOFRIE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    sendTCP(FORMAT_OKFRIE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
    insert_new_flux(friend_index, FRIE_HEADER, users[index].ID, NULL);
    send_udp_notification(friend_index);
    printf("[FRIE?] richiesta d'amicizia inviata all'utente %s\n", friend_id);
    return 0;
}

int mess(char* buffer, int index){
    if (index == -1)  return -1;
    char friend_id[LENGTH_ID+1];
    strncpy(friend_id, buffer+LENGTH_HEADER+1, LENGTH_ID);
    friend_id[LENGTH_ID] = '\0';
    int friend_index=find_user_index(friend_id);
    if (friend_index==-1 || friend_index == index || users[friend_index].socketTCP == -1 || is_friend(index, friend_id) == -1){ //Controllo se: friend_index esiste || non stia cercando di mandare messaggio a se stesso || a un utente non suo amico
        sendTCP(FORMAT_NOMESS, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    char tmp[503];
    strcpy(tmp, buffer+LENGTH_HEADER+1+LENGTH_ID+1);
    int l=strlen(tmp);
    if (l > MAX_MESS+3 || (tmp[l-1] != '+' || tmp[l-2] != '+' || tmp[l-3] != '+')){
        sendTCP(FORMAT_NOMESS, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    char mess[500];
    strncpy(mess, tmp, l-3);
    insert_new_flux(friend_index, MESS_HEADER, users[index].ID, mess);
    send_udp_notification(friend_index);
    sendTCP(FORMAT_OKMESS, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
    printf("[MESS?] messaggio inviato dall'utente %s all'amico %s\n", users[index].ID, friend_id);
    return 0;
}

void list(int index){
    if (index==-1)  return;
    char buf[20];
    pthread_mutex_lock(&usersListMutex);
    sprintf(buf, FORMAT_RLIST ,registeredUsers);
    sendTCP(buf, strlen(buf), users[index].socketTCP);
    for(int i=0;i<registeredUsers;++i){
        sprintf(buf, FORMAT_LINUM , users[i].ID);
        sendTCP(buf, strlen(buf), users[index].socketTCP);
    }
    pthread_mutex_unlock(&usersListMutex);
    printf("[LIST] Inviata lista di %d utenti\n", registeredUsers);
}

void consu(int sock,int index){ 
    if(index==-1) 
        return;
    pthread_mutex_lock(&users[index].userMutex);
    if (users[index].pendingFluxes==NULL) { //caso vuoto
        pthread_mutex_unlock(&users[index].userMutex);
        sendTCP(FORMAT_NOCON,LENGTH_HEADER+LENGTH_END_SYMBOL,sock);
        return;
    }
    FluxNode* flux=users[index].pendingFluxes; //prendo
    char content[500];
    strcpy(content,flux->content);
    char senderID[LENGTH_ID+1];
    strcpy(senderID,flux->senderID);

    users[index].pendingFluxes=flux->next; //rimozione
    if (users[index].pendingCount>0)
        users[index].pendingCount--;
    free(flux);
    pthread_mutex_unlock(&users[index].userMutex);
    sendTCP(content, strlen(content), sock);
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