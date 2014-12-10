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
#include "headers/UAcapabilities.h"
#include "headers/http_functions.h"
#include "headers/socket_functions.h"
#include "headers/auxiliary_functions.h"
#include "headers/image_functions.h"
#include "headers/log.h"

// Data una richiesta, isola i campi Accept, UserAgent e Host e li deposita in una struttura adHoc

Header parse_http_headers(char *buf, char * extension) {

	Header head;
	int counter=0;
	head.fullHeader = buf;
	char *appoggio[10];

	int max_lines = split_lines(head.fullHeader, &appoggio);

	for (counter=0; counter<max_lines;counter++)
	{
		if (counter==0)
		{
			head.request=appoggio[counter];
		}
		else if (strstr(appoggio[counter], "Host:")!=NULL)
		{
			head.host=appoggio[counter];
			head.host=head.host+6;
		}

		else if (strstr(appoggio[counter], "User-Agent:")!=NULL)
		{
			head.userAgent=appoggio[counter];
			head.userAgent=head.userAgent+12;
		}

		else if (strstr(appoggio[counter], "Accept:")!=NULL)
		{
			head.acc=appoggio[counter];
			head.acc=head.acc+8;
			Log(head.acc);
		}
	}
	return head;

}


/* Verifica il tipo di richiesta e ritorna:
 * 1 - GET
 * 2 - HEAD
 * 0 - POST
 * -1 - Altrimenti
 */
int getRequestType(char *input)
{

	int type = -1;
	if ( strlen ( input ) > 0 )
	{
		type = 1;
	}
	char *requestType = malloc(5);
	scan(input, requestType, 0);
	if ( type == 1 && strcmp("GET", requestType) == 0)
	{
		type = 1;
	}
	else if (type == 1 && strcmp("HEAD", requestType) == 0)
	{
		type = 2;
	}
	else if (strlen(input) > 4 && strcmp("POST", requestType) == 0 )
	{
		type = 0;
	}
	else
	{
		type = -1;
	}
	return type;
}

// Gestione della richiesta HTTP ( HEAD / GET )
int handleHTTPRequest(char *input, char *type)
{


	char *NFpath = (char*)malloc(1000);
	char *NFfilename = (char*)malloc(200);

	char *resFilename = (char*)malloc(200);
	char *resPath = (char*)malloc(1000);

	char *filename = (char*)malloc(200);
	char *rootPath = (char*)malloc(1000);
	char *extension = (char*)malloc(10);
	char *mime = (char*)malloc(200);

	user_agent userAgent;
	Header requestHeader;

	FILE *fp;

	int httpVer;
	char httpVersion[20];
	int contentLength = 0;
	int mimeSupported = 0;
	int fileNameLenght = 0;

	memset(NFpath, '\0', 1000);
	memset(NFfilename, '\0', 200);
	memset(rootPath, '\0', 1000);
	memset(filename, '\0', 200);
	memset(extension, '\0', 10);
	memset(mime, '\0', 200);
	memset(httpVersion, '\0', 20);

	if (strcmp(type,"HEAD")==0)
		fileNameLenght = scan(input, filename, 6);
	else
		fileNameLenght = scan(input, filename, 5);

	if ( fileNameLenght >= 5 )
	{
		httpVer = getHttpVersion(input, httpVersion, type);
		if ( httpVer != -1 )
		{
			// Gestisce la richiesta /

			if (fileNameLenght == 6 || fileNameLenght == 5)
			{
				strcpy(filename,"index.html");
				strcpy(extension, "html");
			}

			// Verifica che sia presente l'estensione nella richiesta

			if ( getExtension(filename, extension) == -1 )
			{
				sendHeader("400 Bad Request", mime, contentLength, connecting_socket, httpVer);
				Log("400 Bad Request\n");

				free(filename);
				free(mime);
				free(rootPath);
				free(extension);
				return 1;
			}
			/* Inizializza le directory di lavoro del webServer:
			 * rootPath -> root del webserver
			 * resPath -> cartella contenente le risorse ( immagini, file di testo ecc. )
			 * NFPath -> Path del file 404.html
			 */

			strcpy(rootPath, wwwroot);

			sprintf(resPath,"%sres/",rootPath);

			strcat(rootPath, filename);

			sprintf(resFilename,"%s%s",resPath,filename);

			fp = fopen(rootPath, "rb");

			if (fp==NULL)
				fp= fopen(resFilename, "rb");
			if (fp==NULL)
			{
				strcpy(NFpath, wwwroot);
				strcat(NFpath, "404.html");
				fp=fopen(NFpath,"rb");
				sendHeader("404 Not Found", "text/html", contentLenght(fp), connecting_socket, httpVer);
				Log("404 Not Found");
				sendFile(fp, contentLenght(fp));
				fclose(fp);
				free(filename);
				free(extension);
				free(rootPath);
				return -1;
			}

			// Verifica il supporto del tipo MIME

			mimeSupported = checkMime(extension, mime);
			if ( mimeSupported != 1)
			{
				sendHeader("400 Bad Request", mime, contentLength, connecting_socket, httpVer);
				Log("Mime not supported\n 400 Bad Request\n");
				fclose(fp);
				free(filename);
				free(mime);
				free(rootPath);
				free(extension);
				return -1;
			}

			// Calcola la grandezza del file
			contentLength = contentLenght(fp);
			if (contentLength < 0 )
			{
				Log("La dimensione del file è 0");
				free(filename);
				free(mime);
				free(extension);
				free(rootPath);
				fclose(fp);
				return -1;
			}

			// Se è una HEAD, restituisci l'intestazione
			if (strcmp(type, "HEAD")==0){
				sendHeader("200 OK", mime, contentLength, connecting_socket, httpVer);
				Log("200 OK - Header Sent ( HEAD )");
				fclose(fp);
			}

			// Se è una GET, analizza la richiesta

			else if (strcmp(type, "GET")==0)
			{
				// Se è un'immagine, processa la qualità e le caratteristiche richieste dall'UserAgent

				if (strstr(mime,"image")!=NULL)
				{

					fclose(fp);
					strcpy(rootPath, wwwroot);

					requestHeader=parse_http_headers(input, extension);
					if (requestHeader.userAgent==NULL)
						userAgent=getUserAgentCapabilities("DEFAULT USER AGENT");
					else
						userAgent=getUserAgentCapabilities(cleanUA(requestHeader.userAgent));

					strcat(resPath, convert(filename, userAgent, parseQuality(requestHeader, extension), extension));

					fp = fopen(resPath, "rb");
					contentLength = contentLenght(fp);
				}

				sendHeader("200 OK", mime, contentLength, connecting_socket, httpVer);
				sendFile(fp, contentLength);
				Log("200 OK - File Sent");

				fclose(fp);

			}
			free(filename);
			free(mime);
			free(extension);
			free(rootPath);
			return 1;
		}
		else
		{
			sendHeader("501 Not Implemented", mime, contentLength, connecting_socket,httpVer);
			Log("501 Not Implemented\n");
		}
	}
	return -1;
}

