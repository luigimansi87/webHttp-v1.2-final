#ifndef AUXILIARY_FUNCTIONS_H
#define AUXILIARY_FUNCTIONS_H

#define BUFFER_SIZE 512
#define MAX_FILE_SIZE 5*1024
#define MAX_CONNECTIONS 3
#define TRUE 1
#define FALSE 0
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

typedef int bool;
enum { false, true };

int split_lines(char *buffer, char **line_buffer);
int scan(char *input, char *output, int start);

#endif
