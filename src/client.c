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
#include <dirent.h>
#include "../include/client.h"


#define h_addr h_addr_list[0] //pour vscode, enlever

#define CMD_EXIT 0
#define CMD_LIST 1
#define CMD_LIST_LOCAL 2
#define CMD_FETCH 1
#define CMD_PUSH 2
#define CMD_DISPLAY 1


#define TRUE 1
#define FALSE 0

#define BUFFER_SIZE 256
int nbFichiers_total;
int nbFichiers_local;



void couleur_rouge() { //coloration du texte en rouge
    printf("\033[1;31m");
}
void couleur_reset () { //coloration du texte en blanc
    printf("\033[0m");
}

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

void saisieListe(char *message, char* liste){//saisie les liste d'indice
    //saisie des index avex un espace entre eux
     int chr;
        do
        {
            chr = getchar(); //on évacue ce qui reste dans stdin
        } while ((chr != EOF) && (chr != '\n'));

    printf("Saisir les indices des fichier que vous souhaitez télecharger séparés par un espace(ex:1 3 5) : \n");
    fgets(liste, 20, stdin);
    
    /* On enleve \n. */
    if ((strlen(liste) > 0) && (liste[strlen (liste) - 1] == '\n'))
        liste[strlen (liste) - 1] = '\0';
}

void list(int socket){ 

    char nom_fichier[256];    
    int n=0; //nb caracteres lus
    int index=0;
    int end=FALSE;
    printf("\n");
    int stat; //retrour de read et write


    couleur_rouge();
    while( end==FALSE && (n = read( socket, nom_fichier, 256))>1){ //lis les noms de fichiers un a un
  
            if(strncmp(nom_fichier, "fin_liste", 9)==0){
                printf("fin de liste\n");
                end=TRUE; //fin des fichiers
            }
            else{
                do{
                    stat=write(socket, "recu", 5); //ack fichier
                }while(stat<0);
                index++;
                printf("\t(%d): %s \n", index, nom_fichier);
                bzero(nom_fichier, 256);
            }
            
    }

    couleur_reset();
    nbFichiers_total=index;
    printf("fichier total: %d \n", nbFichiers_total);
}

void receiveFile(int socket){ //reception de un fichier
    char *dir="../files_client";
    char nom_fichier[256];
    char path[500];
    int stat;
    int n;

    do{
        n=read(socket, nom_fichier, 256); //recois  le nom du fichier reçu
    }while(n<0);
    nom_fichier[n]='\0';
    //envoyer ok

    do{
        stat=write(socket, "file_ok", strlen("file_ok")); // pour empecher le serveur d'envoyer autre chose que le nom
    }while(stat<0);
    printf("[...] Téléchargement en cours de %s\n", nom_fichier);

    sprintf(path, "%s/%s",dir, nom_fichier);
    printf("path: %s\n", path);
    FILE *fichier= fopen(path, "wb");
    if (fichier == NULL) {
        perror("[-]Erreur ouverture fichier.\n");
        exit(-1);
    }
 
    int taille;
    
    //recoit la taille
    do{
        stat = read(socket, &taille, sizeof(int));
    }while(stat<0);

    do{
        stat=write(socket, "taille_ok", strlen("taille_ok")); // pour empecher le serveur d'envoyer autre chose que le nom
    }while(stat<0);



    char buffer[256];
 
    int nb=0;
    int rcv_taille=0;
    int nbWrite;
    while (rcv_taille <taille) {
        do{
            nb = read(socket, buffer, 256);
        }while(nb<0);
        nbWrite=fwrite(buffer, 1, nb, fichier);
        rcv_taille += nb;

    }
    //accusé de reception du fichier
    do {
        stat=write(socket, "done", sizeof("done"));
    } while (stat<0);
    
    fclose(fichier);
    couleur_rouge();
    printf("[+] Transfert %s complet (%d octets)\n",nom_fichier, rcv_taille );
    couleur_reset();
    
}

int fileCount(char* index_array, int total){
    //compte les indice de fichier dans une chaine de caractère
    int size=strlen(index_array);
    int compteur_fichier=0; //nombre de fichier a envoyer
    char tmp[2];
    int exists=TRUE;
    for (int i = 0; i < size; i++) //on extrait les indices de la chaine reçu par le client
    {
        if(index_array[i]!=' '){
            if( ((i+1) < size) && index_array[i+1]!=' '){// si c'est un nombre a deux chiffre on ne compte que le deuxieme chiffre
                tmp[0]=index_array[i]; //on ajoute ces deux caractère dans un tempon
                tmp[1]=index_array[i+1];
                if(atoi(tmp)>total || atoi(tmp)<=0){
                    exists=FALSE;
                    printf("indice %d trop grand \n",atoi(tmp));
                }
            }
            else{
                compteur_fichier++;
            }
        }
    }
    if(exists==FALSE){
        compteur_fichier=-1;
    }
    return compteur_fichier;
}

