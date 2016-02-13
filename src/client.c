#include "client.h"


int main(int argc, char* argv[]) {
  introduceSelf();
  while(1) {
    char* query = getMovieQuery();
    // broadcast query over multicase
    char* servInfo = queryServer(query);
    // if there was no server, skip this iteration
    if(servInfo == NULL) {
      noResultsFound();
      continue;
    }
    // contact and connect to the server
    watchMovie(servInfo);
  }
}


void introduceSelf() {
  printf("Client started\n");
}


void noResultsFound() {
  printf("404: movie not found\n");
}


// Read input from the user to see what movie they're looking for
// return char*; a movie name to look for
char* getMovieQuery() {
  char* query = (char *)malloc(sizeof(char) * 255);
  memset(query, 0, sizeof(char)*255);

  printf("What movie are you looking for? ");
  fgets(query, 254, stdin);
  return query;
}


// sends a query for a movie to a server
// returns the ip:port as a string or NULL if no server is found
char* queryServer(char* query) {
  int sock;

  // create the socket to send the query string
  if ((sock=msockcreate(SEND, QUERY_ADDR, QUERY_PORT)) < 0) {
    perror("msockcreate");
    exit(1);
  }

  printf("Querying server: %s\n", query);

  // send the query to the known address
  if (msend(sock, query, strlen(query)+1) < 0) {
    perror("msend");
    exit(1);
  }

  // we're done with the query
  free(query);

  // create a timeout struct for the response
  struct timeval timeout;
  timeout.tv_sec = 4;
  timeout.tv_usec = 0;

  /* set up socket to receive the query response */
  if ((sock=msockcreate(RECV, RESPONSE_ADDR, RESPONSE_PORT)) < 0) {
    perror("msockcreate");
    exit(1);
  }

  // set a timeout for the socket
  if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    perror("setsockopt failed\n");
    exit(1);
  }

  // create memory for the message
  char* message = malloc(sizeof(char) * MAXDATASIZE);
  memset(message, 0, sizeof(char) * MAXDATASIZE);

  // receive the message
  int cnt = mrecv(sock, message, MAXDATASIZE-1);

  // close the socket connection
  if (msockdestroy(sock) < 0) {
    perror("msockdestroy");
    exit(1);
  }

  // check for errors
  if (cnt < 0 && errno != 0) {
    return NULL;
  }

  // if no message was received
  if(strcmp(message, "") == 0) {
    return NULL;
  }

  return message;
}


// watch the movie
void watchMovie(char* servInfo) {
  clearScreen();
  return;
}


// print the magical thing that clears the console
void clearScreen() {
  printf("\033[2J");
  printf("\033[0;0f");
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
