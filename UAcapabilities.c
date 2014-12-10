#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "headers/UAcapabilities.h"

// Ricerca le caratteristiche dell'User Agent, prima scansiona il file
// Cache (contente l'elenco e le caratteristiche degli UA già conosciuti)
// altrimenti scansiona l'elenco completo di wurfl
user_agent getUserAgentCapabilities (char *header)
           {
           user_agent ret;
           ret=parse_cacheUserAgent(header);
           if (strcmp(ret.ID,"NULL")==0)
              return parse_wurflUserAgent (header);
           else
               return ret;
           }

// Ricerca User Agent nel nostro file di user agent già transitati
user_agent parse_cacheUserAgent (char *header)
{
           FILE *cache;
           user_agent temp;
           user_agent not_found_st ={-1, -1, -1, "NULL", "NULL"};
           cache= fopen("utils/cacheUA.bin","rb");
           if( cache==NULL )
               return not_found_st;
           while (fread (&temp, sizeof(user_agent), 1, cache))
                 {
                 if (strcmp(header,temp.ID)==0)
                 return temp;
                 }
           fclose(cache);
           return not_found_st;
}
/* Ricerca User Agent nel file Wurfl contenente tutti, se lo trova lo salva nel
 * file di caching
 */
 user_agent parse_wurflUserAgent (char *header)
 {
          FILE *wurfl,*cache;
          char *result=NULL;
          char *value= (char *)malloc(20);
          char buffer[1000];
          user_agent temp={-1, -1, -1, "NULL", "NULL"};
          wurfl= fopen("utils/wurfl.xml","r");//cambiare percorso
          if( wurfl==NULL )
               {
               perror("Errore in apertura del file");
               return temp;
               }
          while(fgets(buffer, 1000, wurfl))
          {
          
          if (strstr(buffer,header)!=NULL)
             {

                      strcpy(temp.ID,header);
                      fgets(buffer, 1000, wurfl);
                      while (strstr(buffer,"</device>")==NULL)
                      {
                            if (strstr(buffer,"resolution_width")!=NULL)
                               {
                                result=strstr(buffer,"value")+7;
                                strncpy(value,result,strlen(result)-3);
                                temp.width=atoi(value);
                               }
                            else if (strstr(buffer,"resolution_height")!=NULL)
                               {
                               result=strstr(buffer,"value")+7;
                               strncpy(value,result,strlen(result)-3);
                               temp.height=atoi(value);
                               }
                            else if (strstr(buffer,"image_format")!=NULL)
                            {
                             while (strstr(buffer,"</group>")==NULL)
                             {
                               fgets(buffer, 1000, wurfl);
                               if (strstr(buffer,"jpg")!=NULL)
                               {
                                    result=strstr(buffer,"jpg");
                                    if (strstr(result,"true")!=NULL)
                                       {
                                            if (strcmp(temp.format,"NULL")==0)
                                                strcpy(temp.format,".jpg");
                                            else
                                                strcat(temp.format,".jpg");
                                       }
                               }
                            else if (strstr(buffer,"gif")!=NULL)
                               {
                                    result=strstr(buffer,"gif");
                                    if (strstr(result,"true")!=NULL)
                                       {
                                            if (strcmp(temp.format,"NULL")==0)
                                                strcpy(temp.format,".gif");
                                            else
                                                strcat(temp.format,".gif");
                                       }
                               }
                            else if (strstr(buffer,"png")!=NULL)
                               {
                                    result=strstr(buffer,"png");
                                    if (strstr(result,"true")!=NULL)
                                       {
                                            if (strcmp(temp.format,"NULL")==0)
                                                strcpy(temp.format,".png");
                                            else
                                                strcat(temp.format,".png");
                                       }
                               }
                            else if (strstr(buffer,"wbmp")!=NULL)
                               {
                                    result=strstr(buffer,"wbmp");
                                    if (strstr(result,"true")!=NULL)
                                       {
                                            if (strcmp(temp.format,"NULL")==0)
                                                strcpy(temp.format,".wbmp");
                                            else
                                                strcat(temp.format,".wbmp");
                                       }
                               }
                            else if (strstr(buffer,"bmp")!=NULL)
                               {
                                    result=strstr(buffer,"bmp");
                                    if (strstr(result,"true")!=NULL)
                                       {
                                            if (strcmp(temp.format,"NULL")==0)
                                                strcpy(temp.format,".bmp");
                                            else
                                                strcat(temp.format,".bmp");
                                       }
                               }
                            else if (strstr(buffer,"colors")!=NULL)
                               {
                                    result=strstr(buffer,"value")+7;
                                    strncpy(value,result,strlen(result)-3);
                                    temp.colors=atol(value);
                               }
                            }
                      }
                      fgets(buffer, 1000, wurfl);
             }
             fclose(wurfl);
             break;
             }
          }
          if (strcmp(temp.ID,"NULL")!=0)
          {
            cache= fopen("utils/cacheUA.bin","ab");
            if( cache==NULL )
            cache= fopen("utils/cacheUA.bin","wb");
            fwrite (&temp,1,sizeof(user_agent),cache);
            fclose(cache);
          }
          return temp;
}
