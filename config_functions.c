#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers/config_functions.h"
#include "headers/log.h"
#include "headers/socket_functions.h"



/* Funzione che parsa il file di configurazione e ne popola i campi:
 * wwwroot -> home del webserver
 * port -> porta in ascolto
 * log_file -> posizione del file di log
 */
void init()
{
	FILE *filePointer = NULL;

	char* currentLine = malloc(100);

	wwwroot = malloc(100);
	conf_file = malloc(100);
	log_file = malloc(100);
	mime_file = malloc(600);

	conf_file = "httpd.conf";
	strcpy(mime_file, "mime.types");

	filePointer = fopen(conf_file, "r");

	if (filePointer == NULL)
	{
		fprintf(stderr, "Impossibile aprire il file httpd.conf!\n");
		exit(1);
	}

	// Ricava la root del webServer dal file di configurazione

	if (fscanf(filePointer, "%s %s", currentLine, wwwroot) != 2)
	{
		fprintf(stderr, "Errore nella prima riga del file httpd.conf!\n");
		exit(1);
	}
	// Ricava la porta dal file di configurazione

	if (fscanf(filePointer, "%s %i", currentLine, &port) != 2)
	{
		fprintf(stderr, "Errore nella seconda riga del file httpd.conf!\n");
		exit(1);
	}

	// Ricava il path del file di log dal file di configurazione

	if (fscanf(filePointer, "%s %s", currentLine, log_file) != 2)
	{
		fprintf(stderr, "Errore nella terza riga del file httpd.conf!\n");
		exit(1);
	}
	fclose(filePointer);
	free(currentLine);
}
