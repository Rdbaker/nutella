#ifndef SHARED_H
#define SHARED_H

#define USERNAME "rdbaker"
#define PASSWD "Sup3rS3cur3R3m0t3P4ssw0rd"
#define DEFAULTPORT "8080"
#define MAXDATASIZE 1000000
#define _XOPEN_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>


char* getoption(int argc, char* argv[], char* flag);
void *get_in_addr(struct sockaddr *sa);

#endif
