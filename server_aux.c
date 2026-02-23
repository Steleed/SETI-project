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
    //int cnt = users[userIndex].pendingCount;
    //if (cnt > 255) cnt = 255;
    
    if (strcmp(type, FRIE_HEADER) == 0){
        flux->type[0] = '0'; // tipo 0 = richiesta amicizia
        sprintf(flux->content, FORMAT_FLUX_FRIE, sender);
    }
    else if (strcmp(type, MESS_HEADER) == 0){
        flux->type[0] = '3'; // tipo 3 = messaggio
        sprintf(flux->content, FORMAT_FLUX_MESS, sender, mess);
    }
    else{
        flux->type[0] = '4'; // tipo 4 = messaggio di flooding
        sprintf(flux->content, FORMAT_FLUX_FLOO, sender, mess);
    }
    /*flux->type[1] = "0123456789abcdef"[cnt & 0xF];
    flux->type[2] = "0123456789abcdef"[(cnt >> 4) & 0xF];
    flux->type[3] = '\0';*/
    flux->type[1] = users[userIndex].pendingCount & 255;
    flux->type[2] = (users[userIndex].pendingCount >> 8) & 255;
    flux->type[3] = '\0';

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
    printf("[DEBUG] Messaggio aggiunto al flusso = %s\n", aux->next->content);
    pthread_mutex_unlock(&users[userIndex].userMutex);
}
    