// Elimina i ^M dalla stringa UserAgent dell'header

char* cleanUA(char* userAgentString)
{
	char *uaBuff = malloc(250);
	int uaCount = 0;

	while (userAgentString[uaCount]!='\r')
	{
		uaBuff[uaCount]=userAgentString[uaCount];
		uaCount++;
	}
	uaBuff[uaCount] = '\0';
	return uaBuff;

}

// Funzionalità per visualizzare la risposta del webServer alle richieste su client

int sendString(char *message, int socket)
{
	int length, bytes_sent;
	length = strlen(message);
	bytes_sent = send(socket, message, length, 0);
	return bytes_sent;
}


// Invia fisicamente i byte del file binario richiesto

int sendBinary(int *byte, int length)
{
	int bytes_sent = 0;
	bytes_sent = send(connecting_socket, byte, length, 0);
	return bytes_sent;
}

// Invia l'header della risposta con le informazioni del file richiesto

void sendHeader(char *Status_code, char *Content_Type, int TotalSize, int socket, int HttpVersion)
{
	char *head;
	char *content_head = "\nContent-Type: ";
	char *server_head = "\nServer: PT06";
	char *length_head = "\nContent-Length: ";
	char *date_head = "\nDate: ";
	char *newline = "\n";
	char contentLength[100];
	time_t rawtime;
	time(&rawtime);

	sprintf(contentLength, "%i", TotalSize);

	if (HttpVersion == 1)
		head = "HTTP/1.1 ";
	else if (HttpVersion == 0 )
		head = "HTTP/1.0 ";

	char *message = malloc((
			strlen(head) +
			strlen(content_head) +
			strlen(server_head) +
			strlen(length_head) +
			strlen(date_head) +
			strlen(newline) +
			strlen(Status_code) +
			strlen(Content_Type) +
			strlen(contentLength) +
			28 +
			sizeof(char)) * 2);

	strcpy(message, head);
	strcat(message, Status_code);
	strcat(message, server_head);
	strcat(message, length_head);
	strcat(message, contentLength);
	strcat(message, content_head);
	strcat(message, Content_Type);
	strcat(message, date_head);
	strcat(message, (char*)ctime(&rawtime));
	strcat(message, newline);
	sendString(message, socket);
	Log(message);
	free(message);

}

