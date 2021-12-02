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
#define CMD_FETCH 1

#define BUFFER_SIZE 256

#define TRUE 1
#define FALSE 0
//taille fichier
// int nombre_fichiers;

void list(int socket){
    printf("[+]Envoie Liste fichiers\n");
    // printf("Debug: list()\n");
    char nom_fichier[256];
    
    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files");

    //compteur
    int fichiers_count=0; //pour ne pas compter . et ..
    while((repertoire_entree=readdir(repertoire))!=NULL){
        fichiers_count++;
         if(fichiers_count > 2){
            // printf("Debug: envoie nom fichier %d\n", fichiers_count-2);
            strcpy(nom_fichier, repertoire_entree->d_name);
            write(socket, nom_fichier, 256);
            bzero(nom_fichier, 256);
        }
    }
    write(socket, "\0", 256); //dit au client qu'il a fini d'envoyer les fichiers
    // write(socket, '\0', 256); //dit au client qu'il a fini d'envoyer les fichiers
    closedir(repertoire);

    // nombre_fichiers=fichiers_count;
    //envoie des fichiers
}

void checkSum(FILE *fichier){
    // char magic[]="P3\n"
}

void checkMime(char *filename){ //a appeler après reception de fichier
// string string string sscanf('%s %s %s', NULL, buffer, NULL);
     int fd[2];
     char *exec_arg[]={"file","-i", filename, NULL};
     pipe(fd);
     switch(fork()){
         case -1:
            perror("fork");
            exit(1);
            break;
         case 0:  
            //dredirection de la sortie standart pour envoyer le resultat de file -i au processus père
            close(1);
            dup(fd[1]);
            close(fd[0]);
            close(fd[1]);
            execvp("file",exec_arg);
            exit(0);
            break;
         default:;
            char buffer[256];
            close(fd[1]);
            read(fd[0],buffer,256);

            char type[256]; //contiendra le mime extrait du resultat de file -i
            char tmp1[1]; // pour placer placer le caractère vide de sscanf
            char tmp2[1]; // pour placer placer le caractère vide de sscanf

            sscanf(buffer,"%s %s %s",tmp1,type,tmp2);
            type[strlen(type)-1] = '\0'; //on enlève le point virgule
            printf("Mime: %s\n",type);

            // FILE *fichier = fopen("mimetype.txt", "r");
            FILE *fichier;
            fichier=fopen("mimetype.txt", "r");
             
             if (fichier==NULL){
                 perror("fichier\n");
             }

            char line_buffer[256]; //tempon de lecture du fichier contenant les types
            int authorized=FALSE; //boolean pour l'authorisation du fichier

            //parcours les types enregistré dans le fichier et s'arrête lorsqu'il ya une correspondance
            while(authorized==FALSE && fgets(line_buffer, sizeof(line_buffer), fichier)){ 
                line_buffer[strlen(line_buffer)-2]='\0';
                if(strncmp(line_buffer, type, sizeof(line_buffer))==0){ //correspondance avece un type
                    authorized=TRUE;
                }
            }

            if (strcmp(type,"image/x-portable-pixmap")){
                checkSum(fichier);
            }

            if(authorized==TRUE){
                printf("Autorisé\n");
            }
            else{
                //delete fichier
                printf("Interdit\n");
            }
            
            fclose(fichier);

            wait(NULL);

         break;     
     }
}

void sendFile(char* nom_fichier, int socket){ //n'arrive pas a telecharger plusieurs fichier et ne telecharge pas jusqua la fin
    //envoie du fichier nom_fichier
    char *dir="../files";
    char path[256];
    int stat;
    int taille;
    char buffer_ok[256];

    sprintf(path, "%s/%s",dir, nom_fichier); //concatene repertoire au nom du fichier

    FILE *fichier = fopen(path, "rb"); //on ouvre le fichier en mode binaire  
    // FILE *fichier = fopen(path, "r"); //on ouvre le fichier en mode binaire  
    
    if (fichier == NULL) {
        perror("[-]Erreur ouverture fichier.\n");
        // exit(-1);
    }
    else{
        printf("[+]Ouverture fichier %s\n", path);
    }

    printf("writing name \n");
    do{
        stat=write(socket, nom_fichier, strlen(nom_fichier)); //Envoie le chemin et nom du fichier
    }while(stat<0);

    printf("reading name ok\n");
    do{
        stat=read(socket, buffer_ok, 256); //attend ok du client pour commencer a telecharger le fichier
    }while(stat<0);

    if (strncmp(buffer_ok, "file_ok", 7)==0){ 
        printf("[+]ok nom\n");
    }
    //

    char send_buffer[256]; 

    

    fseek(fichier, 0, SEEK_END);
    taille = ftell(fichier);
    fseek(fichier, 0, SEEK_SET);
    printf("Taille: %i\n",taille);
    //Send Picture Size
    printf("writing size\n");
    do{
        stat=write(socket, (void *)&taille, sizeof(int));
    }while (stat<0);
    
    printf("reading size_ok\n");
    do{
        stat=read(socket, buffer_ok, 256); //attend ok du client pour commencer a telecharger le fichier
    }while(stat<0);

    if (strncmp(buffer_ok, "taille_ok", strlen("taille_ok"))==0){ 
        printf("[+]Debut telechargement (ok taille)\n");
    }


    // int nb = fread(send_buffer, 1, sizeof(send_buffer), fichier);
    int nb;
    while (!feof(fichier))
    {
        nb = fread(send_buffer, 1, sizeof(send_buffer), fichier);
        printf("[+]Telechargement... %d octets\n", nb);
        do{
            stat=write(socket, send_buffer, nb);
        }while(stat<0);
        
 
    }

    do{//attend la confirmation que le client a fini de telecharger
        stat=read(socket, send_buffer, sizeof(send_buffer));
    }while(stat<0);

    // nb=write(socket, "fin_fichier", strlen("fin_fichier")); //on indique au client la fin de l'envoie
     //on indique au client la fin de l'envoie
    // printf("Send stop nb: %d octets\n", nb);
    fclose(fichier);    

}

