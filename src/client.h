#ifndef CLIENT_H
#define CLIENT_H
#include "shared.h"


void usage();
void getHelp(int argc, char* argv[]);
char* getHost(int argc, char* argv[]);
char* getPort(int argc, char* argv[]);
void connectToServer();
char* getCommand(int argc, char* argv[]);
void sendusername(int sockfd);
void recvkey(int sockfd, char* key);
void sendpw(int sockfd, char* key);
void sendcmd(int sockfd, char* command);
void recvres(int sockfd);


#endif
