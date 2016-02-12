#include "server.h"


int main(int argc, char* argv[]) {
  getHelp(argc, argv);
  char* port = getPort(argc, argv);
  // check the "dir" arg
  // start the server loop
  runServer(port);
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

  logServerStart(port);

  while(1) {  // main accept() loop
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family,
      get_in_addr((struct sockaddr *)&their_addr),
      s, sizeof s);
    printf("server: got connection from %s\n", s);

    if (!fork()) { // this is the child process
      close(sockfd); // child doesn't need the listener
      // recv username
      recvusername(new_fd);
      // generate a random key
      char* salt = genkey();
      // send key
      sendkey(new_fd, salt);
      // recv password
      recvpw(new_fd, salt);
      logpwok();
      // recv command
      recvcmd(new_fd);

      close(new_fd);
      exit(0);
    }
    close(new_fd);  // parent doesn't need this
  }
}


/*
 * receives the command from the client
 */
void recvcmd(int sockfd) {
  int numbytes;
  char buf[MAXDATASIZE];

  // read the command from the client
  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buf[numbytes] = '\0';

  printf("Running command: %s\n\n", buf);

  dup2(sockfd, STDOUT_FILENO);
  dup2(sockfd, STDERR_FILENO);

  run_command(buf);
  if (send(sockfd, "\n", 1, 0) == -1)
    perror("send");
}


/*
 * receives the password and checks its validity
 */
void recvpw(int sockfd, char* salt) {
  char buf[MAXDATASIZE];
  int numbytes;

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buf[numbytes] = '\0';

  if(strcmp(buf, crypt(PASSWD, salt)) != 0) {
    printf("salt: %s\n", salt);
    printf("buf: %s\ncrypt(PASSWD, salt): %s\n", buf, crypt(PASSWD, salt));
    fprintf(stderr, "Password is not correct\n");
    exit(1);
  }
}

/*
 * logs that the password is ok
 */
void logpwok() {
  printf("Password is ok\n");
}


/*
 * generate a random key
 */
char* genkey() {
  time_t t;
  srand((unsigned) &t);
  char* str = malloc(sizeof(char) * 30);
  sprintf(str, "%d", rand());
  return str;
}


/*
 * send the key to the sockfd
 */
void sendkey(int sockfd, char* salt) {
  // generate random key
  if (send(sockfd, salt, sizeof(salt), 0) == -1)
    perror("send");
}


/*
 * receive the username from the request
 */
void recvusername(int sockfd) {
  char buf[MAXDATASIZE];
  int numbytes;

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buf[numbytes] = '\0';
  logusername(buf);

  if(strcmp(buf, USERNAME) != 0) {
    fprintf(stderr, "Username %s is not registered\n", buf);
    exit(1);
  }
}

/*
 * log the username that was recived
 */
void logusername(char* username) {
  printf("received connection from: %s\n", username);
}


/*
 * logs that the server has started
 */
void logServerStart(char* port) {
  char buff[PATH_MAX + 1];
  getcwd(buff, PATH_MAX + 1);

  printf("Starting server\n");
  printf("\tport: %s\n", port);
  printf("\tdir: %s\n\n", buff);
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

/*
 * run a bash command that was passed in by the user
 */
int run_command(char* command) {
  int pid = fork();

  if(pid < 0) {
    perror("run_command: fork() error");
    exit(1);
  } else if (pid == 0) {
    // we're the child
    exit(system(command));
  } else {
    // we're the parent
    int status;
    struct timeval before;
    struct timeval after;
    gettimeofday(&before, NULL);
    waitpid(pid, &status, 0);
    gettimeofday(&after, NULL);
    return status;
  }
  return 0;
}


void usage() {
  fprintf (stderr, "usage: server [flags], where flags are:\n");
  fprintf (stderr, "\t-p #\t\tthe port to server from (default is 80085)\n");
  fprintf (stderr, "\t-d dir\t\tthe dir to serve files from (default is /home/ubuntu/dsh)\n");
  fprintf (stderr, "\t-h\t\tdisplay this help message\n");
  exit(1);
}
