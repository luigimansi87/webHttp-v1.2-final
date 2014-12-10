#ifndef UACAPABILITIES_H
#define UACAPABILITIES_H
//Header delle funzioni di ricerca su Wurfl.xml

typedef struct   user_agent {
                 int width;
                 int height;
                 long int colors;
                 char format[200];
                 char ID[200];
                 } user_agent;                 

user_agent parse_cacheUserAgent (char *header);
user_agent parse_wurflUserAgent (char *header);
user_agent getUserAgentCapabilities (char *header);

#endif
