#include "server.h"


int main(int argc, char* argv[]) {
  getHelp(argc, argv);
  char* tcpport = getPort(argc, argv);
  // be polite
  introduceSelf();

  // fork for the sake of running a long
  // running process on two different ports
  pid_t pid;
  pid = fork();

  if(pid == 0) {
    // wait for a query to come in
    recvQuery(tcpport);
  } else {
    // wait for a connection for the movie they asked for
    // then send it
    waitForMovieRequest(tcpport);
  }
}


void waitForMovieRequest(char* port) {
  runServer(port);
}


char* searchForMovie(char* movieTitle) {
  // create space for the path
  int bytesize = strlen(MOVIE_DIR) + strlen(movieTitle) + 4;
  char* moviePath = (char *) malloc(sizeof(char) * bytesize);
  memset(moviePath, 0, sizeof(char) * bytesize);
  // concatenate "movies/" to the path
  strcat(moviePath, MOVIE_DIR);
  // concatenate the movie title to the path but remove
  // the newline at the end of the string
  strncat(moviePath, movieTitle, strlen(movieTitle)-1);
  // concatenate the movie file extension to the path
  strcat(moviePath, ".txt");
  // see if the file exists
  if(access(moviePath, F_OK) != -1) {
    return moviePath;
  } else {
    return NULL;
  }
}


void sendServInfo(char* tcpport) {
  int fd;
  struct ifreq ifr;

  fd = socket(AF_INET, SOCK_DGRAM, 0);

  /* I want to get an IPv4 IP address */
  ifr.ifr_addr.sa_family = AF_INET;

  /* I want IP address attached to "eth0" */
  strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

  ioctl(fd, SIOCGIFADDR, &ifr);

  close(fd);

  // send server info to client
  int sock;
  if ((sock=msockcreate(SEND, RESPONSE_ADDR, RESPONSE_PORT)) < 0) {
    perror("msockcreate");
    exit(1);
  }


  // make space for the string
  char* servinfo = (char*) malloc(sizeof(char) * (strlen(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr)) + strlen(tcpport) + 1));

  strcat(servinfo, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
  strcat(servinfo, ":");
  strcat(servinfo, tcpport);

  printf("Movie found, sending IP:port to client: %s\n", servinfo);

  // send the query to the known address
  if (msend(sock, servinfo, strlen(servinfo)+1) < 0) {
    perror("msend");
    exit(1);
  }
}


void movieNotFound() {
  printf("Movie not found, not sending response\n");
}


void introduceSelf() {
  printf("Server started.\n");
  printf("Listening...\n");
}


// receive a query for a movie over the network
void recvQuery(char* tcpport) {
  int sock;

  while(1) {
    // create the socket to send the query string
    if ((sock=msockcreate(RECV, QUERY_ADDR, QUERY_PORT)) < 0) {
      perror("msockcreate");
      exit(1);
    }

    // create memory for the message
    char* query = malloc(sizeof(char) * 255);
    memset(query, 0, sizeof(char) * 255);

    // receive the message
    int cnt = mrecv(sock, query, 255-1);

    // close the socket connection
    if (msockdestroy(sock) < 0) {
      perror("msockdestroy");
      exit(1);
    }

    // check for errors
    if (cnt < 0 && errno != 0) {
      perror("mrecv");
    }

    // search for the movie
    char* moviePath = searchForMovie(query);

    // if we have the movie
    if(moviePath != NULL) {
      // send our IP and port
      sendServInfo(tcpport);
    } else {
      // log that the movie was not found
      movieNotFound();
    }
  }
}


void getHelp(int argc, char* argv[]) {
  for(int i=0; i < argc; i++) {
    if(strcmp(argv[i], "-h") == 0) {
       usage();
    }
  }
}


/*
 * start the server on the given port
 * taken from Beej's guide
 */
void runServer(char* port) {
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }

    break;
  }

  freeaddrinfo(servinfo); // all done with this structure

  if (p == NULL)  {
    fprintf(stderr, "server: failed to bind\n");
    exit(1);
  }

  if (listen(sockfd, 100) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  while(1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
    }

    inet_ntop(their_addr.ss_family,
      get_in_addr((struct sockaddr *)&their_addr),
      s, sizeof s);
    printf("server: got connection from %s\n", s);

    if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener
      // recv command
      char* mvpath = recvtitle(new_fd);
      printf("got the path: %s\n", mvpath);
      char* fps = recvfps(new_fd);
      printf("got the fps: %s\n", fps);
      sendframes(new_fd, mvpath, fps);

      printf("then it exited from sending frames\n");
      close(new_fd);
      exit(0);
    }
    close(new_fd);  // parent doesn't need this
  }
}


// send the movie frames to the client via the sockfd
// at the specified fps
void sendframes(int sockfd, char* mvpath, char* fps) {
  int ifps = atoi(fps);
  int sleeputime = ((float)1000000/((float)ifps*(float)1000000))*1000000;
  FILE* fp = fopen(mvpath, "r");

  if(fp == NULL) {
    perror("sendframes: fopen");
    exit(1);
  }

  char frame[MAXDATASIZE];
  char line[255];
  memset(frame, 0, sizeof(char) * MAXDATASIZE);
  memset(line, 0, sizeof(char) * 255);


  // while we haven't reached the end of the file
  while(!feof(fp)) {
    // copy the line
    if(fgets(line, 255, fp) == NULL) {
      break;
    }

    // while we haven't read "end\n"
    while(strcmp(line, "end\n") != 0) {
      // concat the line to the frame
      strcat(frame, line);
      // reset the line
      memset(line, 0, sizeof(char) * 255);
      if(feof(fp))
        break;
      // get the next line
      if(fgets(line, 255, fp) == NULL) {
        break;
      }
    }

    // send the frame
    if (send(sockfd, frame, sizeof(frame), 0) == -1) {
      perror("send");
      break;
    }
    memset(frame, 0, sizeof(char) * MAXDATASIZE);
    memset(line, 0, sizeof(char) * 255);

    usleep(sleeputime);
  }

  // tell the client we're done
  send(sockfd, "done", sizeof("done"), 0);

  fclose(fp);
}


/*
 * returns the path of the movie from the client's request
 */
char* recvtitle(int sockfd) {
  int numbytes;
  char buf[MAXDATASIZE];
  memset(buf, 0, sizeof(char) * MAXDATASIZE);

  // read the command from the client
  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buf[numbytes] = '\0';


  int bytesize = strlen(MOVIE_DIR) + strlen(buf) + 4;
  char* moviePath = (char *) malloc(sizeof(char) * bytesize);
  memset(moviePath, 0, sizeof(char) * bytesize);
  // concatenate "movies/" to the path
  strcat(moviePath, MOVIE_DIR);
  // concatenate the movie title to the path but remove
  // the newline at the end of the string
  strncat(moviePath, buf, strlen(buf)-1);
  // concatenate the movie file extension to the path
  strcat(moviePath, ".txt");

  return moviePath;
}


// receive the fps from the client
char* recvfps(int sockfd) {
  int numbytes;
  char *fps = (char*) malloc(sizeof(char)*2);

  if ((numbytes = recv(sockfd, fps, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  return fps;
}


/*
 * handle a child signal
 */
void sigchld_handler(int s) {
  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  while(waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
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


void usage() {
  fprintf (stderr, "usage: server [flags], where flags are:\n");
  fprintf (stderr, "\t-p #\t\tthe port to server from (default is 8081)\n");
  fprintf (stderr, "\t-h\t\tdisplay this help message\n");
  exit(1);
}
