#include <stdlib.h>
#include <stdio.h>

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
    while(1){
        int *sockCaller=(int *)malloc(sizeof(int));
        *sockCaller=accept(sock, (struct sockaddr *)&caller, &size);
        if (*sockCaller != EOF){
            //TODO thread
        }
    }
    return EXIT_SUCCESS;
}