#ifndef CLIENT_H
#define CLIENT_H
  

void couleur_rouge();

void couleur_reset ();

int saisieEntier(char* message);

void saisieListe(char *message, char* liste);

void sendFile(char* nom_fichier, int socket);

void list(int socket);

void receiveFile(int socket);//

int fileCount(char* index_array, int total);

void fetch(int socket);

void prompt(int socket);

int main(int argc, char **argv);



#endif
