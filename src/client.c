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
int nbFichiers_total;

void couleur_rouge() { //coloration du texte en rouge
    printf("\033[1;31m");
}
void couleur_reset () { //coloration du texte en blanc
    printf("\033[0m");
}

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

    printf("Saisir les indices des fichier que vous souhaitez télecharger séparés par un espace(ex:1 3 5) : \n");
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

    couleur_rouge();
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
    couleur_reset();
    nbFichiers_total=index;
    printf("fichier total: %d \n", nbFichiers_total);
}
//flush
void receiveFile(int socket){
    char *dir="../files_client";
    char nom_fichier[256];
    char path[256];
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
    // printf("Taille attendu %d: (oct\n", taille);

    do{
        stat=write(socket, "taille_ok", strlen("taille_ok")); // pour empecher le serveur d'envoyer autre chose que le nom
    }while(stat<0);



    char buffer[256];
 
    int end=FALSE;
    int nbWrite;
    int nb=0;
    int rcv_taille=0;

    while (rcv_taille <taille) {
        do{
            nb = read(socket, buffer, sizeof(buffer));
        }while(nb<0);
        nbWrite=fwrite(buffer, 1, nb, fichier);
        rcv_taille += nb;

    }
    //accusé de reception du fichier
    do {
        stat=write(socket, "done", sizeof("done"));
    } while (stat<0);
    
    fclose(fichier);
    // printf("[+] Transfert complet: %d octets reçus\n",downloaded);
    couleur_rouge();
    printf("[+] Transfert %s complet (%d octets)\n",nom_fichier, rcv_taille );
    couleur_reset();
    
}

int fileCount(char* index_array){
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
                printf("atoi %d > nbfichier%d\n",atoi(tmp), nbFichiers_total);
                if(atoi(tmp)>nbFichiers_total){
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
    printf("retour fileCount: %d\n", compteur_fichier);
    return compteur_fichier;
}

void fetch(int socket){
    char index_array[20]="";
    int nbFichiers;
    int stat;
    do{
        saisieListe("\nSaisir l'index des fichier que vous souhaitez télecharger (ex: 1 3 5): \n", index_array);
        printf("liste: %s\n", index_array);
        nbFichiers=fileCount(index_array);
        if(nbFichiers==-1){
            printf("Appuiez sur Entrée pour recommencer...\n");
        }
    }while(nbFichiers==-1);
    
    // if(write(socket, index_array, strlen(index_array))==-1){  //envoie les index des fichiers 
    //     perror("[-] ecriture index list\n");
    // }
    do{
        stat=write(socket, index_array, strlen(index_array));
    }while(stat<0);
        // printf("[+] envoie index list\n");
     //envoie la liste des fichiers a telecharger
    
    printf("nbFichiers %d\n", nbFichiers);

    for(int i = 0; i < nbFichiers; i++){
        //sycnhroniser
        printf("telechargement %d/%d\n", i+1, nbFichiers);
        receiveFile(socket);
    }
}

void promptList(int socket){ 
    int cmd;
    do{
        cmd= saisieEntier("\n1. Télecharger un fichier de la liste\n0. revenir en arrière\n\nCommandes (entrez un entier) ");

    }while(cmd<0 || cmd>1);

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
    int stat;
    do
    {
        cmd = saisieEntier("\n1. Liste des fichiers\n2. Envoyer un fichier\n0. Quitter\n\nCommande: (entrez un entier) ");
        // printf("cmd: %d\n", cmd);
        if(cmd>0 && cmd<=2){
            do{
                stat=write(socket, &cmd,sizeof(int));
            }while(stat<0);
            // if (==-1){
            //     perror("[+] erreur envoie commande");
            // }
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