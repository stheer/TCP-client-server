#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 10
#define MAX_LINE 25

int main(int argc, char *argv[]) {
  int port = atoi(argv[1]);

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
  char message[MAX_LINE];
  message[0] = '\0';
  char buf[MAX_LINE];
  int num;
  int i = 0;
  char number[10];
  number[0] = '\0';
  while(1) {
    if((new_s = accept(s, (struct sockaddr *)&sin, &len)) <0){
      perror("simplex-talk: accept");
      exit(1);
    }
    while((len = recv(new_s, buf, sizeof(buf), 0))){
        printf("%s\n", buf);
        
        memcpy(message, &buf[0], 6);
        memcpy(number, &buf[6], strlen(buf) - 5);
        
        i = i + 1;
        if(i != 1){
          if(atoi(number) - num != 1){
            printf("%s\n", "ERROR");
            close(new_s);
            memset(message, 0, sizeof(message));
            message[0] = '\0';
            memset(number, 0, sizeof(number));
            number[0] = '\0';
            memset(buf, 0, sizeof(buf));
            i = 0;
            break;
          }
        }
        
        if(i == 2){
            close(new_s);
            memset(message, 0, sizeof(message));
            message[0] = '\0';
            memset(buf, 0, sizeof(buf));
            memset(number, 0, sizeof(number));
            number[0] = '\0';
            i = 0;
            break;
        }
        num = atoi(number) + 1;
        sprintf(number, "%d", num);
        strcat(message, number);

        send(new_s, message, strlen(message) + 1, 0);
        memset(message, 0, sizeof(message));
        message[0] = '\0';
        memset(buf, 0, sizeof(buf));
        memset(number, 0, sizeof(number));
        number[0] = '\0';
    }
  }

  return 0;
}
