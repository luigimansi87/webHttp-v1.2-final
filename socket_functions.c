
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include <time.h>

#include "headers/config_functions.h"
#include "headers/auxiliary_functions.h"
#include "headers/socket_functions.h"
#include "headers/log.h"


socklen_t addr_size;
struct sockaddr_in address;
struct sockaddr_storage connector;

// Funzione che elabora la richiesta proveniente dalla connessione aperta

int receive(int socket)
{
	int msgLen = 0;
	char buffer[BUFFER_SIZE];
	memset (buffer,'\0', BUFFER_SIZE);
	if ((msgLen = recv(socket, buffer, BUFFER_SIZE, 0)) == -1)
	{
		printf("Errore nella gestione della richiesta in entrata");
		return -1;
	}
	int request = getRequestType(buffer);
	if ( request == 1 )	// GET
	{
		Log(buffer);
		handleHTTPRequest(buffer, "GET");
	}
	else if ( request == 2 )	// HEAD
	{
		Log(buffer);
		handleHTTPRequest(buffer, "HEAD");
	}
	else if ( request == 0 )	// POST
	{
		Log(buffer);
		sendString("501 Not Implemented\n", connecting_socket);
	}
	else
	{
		sendString("400 Bad Request\n", connecting_socket);
	}
	return 1;
}

// Funzione per la creazione di un socket in ascolto

void createSocket()
{
	current_socket = socket(AF_INET, SOCK_STREAM, 0);
	if ( current_socket == -1 )
	{
		perror("Errore durante la creazione del socket");
		exit(-1);
	}
}

// Funzione per bindare il socket in ascolto sulla porta indicata nel file di configurazione

void bindSocket()
{
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if ( bind(current_socket, (struct sockaddr *)&address, sizeof(address)) < 0 )
	{
		perror("Errore in fase di binding della porta");
		exit(-1);
	}
}

// Inizializza il socket in ascolto

void startListener()
{
	if ( listen(current_socket, MAX_CONNECTIONS) < 0 )
	{
		perror("In ascolto sulla porta");
		exit(-1);
	}
}

// Gestione della richiesta in arrivo

void handle(int socket)
{

	if (receive((int)socket) < 0)
	{
		perror("Errore in fase di ricezione");
		exit(-1);
	}
}

// Funzione che permette di tenere il socket in ascolto

void acceptConnection()
{
	addr_size = sizeof(connector);
	connecting_socket = accept(current_socket, (struct sockaddr *)&connector, &addr_size);
	if ( connecting_socket < 0 )
	{
		perror("Errore in ascolto");
		exit(-1);
	}
	handle(connecting_socket);
	close(connecting_socket);
}

// Richiama le funzioni definite sopra
void start()
{
	createSocket();
	bindSocket();
	startListener();
	while ( 1 )
	{
		acceptConnection();
	}
}

