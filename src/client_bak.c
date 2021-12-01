#define _GNU_SOURCE 

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include<sys/socket.h>
#include<arpa/inet.h>	
#include <signal.h>
#include <netdb.h>

#define h_addr h_addr_list[0] //pour vscode, enlever

#define CMD_EXIT 0
#define CMD_LIST 1
#define CMD_PUSH 2
#define CMD_FETCH 3

#define TRUE 1
#define FALSE 0

int saisieEntier(char* message){ //fonction de saisie entier avec verification et nettoyage de stdin
    int cmd;
    printf("%s :", message);
    while (scanf("%d", &cmd) != 1) //verifier que c'est un entier
    {
        printf("Erreur: Veuillez rentrer un nombre entier...\n");
        int chr;
        do
        {
            chr = getchar(); //on évacue ce qui reste dans stdin
        } while ((chr != EOF) && (chr != '\n'));
        printf("%s : ", message);
    }
    return cmd;
}

void list2(int socket){ 
    printf("Debug: list()\n");
    //rajouter gestion erreurs
    //lit le nombre de fichiers
    int fichier_count;
    char nom_fichier[256];    

    int n=0; //nb caracteres lus
    int index=0;
    int end=FALSE;
    while( end==FALSE && ((n = read( socket, nom_fichier, 256))>1)){
            if(strcmp(nom_fichier, "")==0){
                end=TRUE; //fin des fichiers
            }
            else{
                index++;
                nom_fichier[n] = '\0';
                printf("%d: %s \n", index, nom_fichier);
                bzero(nom_fichier, 256);
            }
            
    }
}

void list(int socket){ 
    printf("Debug: list()\n");
    //rajouter gestion erreurs
    //lit le nombre de fichiers
    int fichier_count;
    char nom_fichier[256];

    if(read(socket, &fichier_count, sizeof(int))==-1){ 
        perror("[-] Reception nombre fichier\n");
    }
    else {
        printf("[+] nb de fichier: %d\n", fichier_count);
    }
    int n; //nb caractere lu
    for (int i = 0; i < fichier_count; i++){
       n=read(socket, nom_fichier, 256*sizeof(char));
       nom_fichier[n]='\0';//fin chaine caractère
       printf("fichier %d: %s \n", i+1, nom_fichier);
       bzero(nom_fichier, 256);
    }

    //while read fichiers  
}

void push(){

}



void prompt(int socket){
    int cmd;
    do
    {
        cmd = saisieEntier("Commandes (entrez un entier): 1. Liste des fichiers | 2. Envoyer un fichier | 0. Quitter ");
        if(cmd>0 && cmd<=2){
            if (write(socket, &cmd,sizeof(int))==-1){
                perror("[+] erreur envoie commande");
            }
            //attend reponse
            switch (cmd)
            {
            case CMD_LIST:
                printf("Debug:prompt() CMD_LIST \n");
                /* attend et affiche liste  */
                ///lis le buffer et affiche la liste sscanf
                 //list(socket);//
                list2(socket);
                break;
            case CMD_PUSH:
                //envoie fichier
                push();
                break;
            default:
                printf("Debug default\n");
                break;
            }
        }
    // } while (cmd!=CMD_EXIT);
    } while (cmd!=CMD_EXIT);
}

int main(int argc, char *argv[]){
    //verifier arguments
    if(argc != 3){
        printf("Usage: %s <host> <port>\n",argv[0]); //cas mauvais argument
        exit(-1);
    }
    //declaration variables
    int socket_client;
    struct sockaddr_in server;
    struct hostent *host;
    int port=atoi(argv[2]);


    //création socket
    socket_client = socket(AF_INET, SOCK_STREAM, 0);

    // host=gethostbyname("host");
    host=gethostbyname(argv[1]);
    
     //on initialise Lhost et Lport 
     
    server.sin_family = AF_INET; //protocole internet
    server.sin_port = htons(port); //htons converti dans le bon format (netwrk byte order)
    // server.sin_addr.s_addr =INADDR_ANY; //ecoute sur toutes les interfactes
    memcpy(&server.sin_addr.s_addr, host->h_addr, sizeof(host->h_addr));

    //demande de connexion
    if(connect(socket_client, (struct sockaddr *)&server, sizeof(server))==-1){
        perror("[-] Erreur connection socket...\n");
        exit(-1);
    }
    else{
        printf("[+] Succès connection socket\n");
    }

    

    prompt(socket_client);
    
    close(socket_client);
    
   return 0;
}


    // char buffer[256];
    // char reponse[256];

    // int cmd;
    // while(1){
    //     bzero(buffer, 256);
    //     // printf("Message a ecrire:");
    //     // scanf("%s", buffer);
    //     // printf("\n");
    //     // write(socket_client, buffer,strlen(buffer));
    //     // bzero(buffer, 256);
    //     // if(read(socket_client, buffer, 256 * sizeof(char))==-1){
    //     //     perror("[-] Erreur lecture");
    //     // }
    //     // printf("message reçu: %s\n", buffer);
    // }