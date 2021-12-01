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
#include <string.h>

#define h_addr h_addr_list[0] //pour vscode, enlever

#define CMD_EXIT 0
#define CMD_LIST 1
#define CMD_PUSH 2
#define CMD_FETCH 1


#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 256
// char (*liste_fichiers)[50];
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

void saisieListe(char *message, char* liste){
    //saisie des index avex un espace entre eux
     int chr;
        do
        {
            chr = getchar(); //on évacue ce qui reste dans stdin
        } while ((chr != EOF) && (chr != '\n'));

    printf("Saisir l'index des fichier que vous souhaitez télecharger (ex: 1 3 5): \n");
    fgets(liste, 20, stdin);
    
    /* On enleve \n. */
    if ((strlen(liste) > 0) && (liste[strlen (liste) - 1] == '\n'))
        liste[strlen (liste) - 1] = '\0';
}

void list(int socket){ 
    // printf("Debug: list()\n");
    //rajouter gestion erreurs
    //lit le nombre de fichiers
    int fichier_count;
    char nom_fichier[256];    
    int n=0; //nb caracteres lus
    int index=0;
    int end=FALSE;
    printf("\n");

    
    while( end==FALSE && ((n = read( socket, nom_fichier, 256))>1)){ //lis les noms de fichiers un a un
            if(strcmp(nom_fichier, "")==0){
                end=TRUE; //fin des fichiers
            }
            else{
                index++;
                nom_fichier[n] = '\0';
                printf("\t(%d): %s \n", index, nom_fichier);
                bzero(nom_fichier, 256);
            }
            
    }
}
//flush
void receiveFile(int socket){
    char *dir="../files_client";
    char nom_fichier[256];
    char path[256];

    int n=read(socket, nom_fichier, 256); //recois  le nom du fichier reçu
    nom_fichier[n]='\0';
    //envoyer ok
    write(socket, "file_ok", strlen("file_ok")); // pour empecher le serveur d'envoyer autre chose que le nom

    printf("[...] Reception de %s\n", nom_fichier);

    sprintf(path, "%s/%s",dir, nom_fichier);

    FILE *fichier= fopen(path, "w");
    if (fichier == NULL) {
        perror("[-]Erreur ouverture fichier.\n");
        // exit(-1);
    }
    else{
        printf("[+]Ouverture fichier %s\n", path);
    }

    char buffer[256];
    int nb = read(socket, buffer, 256);
    printf("read done\n");
 
    int end=FALSE;
    int nbWrite;
    while (end==FALSE && nb > 0) {
    // while (nb > 0) {
        printf("[+]Telechargement... %d octets\n", nb);
        nbWrite=fwrite(buffer, 1, nb, fichier);
        // nbWrite=fwrite(buffer, nb, 1, fichier);
        // fwrite(p_array, 1, nb, fichier);
        nb = read(socket, buffer, 256);
        printf("ecriture fichier: %d\n", nbWrite);
        if(strncmp(buffer, "fin_fichier", 11)==0){
            printf("[+] Fin fichier\n");
            end=TRUE;
        }
        // printf("nb recu: %d\n", nb);
        // printf("strlen(buffer)=%d\n", strlen(buffer));
        // if(nb==3){ //on verifie que ce n'est pas la fin de l'envoie
        //     printf("fin");
        //     end=TRUE;
        // }
    }
    fclose(fichier);
    printf("[!] Fin Reception de %s\n", nom_fichier);

    
}

int fileCount(char* index_array){
    int size=strlen(index_array);
    int compteur_fichier=0; //nombre de fichier a envoyer
    for (int i = 0; i < size; i++) //on extrait les index de la chaine r1eçu
    {
        if(index_array[i]!=' '){
            compteur_fichier++;
        }
    }
    return compteur_fichier;
}

void fetch(int socket){
    char index_array[20]="";
    int nbFichiers;
    // scanf("%19s", index_array); //%20s limite
    saisieListe("\nSaisir l'index des fichier que vous souhaitez télecharger (ex: 1 3 5): \n", index_array);
    printf("liste: %s\n", index_array);
    if(write(socket, index_array, strlen(index_array))==-1){  //envoie les index des fichiers 
        perror("[-] ecriture index list\n");
    }
    else{
        // printf("[+] envoie index list\n");
    } //envoie la liste des fichiers a telecharger
    nbFichiers=fileCount(index_array);
    printf("nbFichiers %d\n", nbFichiers);

    for(int i = 0; i < nbFichiers; i++){
        receiveFile(socket);
    }
}

void promptList(int socket){ 
    int cmd;
    do{
        cmd= saisieEntier("\n1. Télecharger un fichier de la liste\n0. Quitter\n\nCommandes (entrez un entier) ");

    }while(cmd<0 && cmd>1);

    write(socket,&cmd,sizeof(int)); //envoie la commande au serveur
    if(cmd==CMD_FETCH){
        fetch(socket); //o
    }   
    // } while (cmd!=CMD_EXIT);

}

void push(){

}



void prompt(int socket){
    int cmd;
    do
    {
        cmd = saisieEntier("\n1. Liste des fichiers\n2. Envoyer un fichier\n0. Quitter\n\nCommande: (entrez un entier) ");
        // printf("cmd: %d\n", cmd);
        if(cmd>0 && cmd<=2){
            if (write(socket, &cmd,sizeof(int))==-1){
                perror("[+] erreur envoie commande");
            }
            //attend reponse
            switch (cmd)
            {
            case CMD_LIST:
                // printf("Debug:prompt() CMD_LIST \n");
                /* attend et affiche liste  */
                list(socket);
                promptList(socket);
                break;
            case CMD_PUSH:
                //envoie fichier
                push();
                cmd=0; //temporaire
                break;
            default:
                // printf("Debug default\n");
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
        printf("[+] Succès connection socket\n\n");
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