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


Header parse_http_header(char *buf) {

	Header head;
	int counter=0;
	head.fullHeader = buf;
	head.host='\0';
	head.acc='\0';
	head.userAgent='\0';
	char *appoggio[100];

	int max_lines = splitLines(head.fullHeader, &appoggio);

	for (counter=0; counter<max_lines;counter++)
	{
		if (counter==0)
		{
			head.request=appoggio[counter];
			head.request=cleanCR(head.request);
		}
		else if (strstr(appoggio[counter], "Host:")!=NULL)
		{
			head.host=appoggio[counter];
			head.host=head.host+6;
			head.host=cleanCR(head.host);
		}

		else if (strstr(appoggio[counter], "User-Agent:")!=NULL)
		{
			head.userAgent=appoggio[counter];
			head.userAgent=head.userAgent+12;
			head.userAgent=cleanCR(head.userAgent);
		}

		else if (strstr(appoggio[counter], "Accept:")!=NULL)
		{
			head.acc=appoggio[counter];
			head.acc=head.acc+8;
			head.acc=cleanCR(head.acc);
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
int handleHTTPRequest(char *input)
{

	char *type = (char*)malloc(50);
	char *errPath = (char*)malloc(1000);
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

	memset(errPath, '\0', 1000);
	memset(rootPath, '\0', 1000);
	memset(filename, '\0', 200);
	memset(extension, '\0', 10);
	memset(mime, '\0', 200);
	memset(httpVersion, '\0', 20);
	requestHeader=parse_http_header(input);

	int request = getRequestType(requestHeader.request);
	if ( request == 1 )	// GET
	{
		type = "GET";
	}
	else if ( request == 2 )	// HEAD
	{
		type = "HEAD";
	}
	else if ( request == 0 )	// POST
	{
		sendString("501 Not Implemented\n", connecting_socket);
	}
	else
	{
		sendString("400 Bad Request\n", connecting_socket);
	}

	if (request == 2)
		fileNameLenght = scan(requestHeader.request, filename, 6);
	else
		fileNameLenght = scan(requestHeader.request, filename, 5);

	if ( fileNameLenght >= 5 )
	{
		httpVer = getHttpVersion(requestHeader.request, httpVersion, type);
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
				strcpy(errPath, wwwroot);
				strcat(errPath, "400.html");
				fp=fopen(errPath,"rb");
				sendHeader("400 Bad Request", "text/html", getContentLength(fp), connecting_socket, httpVer);
				Log("400 Bad Request");
				sendFile(fp, getContentLength(fp));
				fclose(fp);
				free(filename);
				free(mime);
				free(extension);
				free(rootPath);
				free(errPath);
				return -1;
			}

			/* Inizializza le directory di lavoro del webServer:
			 * rootPath -> root del webserver
			 * resPath -> cartella contenente le risorse ( immagini, file di testo ecc. )
			 * errPath -> Path per i file di errore
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
				strcpy(errPath, wwwroot);
				strcat(errPath, "404.html");
				fp=fopen(errPath,"rb");
				sendHeader("404 Not Found", "text/html", getContentLength(fp), connecting_socket, httpVer);
				Log("404 Not Found");
				sendFile(fp, getContentLength(fp));
				fclose(fp);
				free(filename);
				free(extension);
				free(rootPath);
				free(errPath);
				return -1;
			}

			// Verifica il supporto del tipo MIME

			mimeSupported = checkMime(extension, mime);
			if ( mimeSupported != 1)
			{
				fclose(fp);

				strcpy(errPath, wwwroot);
				strcat(errPath, "415.html");
				fp=fopen(errPath,"rb");
				sendHeader("415 Unsupported Media Type", "text/html", getContentLength(fp), connecting_socket, httpVer);
				Log("415 Unsupported Media Type");
				sendFile(fp, getContentLength(fp));
				fclose(fp);
				free(filename);
				free(mime);
				free(extension);
				free(rootPath);
				free(errPath);
				return -1;
			}

			// Calcola la grandezza del file
			contentLength = getContentLength(fp);
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
			// Se è un'immagine, processa la qualità e le caratteristiche richieste dall'UserAgent
			if (strstr(mime,"image")!=NULL)
			{
				fclose(fp);
				strcpy(rootPath, wwwroot);

				if (requestHeader.userAgent==NULL)
					userAgent=getUserAgentCapabilities("DEFAULT USER AGENT");
				else
					userAgent=getUserAgentCapabilities(requestHeader.userAgent);

				strcat(resPath, convert(filename, userAgent, parseQuality(requestHeader, extension), extension));
				fp = fopen(resPath, "rb");
				contentLength = getContentLength(fp);
			}

			sendHeader("200 OK", mime, contentLength, connecting_socket, httpVer);
			if (request==1)
			{
				sendFile(fp, contentLength);
				Log("200 OK - File Sent");
				fclose(fp);
			}
			else
				{
				Log("200 OK - Header Sent");
				fclose(fp);
				}

			fclose(fp);
			free(filename);
			free(mime);
			free(extension);
			free(rootPath);
			return 1;
		}
		else
		{
			strcpy(errPath, wwwroot);
			strcat(errPath, "501.html");
			fp=fopen(errPath,"rb");
			sendHeader("501 - Not Implemented", "text/html", getContentLength(fp), connecting_socket, httpVer);
			Log("501 - Not Implemented");
			sendFile(fp, getContentLength(fp));
			fclose(fp);
			free(filename);
			free(mime);
			free(extension);
			free(rootPath);
			free(errPath);
			return -1;
		}
	}
	return -1;
}

// Elimina i ^M dalla stringa UserAgent dell'header

char* cleanCR(char* crString)
{
	char *crFreeString = malloc(250);
	int count = 0;

	while (crString[count]!='\r')
	{
		crFreeString[count]=crString[count];
		count++;
	}
	crFreeString[count] = '\0';
	return crFreeString;

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
	char *head = (char*)malloc(200);
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
	while((current_char = fgetc(fdesc))!= EOF)
	{
		sendBinary(&current_char, sizeof(char));
	}
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
	fclose(mimeFile);
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

int getContentLength(FILE *fp)
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
	int quality;
	char *q_start;
	char *q_end = malloc(200);
	char qmime[200]="image/";

	// Se l'header Accept: è vuoto ritorna la qualità standard 100%
	if (reqHeader.acc=='\0')
		return 100;

	// Se nell'accept è presente il mimeType image, verifica se l'estensione del file richiesto ne fa parte


	if (strstr(reqHeader.acc,strcat(qmime,imageExtension))!=NULL)
		q_start = strstr(reqHeader.acc, qmime);
	else if (strstr(reqHeader.acc,"image/*")!= NULL)
		q_start = strstr(reqHeader.acc, "image/*");
	else if  (strstr(reqHeader.acc, "*/*")!=NULL)
		q_start = strstr(reqHeader.acc, "*/*");
	else return 100;


	// Ottiene il valore della qualità dall'header Accept

	if (strstr(q_start,"q=")!=NULL){

		int j=0;
		q_start = strstr(q_start,"q=");
		q_start=q_start+2;
		if (q_start[0] == '1')
			return 100;
		else
			q_start=q_start+2;
		while (q_start[j] != ',' &&  q_start[j] != '\0')
		{
			q_end[j] = q_start[j];
			j++;
		}
		q_end[j]='\0';
		quality=atoi(q_end);
		if (j==2)
			return quality;
		return quality*10;
	}
	else return 100;
}