void fetch(int socket){
    //saisie des indices et boucle de reception des fichiers
    char index_array[20]="";
    int nbFichiers;
    int stat;

    do{
        saisieListe("\nSaisir l'index des fichier que vous souhaitez télecharger (ex: 1 3 5): \n", index_array);
        printf("liste: %s\n", index_array);
        nbFichiers=fileCount(index_array, nbFichiers_total);
        if(nbFichiers==-1){
            printf("Appuiez sur Entrée pour recommencer...\n");
        }
    }while(nbFichiers==-1);
    

    do{
        stat=write(socket, index_array, strlen(index_array));
    }while(stat<0);

     //envoie la liste des fichiers a telecharger
    
    printf("nbFichiers %d\n", nbFichiers);

    for(int i = 0; i < nbFichiers; i++){
        //sycnhroniser
        printf("telechargement %d/%d\n", i+1, nbFichiers);
        receiveFile(socket);
        printf("\n");
    }
}

void promptList(int socket){ 
    //menu telechargement fichier distant
    int stat;
    int cmd;
    do{
        cmd= saisieEntier("\n1. Télecharger un fichier de la liste\n0. revenir en arrière\n\nCommandes (entrez un entier) ");
    }while(cmd<0 || cmd>1);

    do{
        stat=write(socket,&cmd,sizeof(int)); //envoie la commande au serveur
    }while(stat<0);

    if(cmd==CMD_FETCH){
        fetch(socket); //o
    }   

}

void sendFile(char *nom_fichier, int socket){
    //envoie un fichier au serveur
    char *dir="../files_client";
    char path[500];
    int stat;
    int taille;
    char buffer_ok[256];

    sprintf(path, "%s/%s",dir, nom_fichier); //concatene repertoire au nom du fichier

    FILE *fichier = fopen(path, "rb"); //on ouvre le fichier en mode binaire  
    
    if (fichier == NULL) {
        perror("[-]Erreur ouverture fichier.\n");
        // exit(-1);
    }
    else{
    }

     printf("envoie nom \n");
    do{
        stat=write(socket, nom_fichier, 256); //Envoie le chemin et nom du fichier //receive file
    }while(stat<0);

     printf("name ok\n");
    do{
        stat=read(socket, buffer_ok, 256); //attend ok du client pour commencer a telecharger le fichier
    }while(stat<0);

    if (strncmp(buffer_ok, "file_ok", 7)==0){ 
    }
    //

    char send_buffer[256]; 

    
    //taille du fichier
    fseek(fichier, 0, SEEK_END);
    taille = ftell(fichier);
    fseek(fichier, 0, SEEK_SET);
 
    printf("Taille: %i\n",taille);
    printf("envoie taille\n");
    
    do{
        stat=write(socket, (void *)&taille, sizeof(int));
    }while (stat<0);
    
    do{
        stat=read(socket, buffer_ok, 256); //attend ok du client pour commencer a telecharger le fichier
    }while(stat<0);

    if (strncmp(buffer_ok, "taille_ok", strlen("taille_ok"))==0){ 
        printf("[+]Début transfert %d octets\n", taille);
    }


    int nb;
    while (!feof(fichier))
    {
        nb = fread(send_buffer, 1, sizeof(send_buffer), fichier);
        // printf("[+]Telechargement... %d octets\n", nb);
        do{
            stat=write(socket, send_buffer, nb);
        }while(stat<0);
        
 
    }

    do{//attend la confirmation que le client a fini de telecharger
        stat=read(socket, send_buffer, 256);
    }while(stat<0);

    
    
    fclose(fichier);    

    do{ //lis le mime (type)
        stat=read(socket,send_buffer,256);
    }while(stat<0);

    couleur_rouge();
    printf("[+] %s\n", send_buffer);
    couleur_reset();
}

