#ifndef SERVER_H
#define SERVER_H

#ifndef SA_RESTART
#define SA_RESTART 2
#endif

#include "shared.h"

void usage();
void getHelp(int argc, char* argv[]);
char* getPort(int argc, char* argv[]);
void runServer(char* port);
void sigchld_handler(int s);
void logServerStart();
void logusername(char* username);
void recvusername(int sockfd);
int run_command(char* command);
char* genkey();
void sendkey(int sockfd, char* salt);
void recvpw(int sockfd, char* salt);
void recvcmd(int sockfd);
void logpwok();

#endif
