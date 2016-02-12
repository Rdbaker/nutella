#include "shared.h"

char* getoption(int argc, char* argv[], char* flag) {
  for(int i=0; i < argc; i++) {
    if(strcmp(argv[i], flag) == 0) {
       return argv[++i];
    }
  }
  return NULL;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}



