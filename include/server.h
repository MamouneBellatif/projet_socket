#ifndef SERVER_H
#define SERVER_H

void service(int* socket);

void handler(int signal);

void list(int socket);

void checkMime(char *filename);

void sendFile(char* nom_fichier, int socket);

void getFileNames(int socket, char* index_array, int size);

void checkIndex();

void fileExists();

void getIndex(int socket);

void push();

void service2(int socket);


int main(int argc, char **argv);


#endif // !SERVER_H
