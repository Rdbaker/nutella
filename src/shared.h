#ifndef SHARED_H
#define SHARED_H

#define DEFAULTPORT "8083"
#define MAXDATASIZE 10000
#define QUERY_ADDR "239.0.0.1"
#define QUERY_PORT 8081
#define RESPONSE_ADDR "239.0.0.1"
#define RESPONSE_PORT 8082
#define DEFAULT_FPS "3"
#define _XOPEN_SOURCE

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "msock.h"


char* getoption(int argc, char* argv[], char* flag);
void *get_in_addr(struct sockaddr *sa);

#endif