void push(int socket){
    //sasie et boucle d'envoie de fichier du client au serveur
    char index_array[20]="";
    int *index_list;
    int nbFichiers;
    int stat;
    
    do{
        saisieListe("\nSaisir l'indice des fichier que vous souhaitez télecharger (ex: 1 3 5): \n", index_array);
        printf("liste: %s\n", index_array);
        nbFichiers=fileCount(index_array, nbFichiers_local); //verifie que les indices sont bien présent dans le repertoire
        if(nbFichiers==-1){
            printf("Appuiez sur Entrée pour recommencer...\n");
        }
    }while(nbFichiers==-1);

   


    index_list=malloc(nbFichiers*sizeof(int));
    
    //recupere les indices a envoyer
    char tmp[2];
    int cpt=0; //
    int double_digit=FALSE;

    int size = strlen(index_array);
    for (int i = 0; i < size; i++) //on erempli le tableau d'index
    {   

        if(index_array[i]!=' '){ //on prend en compte les nombres a deux chiffres
            if( ((i+1) < size) && index_array[i+1]!=' '){ //si caractère non nulet caracère suivant non nul
                tmp[0]=index_array[i]; //on ajoute ces deux caractère dans un tempon
                tmp[1]=index_array[i+1];
                //tmp[2]='\0';
                index_list[cpt]=atoi(tmp); //on les convertit en entier
                double_digit=TRUE; //on met le boolean a vrai pour ne pas compter deux fois a cause du deuxieme chiffre
                cpt++;
                
            }
            else if(double_digit==FALSE){
                index_list[cpt]=atoi(&index_array[i]);
                cpt++;
            }
            
        }
        else{
            double_digit=FALSE;
        }
    }

   

    //boucle sur le reperoire et envoie les fichiers dont l'indice correspond
    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files_client");
    


    printf("nombre de fichiers %d \n", nbFichiers);
    printf("Fichiers a envoyer: \n");

    do{//envoie du nmbre de fichier a envoyer
        stat=write(socket, &nbFichiers, sizeof(int));
    }while(stat<0);

    char buffer_ok[100]; 
    do{
        stat=read(socket,buffer_ok,100);
    }while(stat<0);
    
    int i=-1; //iterateur -1 pour ne pas compter .. et . et pour que le premier fichier soit égal a 1
    while((repertoire_entree=readdir(repertoire))!=NULL){
    int file_exists=FALSE;
        if(i>=1){ //on ne prend pas en compte ./ et ../

            for(int j = 0; j<nbFichiers; j++){ //on parcours la liste d'indice pour voir le fichier correspondant 
                if(i==index_list[j]){
                  file_exists=TRUE;
                  couleur_rouge();
                  printf("fichier: %s\n", repertoire_entree->d_name);
                  couleur_reset();
                  sendFile(repertoire_entree->d_name, socket);
                }
            }
        }
        i++;
    }
    closedir(repertoire);
    printf("[+] Fin telechargement\n");   

}



void display(){
    //affichage d'image
    int indice=saisieEntier("Entrez l'indice du fichier que vous voulez afficher \n");
    int existe=FALSE;
    char *dir="../files_client";
    char nom_fichier[256]; //
    char path[500];
    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files_client");
    
    
    int i=-1; //pour ne pas compter . et ..
    while( (existe == FALSE) && ((repertoire_entree=readdir(repertoire))!=NULL) ){
         if(i==indice){
            printf("trouvé\n");
            strcpy(nom_fichier, repertoire_entree->d_name);
            existe=TRUE;
        }
        i++;
    }
    closedir(repertoire);

    if(existe==TRUE){
        sprintf(path, "%s/%s", dir, nom_fichier);
        switch (fork()){  
            case -1 : 
            fprintf(stderr, "Erreur de fork\n"); 
            exit(-1);
            case 0 :;
                char *arg_exec[]={"display",path, NULL}; // on peut pas initiliser des variables apres le case 
                execvp("display",arg_exec);
                exit(0);
            default : 
                wait(NULL);
                break;
        }
    }
}

void promptListLocal(int socket){
    //menu liste locale et envoie de fichier
    int cmd;
    int stat; //retour de read et write

    do{
        cmd= saisieEntier("\n1.Afficher une image \n2. Envoyer un ou des fichiers au serveur\n0. Retour\nCommandes (entrez un entier) ");
    }while(cmd<0 || cmd>2);

    switch (cmd){
    case CMD_PUSH:

        do{
            stat=write(socket, &cmd, sizeof(int)); //envoie commande au server
        }while(stat<0);
        
        push(socket); 
        break;
    case CMD_DISPLAY:
        //affiche et reviens au prompt initial
        display();
        break;

    default:
        break;
    }
 
}
void listLocal(){
    //reception et affichage des fichier distants
    char nom_fichier[256];
    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files_client");
    //compteur
    int fichiers_count=0; //pour ne pas compter . et ..
    couleur_rouge();
    while((repertoire_entree=readdir(repertoire))!=NULL){
        fichiers_count++;
         if(fichiers_count > 2){
            strcpy(nom_fichier, repertoire_entree->d_name);
            printf("(%d): %s\n",fichiers_count-2, nom_fichier);
            bzero(nom_fichier, 256);
        }
    }
    closedir(repertoire);
    nbFichiers_local=fichiers_count-2;
    couleur_reset();
}

void prompt(int socket){
    //menu principal
    int cmd; //commande
    int stat;//retour de read et write
    do
    {
        cmd = saisieEntier("\n1. Liste des fichiers (télécharger)\n2. Liste des fichiers locaux (envoyer et afficher)\n0. Quitter\n\nCommande: (entrez un entier) ");
        if(cmd>0 && cmd<=2){

            switch (cmd)
            {
            case CMD_LIST:
                do{
                    stat=write(socket, &cmd,sizeof(int)); //envoie la commande au serveru
                }while(stat<0);
                //list et recuperation
                list(socket);
                promptList(socket);
                break;
            case CMD_LIST_LOCAL:
                //liste local envoie et display
                listLocal();
                promptListLocal(socket);
                break;
            default:
                break;
            }
        }
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
    memcpy(&server.sin_addr.s_addr, host->h_addr, host->h_length);

    
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