void checkIndex(){

}

void fileExists(){
    
}
void getFileNames(int socket, char* index_array, int size){
    //prendre en compte les index a deux chiffres

    int index; //index de fichier reçu
    int *index_list; // tableau des index a envoyer
    int compteur_fichier=0; //nombre de fichier a envoyer

    
    for (int i = 0; i < size; i++) //on extrait les indices de la chaine reçu par le client
    {
        if(index_array[i]!=' '){
            if( ((i+1) < size) && index_array[i+1]!=' '){// si c'est un nombre a deux chiffre on ne compte que le deuxieme chiffre
                
            }
            else{
                compteur_fichier++;
            }
        }
    }

    // do{
    //     stat=write(socket, &compteur_fichier, sizeof(int));
    // }while(stat<0);
    

    index_list=malloc(compteur_fichier*sizeof(int)); //on alloue la taille de la liste d'indice
    int stat;
    char tmp[2];
    int cpt=0; //
    int double_digit=FALSE;
    for (int i = 0; i < size; i++) //on erempli le tableau d'index
    {
        if(index_array[i]!=' '){ //on prend en compte les nombres a deux chiffres
            if( ((i+1) < size) && index_array[i+1]!=' '){ //si caractère non nulet caracère suivant non nul
                tmp[0]=index_array[i]; //on ajoute ces deux caractère dans un tempon
                tmp[1]=index_array[i+1];
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
    
  
    DIR *repertoire;
    struct dirent *repertoire_entree;
    repertoire=opendir("../files");
    
    int i=-1;
    printf("Fichiers a envoyer: \n");
    
    while((repertoire_entree=readdir(repertoire))!=NULL){
    int file_exists=FALSE;
        if(i>=1){ //on ne prend pas en compte ./ et ../
            for(int j = 0; j<compteur_fichier; j++){ //on parcours la liste d'indice pour voir le fichier correspondant 
                if(i==index_list[j]){
                  file_exists=TRUE;
                  printf("fichier: %s\n", repertoire_entree->d_name);
                  sendFile(repertoire_entree->d_name, socket);
                }
            }
        }
        i++;
    }
    closedir(repertoire);
    printf("[+] Fin telechargement\n");
    
    //free(index_list)
}

void getIndex(int socket){
    // printf("getIndex()\n");
    int n;
    int cmd;
    char index_array[20]="";
    read(socket, &cmd, sizeof(int)); //verifie que l'utilisateur veux recuperer des fichiers
    // printf("Commande2: %d\n", cmd);
    if(cmd==CMD_FETCH){
        // printf("cmd_fetch\n");
        n=read(socket, index_array, 20); //lis le write de client.fetch()
        index_array[n]='\n';
        // printf("fichiers a telecharger : %s\n n=%d \n", index_array,n);
        getFileNames(socket, index_array, n);
    }
    else{

    }
}

void receive(){

}

void service2(int socket){
    // printf("Debug: Service2()\n");
    char buffer[BUFFER_SIZE];
    int cmd;
    int stat;
    do{
        printf("[...] En attente d'une commande\n");
        // printf("cmd avant %d\n", cmd);
        cmd=0; //si le client se deconnecte ou met la commande a 0
        do{
            stat=read(socket, &cmd, sizeof(int)); //lis la commande
        }while(stat<0);
        switch (cmd)
        {
        case CMD_LIST:
            /* code pour lister  */
            // printf("Debug:service2() CMD_LIST \n");
            list(socket);
            getIndex(socket);
            break;
        case CMD_PUSH:
            receive();
            cmd=0; //temporaire
                //reçois liste
                //lis un fichier
        default:
            break;
       }
    } while(cmd!=CMD_EXIT);
    

    //redirection de stdin pour le resultat de file -i (verification MIME)

}


void handler(int signal){
    wait(NULL);
}

int main(int argc, char **argv){

    if(argc != 2){
        printf("Usage: %s <port>\n",argv[0]); //cas mauvais argument
        exit(-1);
    }
    int port=atoi(argv[1]);

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
    server.sin_port = htons(port); //htons converti dans le bon format (netwrk byte order)
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
                printf("[+] Nouvelle connexion! (pid %d)\n",getpid());
                signal(SIGCHLD, SIG_DFL);
                close(socket_ecoute);
                // service(socket_service);
                service2(socket_service);
                close(socket_service);
                printf("[+]Fin connexion (pid %d)\n",getpid());
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
