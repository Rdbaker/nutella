#include "client.h"


int main(int argc, char* argv[]) {
  getHelp(argc, argv);
  char* host = getHost(argc, argv);
  char* port = getPort(argc, argv);
  char* command = getCommand(argc, argv);
  // connect to server
  connectToServer(host, port, command);
  // send command string
  // print response from server
}


void getHelp(int argc, char* argv[]) {
  for(int i=0; i < argc; i++) {
    if(strcmp(argv[i], "-h") == 0) {
       usage();
    }
  }
}


/*
 * Returns the host string if it's there,
 * else exist with status code 1
 */
char* getHost(int argc, char* argv[]) {
  char* host = getoption(argc, argv, "-s");
  if(host == NULL)
    usage();
  else
    return host;
  return NULL;
}


/*
 * Returns the port string if it's there,
 * else exist with status code 1
 */
char* getPort(int argc, char* argv[]) {
  char* port = DEFAULTPORT;
  char* newport = getoption(argc, argv, "-p");
  if(newport == NULL)
    return port;
  else
    return newport;
}

/*
 * get the command option from the args
 */
char* getCommand(int argc, char* argv[]) {
  char* command = getoption(argc, argv, "-c");
  if(command == NULL)
    usage();
  else
    return command;
  return NULL;
}

void connectToServer(char* host, char* port, char* command) {
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("client: connect");
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    exit(2);
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
      s, sizeof s);
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo); // all done with this structure

  // send username
  sendusername(sockfd);
  // recv salt
  char* salt = malloc(sizeof(char) * MAXDATASIZE);
  memset(salt, '\0', strlen(salt));
  recvkey(sockfd, salt);
  // send pw
  sendpw(sockfd, salt);
  // send cmd
  sendcmd(sockfd, command);
  // recv result
  recvres(sockfd);

  close(sockfd);

  exit(0);

}

/*
 * receive the result of the command from the server
 */
void recvres(int sockfd) {
  int numbytes;
  char buf[MAXDATASIZE];

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    printf("this failed\n");
    perror("recv");
    exit(1);
  }
  buf[numbytes] = '\0';

  printf("%s\n",buf);
}

/*
 * send the command to the server
 */
void sendcmd(int sockfd, char* command) {
  if (send(sockfd, command, strlen(command), 0) == -1)
    perror("send");
}

/*
 * send the pw to the server
 */
void sendpw(int sockfd, char* key) {
  char* crypted = crypt(PASSWD, key);
  if (send(sockfd, crypted, strlen(crypted), 0) == -1)
    perror("send");
}

/*
 * receive the random key to encrypt the password
 */
void recvkey(int sockfd, char* key) {
  int numbytes;

  if ((numbytes = recv(sockfd, key, MAXDATASIZE-1, 0)) == -1) {
      perror("recv");
      exit(1);
  }
}


/*
 * send the username to the server
 */
void sendusername(int sockfd) {
  if (send(sockfd, USERNAME, strlen(USERNAME), 0) == -1)
    perror("send");
}



void usage() {
  fprintf (stderr, "usage: dsh [flags] {-c command}, where flags are:\n");
  fprintf (stderr, "\t{-c command}\tcommand to execute remotely\n");
  fprintf (stderr, "\t{-s host}\tthe host the server is on\n");
  fprintf (stderr, "\t[-p #]\t\tthe port the server is on\n");
  fprintf (stderr, "\t-h\t\tdisplay this help message\n");
  exit(1);
}
