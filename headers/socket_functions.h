#ifndef SOCKET_H
#define SOCKET_H

#define BUFFER_SIZE 1024
#define MAX_FILE_SIZE 8*1024
#define MAX_CONNECTIONS 3
#define TRUE 1
#define FALSE 0
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int current_socket;
int connecting_socket;
int port;

int receive(int socket);
void createSocket();
void bindSocket();
void startListener();
void handle(int socket);
void acceptConnection();
void start();

#endif
