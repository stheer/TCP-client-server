#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 10
#define MAX_LINE 25

struct thread_params {
    int s;
    socklen_t len;
    pthread_t thread;
};

void *tcpCommunicate(void *args);

int main(int argc, char *argv[]) {
  int port = atoi(argv[1]);
  struct thread_params *args;

  /*setup passive open*/
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0) {
    perror("cannot create socket");
    exit(1);
  }

  /* Config the server address */
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr("127.0.0.1");
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Bind the socket to the address */
  if((bind(s, (struct sockaddr*)&sin, sizeof(sin)))<0) {
    perror("cannot bind socket to localhost");
    exit(1);
  }

  // connections can be pending if many concurrent client requests
  listen(s, MAX_PENDING);  

  /* wait for connection, then receive and print text */
  int new_s;
  socklen_t len = sizeof(sin);
  
  while(1) {
    if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      exit(1);
    }


    args = malloc(sizeof(struct thread_params));
    if(args == NULL){
        perror("Error in thread malloc");
        exit(EXIT_FAILURE);
    }
    args->s = new_s;
    args->len = len;
    pthread_create(&args->thread, NULL, tcpCommunicate, args);
    pthread_join(*(&args->thread), NULL);
  }

  close(s);
  return 0;
}


void *tcpCommunicate(void *args){
  struct thread_params *thread_args = args;
  int len = thread_args->len;
  socklen_t s = thread_args->s;

  char message[MAX_LINE];
  char buf[MAX_LINE];
  int num;
  int i = 0;
  char number[10];

  while((len = recv(s, buf, sizeof(buf), 0))){
    printf("%s\n", buf);
        
    memcpy(message, &buf[0], 6);
    memcpy(number, &buf[6], strlen(buf) - 5);
    
    i = i + 1;
    if(i != 1){
      if(atoi(number) - num != 1){
          printf("%s\n", "ERROR");
          close(s);
          free(thread_args);
          memset(message, 0, sizeof(message));
          memset(number, 0, sizeof(number));
          memset(buf, 0, sizeof(buf));
          i = 0;
          break;
      }
    }

    if(i == 2){
        close(s);
        free(thread_args);
        memset(message, 0, sizeof(message));
        memset(buf, 0, sizeof(buf));
        memset(number, 0, sizeof(number));
        i = 0;
        break;
    }
    num = atoi(number) + 1;
    sprintf(number, "%d", num);
    strcat(message, number);

    send(s, message, strlen(message) + 1, 0);
    memset(message, 0, sizeof(message));
    memset(buf, 0, sizeof(buf));
    memset(number, 0, sizeof(number));
  }

  pthread_exit(NULL);
}
