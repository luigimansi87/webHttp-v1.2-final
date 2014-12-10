
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "headers/image_functions.h"
#include "headers/UAcapabilities.h"
#include "headers/config_functions.h"
#include "headers/log.h"

// Funzione per l'aggiunta dell'opzione -quality al comando

char *qualityAdapter(int quality)
{
	if (quality==100)
		return "";
	char* quality_str = malloc(20);
	char* command= malloc (20);
	strcpy (command," -quality ");
	sprintf(quality_str, "%d",quality);

	return strcat(command, quality_str);
}

// Funzione per l'aggiunta dell'opzione -size al comando

char *sizeAdapter (int width, int height )
{
	if (width==-1 || height==-1)
		return "";
	char* width_str= malloc(20);
	char* height_str=malloc(20);
	char* command= malloc (20);
	strcpy (command," -size ");

	sprintf(width_str, "%d",width);
	sprintf(height_str, "%d",height);
	strcat(strcat(width_str,"x"), height_str);

	return strcat(command,width_str);
}

// Funzione per l'aggiunta dell'opzione -colors al comando

char *colorAdapter(long colors)
{
	if (colors==-1)
		return "";
	char* colors_str= malloc(75);
	char* command= malloc (20);
	strcpy (command," -colors ");
	sprintf(colors_str, "%ld",colors);

	return strcat(command, colors_str);
}

// Funzione per elaborare l'estensione a seconda delle estensioni accettate dall'UA

char *extensionAdapter(char* acceptedExt, char* newExtension)
{

	char *tempExtension = malloc(20);
	strcpy(tempExtension,".");
	if (strstr(acceptedExt,newExtension)!=NULL || strcmp(acceptedExt, "NULL")== 0)
		return strcat(tempExtension,newExtension);
	int i = 1;
	while (acceptedExt[i]!='.')
	{
		tempExtension[i]=acceptedExt[i];
		i++;
	}
	tempExtension[i]='\0';

	return tempExtension;
}

/* Funzione in cui viene generato il filename in base alle caratteristiche richieste dall'userAgent
 * e dall'header Accept della richiesta, in modo da poter cercare eventuali versioni in cache.
 * Se l'immagine non è presente, viene generato il comando ImageMagick "convert"
 */

char *convert (char* originalFilename, user_agent UserAgent, int Quality, char* Extensions)
{
	char* imageSpecs= (char*) malloc (200);
	char* targetFilename= (char*) malloc (200);
	char* midFilename= (char*) malloc (200);
	char* command = (char*) malloc (1000);

	imageSpecs = generatePrefix( originalFilename, UserAgent, Quality);
	midFilename = createNewFilename( originalFilename, UserAgent.format, Extensions);
	targetFilename = strcat(imageSpecs,midFilename);
	command = generateCommand(originalFilename, targetFilename, UserAgent, Quality);

	return convertImage(targetFilename, command);
}


// Funzione che genera il comando in base alle informazioni reperite con le funzioni precedenti

char *generateCommand(char* originalFilename, char* newFileName, user_agent UserAgent, int Quality)
{
	char* cmd= (char*) malloc (100);
	sprintf (cmd, "convert %s%s%s%s %s", originalFilename, qualityAdapter(Quality),sizeAdapter(UserAgent.width, UserAgent.height), colorAdapter(UserAgent.colors), newFileName);
	return cmd;
}

// Funzione che computa il prefisso del file in base alle caratteristiche dell'UserAgent e della qualità

char *generatePrefix(char* originalFilename, user_agent UserAgent, int Quality )
{

	char* prefix= (char*) malloc (200);

	if ( UserAgent.width != -1 && UserAgent.height != -1)
		sprintf(prefix,"%s%sx%s%s", returnEmptyLine(Quality,1), returnEmptyLine(UserAgent.width,0), returnEmptyLine(UserAgent.height,0), returnEmptyLine(UserAgent.colors,0));
	else
		sprintf(prefix,"%s%s%s%s", returnEmptyLine(Quality,1), returnEmptyLine(UserAgent.width,0), returnEmptyLine(UserAgent.height,0), returnEmptyLine(UserAgent.colors,0));

	return prefix;
}

/* Funzione che adatta l'estensione del file a seconda di quanto indicato nell'userAgent.
 * In caso di estensione non accettata dall'UA, converte nel primo formato accettato
 */

char *generateSuffix(char* acceptedFormats, char* Extension)
{
	return extensionAdapter(acceptedFormats,Extension);
}

// Funzione che modifica il nome del file in base all'estensione richiesta

char *createNewFilename(char* originalFilename, char* acceptedFormats, char* Extension)
{
	char* newFile= (char*) malloc (200);
	int i = 0;
	while (originalFilename[i]!='.')
	{
		newFile[i]=originalFilename[i];
		i++;
	}
	newFile[i]='\0';

	strcat(newFile, generateSuffix(acceptedFormats, Extension));

	return newFile;
}

// Funzione di supporto per comporre correttamente il suffisso del file

char *returnEmptyLine(int toConv, int isQuality)
{
	char* vBuff = malloc(200);
	if ((toConv == -1 && isQuality == 0)|| ( toConv == 100 && isQuality == 1))
		return "";
	sprintf(vBuff, "%d", toConv);

	return vBuff;
}

/* Funzione che verifica l'esistenza del file richiesto dalle specifiche per UserAgent e Accept.
 * In caso di file già esistente, restituisce solo il nome del file, altrimenti prima esegue il comando
 * generato e successivamente ritorna il nome del file convertito.
 */

char *convertImage(char* filename, char* command)
{

	FILE* converted;
	FILE* check;
	char* convertLog=malloc(500);
	char* cachePath=malloc(500);

	char* pwd= (char*) malloc (300);
	sprintf(pwd,"%sres/", wwwroot);

	sprintf(cachePath,"%s%s", pwd, filename);
	if (!(check = fopen(cachePath,"r")))
	{
		chdir(pwd);
		converted = popen(command,"r");
		chdir(wwwroot);
		pclose(converted);
		sprintf(convertLog, "File non presente nella cache, convertito in: %s attraverso il comando %s",filename,command);
		Log(convertLog);
	}
	else
	{
		sprintf(convertLog, "File esistente: %s",filename);
		Log(convertLog);
		fclose(check);
	}

	return filename;
}
