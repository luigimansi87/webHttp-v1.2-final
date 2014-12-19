//#define fseeko64 fseek
//#define fopen64 fopen

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "headers/log.h"
#include "headers/socket_functions.h"
#include "headers/config_functions.h"


int main(int argc, char* argv[])
{
// Inizializza la configurazione leggendo da httpd.conf
	init();
// Inizializza il file di Log
	createLog(log_file);

	printf("Settings:\n");
	printf("Porta:\t\t\t%i\n", port);
	printf("Server root:\t\t%s\n", wwwroot);
	printf("Configuration file:\t%s\n", conf_file);
	printf("Logfile:\t\t%s\n", log_file);

	chdir(wwwroot);
//Avvia il webServer
	start();
	return 0;

}


