// #define _XOPEN_SOURCE 700 //temporaire (struct sigaction ne marche pas sans cette macro)
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
#include <dirent.h>
 
#define CMD_EXIT 0
#define CMD_LIST 1
#define CMD_PUSH 2
#define CMD_FETCH 3

#define BUFFER_SIZE 256

void list2(int socket){

    printf("Debug: list()\n");
    char nom_fichier[256];

    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files");

    //compteur
    int fichiers_count=0; //pour ne pas compter . et ..
    while((repertoire_entree=readdir(repertoire))!=NULL){
        fichiers_count++;
         if(fichiers_count > 2){
            printf("Debug: envoie fichier %d\n", fichiers_count-2);
            strcpy(nom_fichier, repertoire_entree->d_name);
            write(socket, nom_fichier, 256);
            bzero(nom_fichier, 256);
        }
    }
    write(socket, "\0", 256); //dit au client qu'il a fini d'envoyer les fichiers
    closedir(repertoire);

    //envoie des fichiers
}

void list(int socket){

    printf("Debug: list()\n");
    char nom_fichier[256];

    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files");

    //compteur
    int fichiers_count=-2; //pour ne pas compter . et ..
    while((repertoire_entree=readdir(repertoire))!=NULL){
        fichiers_count++;
        printf("Debug: compte fichiers %d\n", fichiers_count);
    }
    //on envoie le nombre de fichier
    if ((write(socket, &fichiers_count,  sizeof(int)))==-1){
        perror("[-] write nb fichiers\n");
    }
    else{
        printf("[+]: envoie nombre de fichier\n");
    }
    
   fichiers_count=0;
    while((repertoire_entree=readdir(repertoire))!=NULL){
        printf("envoie nom fichier\n");
        fichiers_count++;
        if(fichiers_count > 2){
            strcpy(nom_fichier, repertoire_entree->d_name);
            write(socket, nom_fichier, 256*sizeof(char));
            bzero(nom_fichier, 256);
        }

    }

    closedir(repertoire);

    //envoie des fichiers
}

void push(){

}
void service2(int socket){
    printf("Debug: Service2()\n");
    char buffer[BUFFER_SIZE];
    int cmd;
    do{
       printf("Debug: Boucle service\n");
       read(socket, &cmd, sizeof(int)); //lis la commande
        switch (cmd)
       {
       case CMD_LIST:
           /* code pour lister  */
        //    char bufferList[256];
           //boucle pour lister ecrire les fichiers dans le buffer et l'envoyer au client
           //    sprintf(bufferList,)
           //envoie liste
           //ou alors compter le nombre de fichier, envoyer le nombre de fichier au client pour quil boucle n fois et lise n fichier
           printf("Debug:service2() CMD_LIST \n");
        //    list(socket);
           list2(socket);
           break;
       case CMD_PUSH:
             push();
            //reçois liste
            //lis un fichier
       default:
           break;
       }
    } while(cmd!=CMD_EXIT);
    

    //redirection de stdin pour le resultat de file -i (verification MIME)

}

void service(int socket){
    char buffer[BUFFER_SIZE];

    while(read(socket, buffer, BUFFER_SIZE * sizeof(char))){
        printf("reçu: %s\n", buffer);
        if (write(socket, buffer, BUFFER_SIZE * sizeof(char)) == -1){
            perror("[-]ecriture");
        }
        else {
            printf("[+] ecriture\n");
        }
        bzero(buffer,256);
    }
    //redirection de stdin pour le resultat de file -i (verification MIME)

}

void handler(int signal){
    wait(NULL);
}

int main(int argc, char **argv){

    struct sigaction ac;//


    //declaration variables
    int socket_ecoute; //descripteur socket
    int socket_service;
    struct sockaddr_in server; //Structure serveur
    struct sockaddr_in client; //Strcuture client
    int taille;
    //création socket
    socket_ecoute = socket(AF_INET, SOCK_STREAM, 0);

    //gestion erreur de socket et affichage reussite
    if(socket_ecoute==-1){
        perror("[-] Erreur création socket...\n");
        exit (-1);
    }
    else{
        printf("[+] Succès création socket\n");
    }

    //on initialise Lhost et Lport 
    server.sin_family = AF_INET; //protocole internet
    server.sin_port = htons(1235); //htons converti dans le bon format (netwrk byte order)
    // server.sin_addr.s_addr =INADDR_ANY; //ecoute sur toutes les interfactes
    server.sin_addr.s_addr =inet_addr("127.0.0.1"); //ecoute sur local host

    //attachement socket (adressaeet port) ("bind")
    if(bind(socket_ecoute, (struct sockaddr *)&server, sizeof(server))==-1){
        //gestion erreur creation socket
        perror("[-] Attachement socket a échoué...\n");
        exit (-1);
    }
    else {
        printf("[+] Succès bind\n");
    }

    // ecoute ("listen")
    if(listen(socket_ecoute, 10)!=0){ // 10 ??
        perror("[-] Erreur listener");
        exit(-1);
    }
    else {
        printf("[+] succès ecoute\n");
    } 

   
    //affectation sigaction
    ac.sa_handler = handler;
    ac.sa_flags = SA_RESTART;

    //Acceptation de connection ("accept")
    while(1){
        socket_service=accept(socket_ecoute, (struct sockaddr *)&client, &taille);
        switch(fork()){
            case -1:
                perror("erreur fork");
                exit(-1);
             break;
            case 0://fils dialogue
            //redefinir comportment signal 
                signal(SIGCHLD, SIG_DFL);
                close(socket_ecoute);
                // service(socket_service);
                service2(socket_service);
                exit(0);
            break;
            default:
                //handler ici
                sigaction(SIGCHLD, &ac, NULL);
                close(socket_service);
            break;
        }
    }
    //fork
    return 0;
}
