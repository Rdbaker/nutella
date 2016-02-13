#ifndef CLIENT_H
#define CLIENT_H
#include "shared.h"


void introduceSelf();
void noResultsFound();
char* getMovieQuery();
char* queryServer(char* query);
void watchMovie(char* servInfo);
void connectToServer();
void sendcmd(int sockfd, char* command);
void recvres(int sockfd);
void clearScreen();


#endif
