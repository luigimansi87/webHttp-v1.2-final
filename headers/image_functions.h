#ifndef IMAGEFUNCTIONS_H
#define IMAGEFUNCTIONS_H

#include "UAcapabilities.h"
char *qualityAdapter(int quality);
char *sizeAdapter(int width, int height);
char *colorAdapter(long colors);
char *extensionAdapter(char* acceptedExt, char* newExtension);
char *convert (char* fname, user_agent uatemp, int qualitytmp, char* extensions);
char *returnEmptyLine(int toConv, int isQuality);
char *createNewFilename(char* fname, char* format, char* extensions);
char *generateCommand(char* fname, char* newFileName, user_agent uatemp, int qualitytmp);
char *generatePrefix(char* fname, user_agent uatemp, int qualitytmp );
char *generateSuffix(char* format, char* extensions);
char *createNewFilename(char* fname, char* format, char* extensions);
char *convertImage(char* filename, char* command);


#endif
