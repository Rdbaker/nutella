#ifndef SERVER_H
#define SERVER_H

#ifndef SA_RESTART
#define SA_RESTART 2
#endif

#define MOVIE_DIR "movies/"

#include "shared.h"

void waitForMovieRequest(char* port);
void sendServInfo(char* tcpport);
void movieNotFound();
void introduceSelf();
void recvQuery(char* tcpport);
void sendframes(int sockfd, char* mvpath, char* fps);
char* recvfps(int sockfd);
char* recvtitle(int sockfd);
char* searchForMovie(char* movieTitle);
void usage();
void getHelp(int argc, char* argv[]);
char* getPort(int argc, char* argv[]);
void runServer(char* port);
void sigchld_handler(int s);

#endif
