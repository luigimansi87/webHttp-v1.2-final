#ifndef HTTP_FUNCTIONS_H
#define HTTP_FUNCTIONS_H


typedef struct
{
	char *fullHeader;
	char *host;
	char *acc;
	char *userAgent;
	char *language;
	char *request;
} Header;

Header parse_http_header(char *buf);
int getRequestType(char *input);
int handleHTTPRequest(char *input);
int sendString(char *message, int socket);
int sendBinary(int *byte, int length);
void sendHeader(char *Status_code, char *Content_Type, int TotalSize, int socket, int HttpVersion);
char* cleanCR(char* crString);
void sendFile(FILE *fdesc, int file_size);
int checkMime(char *extension, char *mime_type);
int getHttpVersion(char *input, char *output, char *type);
int getExtension(char *input, char *output);
int getContentLength(FILE *fp);
int parseQuality(Header head, char* extension);

#endif