void send_udp_notification(int userIndex){
    pthread_mutex_lock(&users[userIndex].userMutex);
    FluxNode* aux=users[userIndex].pendingFluxes;
    while (aux->next != NULL){
        aux=aux->next;
    }
    sendto(socketUDP, aux->type, LENGTH_UDP_NOT, 
        0, (struct sockaddr *)&users[userIndex].clientAddr, (socklen_t)sizeof(struct sockaddr_in));
    pthread_mutex_unlock(&users[userIndex].userMutex);  
    printf("[DEBUG] tipo notifica = %c\n", aux->type[0]);
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
    if (friend_index==-1 || friend_index == index || is_friend(index, friend_id) == 0){//Controllo se: friend_index esiste || non stia cercando di richiedere amicizia a se stesso || a un utente già amico
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
    char mess[503];
    strcpy(mess, buffer+LENGTH_HEADER+1+LENGTH_ID+1);
    int l=strlen(mess);
    if (l > MAX_MESS+3 || (mess[l-1] != '+' || mess[l-2] != '+' || mess[l-3] != '+')){
        sendTCP(FORMAT_NOMESS, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    mess[l-3] = '\0'; 
    insert_new_flux(friend_index, MESS_HEADER, users[index].ID, mess);
    send_udp_notification(friend_index);
    sendTCP(FORMAT_OKMESS, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
    printf("[MESS?] messaggio inviato dall'utente %s all'amico %s\n", users[index].ID, friend_id);
    return 0;
}

void floo_aux(int index, int* visited, char* sender, char* mess){
    pthread_mutex_lock(&users[index].userMutex);
    Friends* aux=users[index].friends;
    while (aux != NULL){
        if (users[aux->friend_index].socketTCP != -1 && visited[aux->friend_index] == 0){
            insert_new_flux(aux->friend_index, FLOO_HEADER, sender, mess);
            send_udp_notification(aux->friend_index);
            visited[aux->friend_index] = 1;
            floo_aux(aux->friend_index, visited, sender, mess);
        }
        aux=aux->next;
    }
    pthread_mutex_unlock(&users[index].userMutex);
}

int floo(char* buffer, int index){
    if (index == -1)  return -1;
    char mess[503];
    strcpy(mess, buffer+LENGTH_HEADER+1);
    int l=strlen(mess);
    if (l > MAX_MESS+3 || (mess[l-1] != '+' || mess[l-2] != '+' || mess[l-3] != '+')){
        sendTCP(FORMAT_GOBYE, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
        return -1;
    }
    sendTCP(FORMAT_FLOO_ANSWER, LENGTH_HEADER+LENGTH_END_SYMBOL, users[index].socketTCP);
    mess[l-3]='\0';

    int *visited=calloc(MAX_USERS, sizeof(int)); //Bitmask per vederese un utente ha gia ricevuto il flooding
    visited[index]=1;
    floo_aux(index, visited, users[index].ID, mess);
    printf("[FLOO] messaggio di flooding inviato\n");
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
void add_friend(int indexA, int indexB){
    Friends* nuovoA = malloc(sizeof(Friends)); //aggiunge B alla lista amici di A
    strcpy(nuovoA->friendID, users[indexB].ID);
    nuovoA->friend_index = indexB;
    nuovoA->next = NULL;
    pthread_mutex_lock(&users[indexA].userMutex);
    nuovoA->next = users[indexA].friends;
    users[indexA].friends = nuovoA;
    users[indexA].friendsCount++;
    pthread_mutex_unlock(&users[indexA].userMutex);

    Friends* nuovoB = malloc(sizeof(Friends)); //aggiunge A alla lista amici di B
    strcpy(nuovoB->friendID, users[indexA].ID);
    nuovoB->friend_index = indexA;
    nuovoB->next = NULL;
    pthread_mutex_lock(&users[indexB].userMutex);
    nuovoB->next = users[indexB].friends;
    users[indexB].friends = nuovoB;
    users[indexB].friendsCount++;
    pthread_mutex_unlock(&users[indexB].userMutex);

    printf("[FRIEND] %s e %s ora sono amici\n", users[indexA].ID, users[indexB].ID);
}

void insert_new_flux_frien(int userIndex, char* content, char tipo_udp){
    FluxNode* flux = malloc(sizeof(FluxNode)); //inzializza
    strcpy(flux->content, content);
    flux->senderID[0] = '\0';
    flux->next = NULL;

    pthread_mutex_lock(&users[userIndex].userMutex); 
    users[userIndex].pendingCount++;
    //int cnt = users[userIndex].pendingCount;
    flux->type[0] = tipo_udp;
    /*flux->type[1] = "0123456789abcdef"[cnt & 0xF];    
    flux->type[2] = "0123456789abcdef"[(cnt>>4) & 0xF]; 
    flux->type[3] = '\0';*/
    flux->type[1] = users[userIndex].pendingCount & 255;
    flux->type[2] = (users[userIndex].pendingCount >> 8) & 255;
    flux->type[3] = '\0';

    if (users[userIndex].pendingFluxes == NULL){ //inserisce il flusso nella lista
        users[userIndex].pendingFluxes = flux;
    } else {
        FluxNode* aux = users[userIndex].pendingFluxes;
        while (aux->next != NULL){
            aux = aux->next;
        }
        aux->next = flux;
    }
    pthread_mutex_unlock(&users[userIndex].userMutex);

    send_udp_notification(userIndex); //invia la notifica
    printf("[FLUX] Flusso aggiunto per %s: %s\n", users[userIndex].ID, content);
}

void consu(int sock,int index){ 
    if(index==-1) 
        return;
    pthread_mutex_lock(&users[index].userMutex);
    if (users[index].pendingFluxes==NULL) { //caso vuoto
        pthread_mutex_unlock(&users[index].userMutex);
        sendTCP(FORMAT_NOCON,LENGTH_HEADER+LENGTH_END_SYMBOL,sock);
        printf("[CONSU] non ci sono flussi da consultare\n");
        return;
    }
    FluxNode* flux=users[index].pendingFluxes; //prendo la prima notifica
    char content[500];
    strcpy(content,flux->content);
    char senderID[LENGTH_ID+1];
    strcpy(senderID,flux->senderID);

    users[index].pendingFluxes=flux->next; //rimozione
    if (users[index].pendingCount>0)
        users[index].pendingCount--;
    free(flux);
    pthread_mutex_unlock(&users[index].userMutex);

    sendTCP(content, strlen(content), sock); //manda la richiesta

    if (strncmp(content, FLUX_FRIE_HEADER, LENGTH_HEADER) == 0) { //caso richiedsta d'amicizia
        char risposta[LENGTH_HEADER+LENGTH_END_SYMBOL+1];
        int r = read(sock, risposta, LENGTH_HEADER+LENGTH_END_SYMBOL);
        risposta[r] = '\0';
        printf("[CONSU] Risposta richiesta amicizia: %s\n", risposta);
        int sender_index = find_user_index(senderID);
        if (strncmp(risposta, OKIRF_HEADER, LENGTH_HEADER)==0) { //caso accetta
            add_friend(index, sender_index);
            char frien_msg[20];
            sprintf(frien_msg, FORMAT_FRIEN, users[index].ID);
            insert_new_flux_frien(sender_index, frien_msg, '1');
        } else { //caso rifiuto
            char nofri_msg[20];
            sprintf(nofri_msg, FORMAT_NOFRI, users[index].ID);
            insert_new_flux_frien(sender_index, nofri_msg, '2'); 
        }
        sendTCP(FORMAT_ACKRF, LENGTH_HEADER+LENGTH_END_SYMBOL, sock);
    }
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