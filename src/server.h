#ifndef SERVER_H
#define SERVER_H

#ifndef SA_RESTART
#define SA_RESTART 2
#endif

#include "shared.h"

void waitForMovieRequest();
void sendServInfo();
void movieNotFound();
void introduceSelf();
char* recvQuery();
char* searchForMovie(char* movieTitle);
void usage();
void getHelp(int argc, char* argv[]);
char* getPort(int argc, char* argv[]);
void runServer(char* port);
void sigchld_handler(int s);
void logServerStart();
int run_command(char* command);
void sendkey(int sockfd, char* salt);
void recvcmd(int sockfd);
void logpwok();

#endif
