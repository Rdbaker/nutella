#ifndef CLIENT_H
#define CLIENT_H
#include "shared.h"


void introduceSelf();
void noResultsFound();
char* getMovieQuery();
void sendfps(int sockfd, char* fps);
char* getFps(int argc, char* argv[]);
char* queryServer(char* query);
void recvframes(int sockfd);
void watchMovie(char* servInfo, char* movie, char* fps);
void connectToServer(char* host, char* port, char* movie, char* fps);
void sendtitle(int sockfd, char* movie);
void clearScreen();


#endif
