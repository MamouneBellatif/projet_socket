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
 
#define FALSE 0
#define TRUE 1
 int main(int argc, char  *argv[])
 {
     if(argc != 2){
         printf("Usage: %s <file>",argv[0]);
     }
     char *filename = argv[1];
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

            char type[256];
            char tmp1[1]; // pour placer 
            char tmp2[1];

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

            if(authorized==TRUE){
                printf("Autorisé\n");
            }
            else{
                printf("Interdit\n");
            }
            
            fclose(fichier);

            wait(NULL);

         break;     
     }
     return 0;
 }
 