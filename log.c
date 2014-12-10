#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "headers/log.h"
#include "headers/auxiliary_functions.h"
#include "headers/config_functions.h"

void createLog(char* filename)
{
	FILE *file;
	time_t data;
	char* log_date;
	time (&data);
	struct tm* timeStruct = localtime(&data);
	log_date=malloc(100);
	strftime (log_date,80,"%Y%m%d-%H%M%S\n",timeStruct);
	if ((file = fopen(log_file, "a")) == NULL)
	{
		fprintf(stderr, "Can't open log file!\n");
		file = fopen(log_file, "w");
	}
		fputs(log_date, file);
		fclose(file);

}

void Log (char *message)
{
	char *output= (char*) malloc (2000);
	strcpy(output, message);
	FILE* log;
	strcat (output, "\n");
	log=fopen(log_file,"a");
	fputs(output,log);
	fclose(log);
}
