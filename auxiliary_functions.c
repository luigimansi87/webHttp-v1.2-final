#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers/auxiliary_functions.h"
#include "headers/socket_functions.h"
#include "headers/log.h"

// Funzione ausiliaria per dividere una richiesta in singole linee terminate da un \0

int splitLines(char *buffer, char **line_buffer) {
	size_t cur_line;
	char *l_start, *l_end;

	cur_line = 0;
	l_start = buffer;
	l_end = buffer;

	while ('\0' != l_end[1]) {

		l_end++;
		if ('\n' == l_end[0]) {

			if ((size_t) (l_end - l_start) > 1) {
				line_buffer[cur_line] = calloc(800, sizeof(char));
				if (NULL == line_buffer[cur_line])
					return -1;

				strncpy(line_buffer[cur_line++], l_start,
						(size_t) (l_end - l_start));
				l_start = ++l_end;
			} else if ((1 == (size_t) (l_end - l_start))
					&& (l_start[0] == l_end[0]))
				return cur_line;
		}
	}

	return cur_line;
}

/* Data una stringa in input, verifica la presenza di terminatori/blank/CR/newline e ritorna la stringa "pulita"
 * input -> stringa da pulire
 * output -> stringa pulita
 * start -> indice del carattere di partenza
 */

int scan(char *input, char *output, int start) {
	if (start >= strlen(input))
		return -1;
	int appending_char_count = 0;
	int i = start;
	for (; i < strlen(input); i++) {
		if (*(input + i) != '\t' && *(input + i) != ' ' && *(input + i) != '\n'
				&& *(input + i) != '\r') {
			*(output + appending_char_count) = *(input + i);
			appending_char_count += 1;
		} else
			break;
	}
	*(output + appending_char_count) = '\0';
	i += 1;
	for (; i < strlen(input); i++) {
		if (*(input + i) != '\t' && *(input + i) != ' ' && *(input + i) != '\n'
				&& *(input + i) != '\r')
			break;
	}
	return i;
}