// Invia il file binario richiesto

void sendFile(FILE *fdesc, int file_size)
{
	int current_char = 0;
	do{
		current_char = fgetc(fdesc);
		sendBinary(&current_char, sizeof(char));
	}
	while(current_char != EOF);
}

// Verifica l'esistenza dell'estensione del file richiesto confrontandolo con i MIME standard presenti nel file mime.types

int checkMime(char *extension, char *mime_type)
{
	char *current_word = malloc(600);
	char *mimeList = malloc(500);
	char *word_holder = malloc(600);
	char *line = malloc(350);

	int startline = 0;

	strcat(mimeList,wwwroot);
	strcat(mimeList,"utils/mime.types");

	FILE *mimeFile;

	mimeFile=fopen(mimeList, "r");
	if (mimeFile==NULL)
	{
		perror("mimeFile not Found");
		return -1;
	}

	free(mime_type);

	mime_type = (char*)malloc(200);
	memset (mime_type,'\0',200);

	while(fgets(line, 200, mimeFile) != NULL) {

		if ( line[0] != '#' )
		{
			startline = scan(line, current_word, 0);
			while ( 1 )
			{
				startline = scan(line, word_holder, startline);
				if ( startline != -1 )
				{
					if ( strcmp ( word_holder, extension ) ==0 )
					{
						memcpy(mime_type, current_word, strlen(current_word));
						free(current_word);
						free(word_holder);
						free(line);
						return 1;
					}
				}
				else
				{
					break;
				}
			}
		}
		memset (line,'\0',200);
	}
	free(current_word);
	free(word_holder);
	free(line);
	return 0;
}

// Controlla la versione di HTTP utilizzata, restituisce 0 se HTTP1.0 e 1 se HTTP1.1

int getHttpVersion(char *input, char *output, char *type)
{

	int tipo = 4;
	char *filename = malloc(100);

	if (strcmp(type,"HEAD")==0)
		tipo=5;

	int start = scan(input, filename, tipo);

	if ( start > 0 )
	{
		if ( scan(input, output, start) )
		{
			output[strlen(output)+1] = '\0';
			if ( strcmp("HTTP/1.1" , output) == 0 )
				return 1;
			else if ( strcmp("HTTP/1.0", output) == 0 )
				return 0;
			else
				return -1;
		}
		else
			return -1;
	}
	return -1;
}

// Isola l'estensione del file richiesto

int getExtension(char *input, char *output)
{
	int in_position = 0;
	int appended_position = 0;
	int i = 0;
	for ( ; i < strlen(input); i ++ )
	{
		if ( in_position == 1 )
		{
			output[appended_position] = input[i];
			appended_position +=1;
		}
		if ( input[i] == '.' )
			in_position = 1;
	}
	output[appended_position+1] = '\0';
	if ( strlen(output) > 0 )
		return 1;
	return -1;
}

// Computa la lunghezza del file richiesto

int contentLenght(FILE *fp)
{
	int filesize = 0;
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);
	return filesize;
}

// Funzione per parsare la qualità dall'header della richiesta ( Accept: )

int parseQuality(Header reqHeader, char* imageExtension)
{
	char *q_buff = malloc(200);
	int quality;
	char *q_start;
	char *q_end = malloc(200);
	char qmime[20]="image/";

	// Se l'header Accept: è vuoto ritorna la qualità standard 100%
	if (reqHeader.acc==NULL)
		return 100;

	// Se nell'accept è presente il mimeType image, verifica se l'estensione del file richiesto ne fa parte

	if ( strstr(reqHeader.acc, "image")!=NULL)
	{
		if (strstr(reqHeader.acc,strcat(qmime,imageExtension))!=NULL)
			q_start = strstr(reqHeader.acc, qmime);
		else
			q_start = strstr(reqHeader.acc, "image/*");
		Log(q_start);
	}

	// Altrimenti se il mimeType è globale ne imposta la qualità.

	else if  (strstr(reqHeader.acc, "*/*")!=NULL)
		q_start = strstr(reqHeader.acc, "*/*");

	// Ottiene il valore della qualità dall'header Accept

	if (strstr(q_start,"q=")!=NULL){

		int j=0;
		q_start = strstr(q_start,"q=");
		q_start=q_start+2;
		if (q_start[0] == '1')
			return 100;
		else
			q_start=q_start+2;
		while (q_start[j] != ',' && q_start[j] != '\r')
		{
			q_end[j] = q_start[j];
			j++;
		}
		q_end[j]='\0';
		quality=atoi(q_end);
		sprintf(q_buff, "%d", quality);
		Log(q_buff);
		if (j==2)
			return quality;
		return quality*10;
	}
	else return 100;
}
