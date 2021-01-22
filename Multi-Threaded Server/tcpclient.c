#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#define MAX_LINE 25

int main (int argc, char *argv[]) {
  char* host_addr = argv[1];
  int port = atoi(argv[2]);
  int num = atoi(argv[3]);

  /* Open a socket */
  int s;
  if((s = socket(PF_INET, SOCK_STREAM, 0)) <0){
    perror("simplex-talk: socket");
    exit(1);
  }

  /* Config the server address */
  struct sockaddr_in sin;
  sin.sin_family = AF_INET; 
  sin.sin_addr.s_addr = inet_addr(host_addr);
  sin.sin_port = htons(port);
  // Set all bits of the padding field to 0
  memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

  /* Connect to the server */
  if(connect(s, (struct sockaddr *)&sin, sizeof(sin))<0){
    perror("simplex-talk: connect");
    close(s);
    exit(1);
  }

  /*main loop: get and send lines of text */
  char message[MAX_LINE] = "HELLO ";
  char buf[MAX_LINE];
  int last = num;
  char number[10];
  sprintf(number, "%d", num);
  strcat(message, number);
  send(s, message, strlen(message)+1, 0);
  memset(message, 0, sizeof(message));
  memset(number, 0, sizeof(number));

  while(read(s, buf, sizeof(buf)) > 0){ 
    printf("%s\n", buf);
    
    memcpy(message, &buf[0], 6);
    memcpy(number, &buf[6], strlen(buf) - 5);
    
    if(atoi(number) - last != 1){
        printf("%s\n", "ERROR");
        send(s, '\0', 1, 0);
        close(s);
        break;
    }
    last = atoi(number) + 1;
    sprintf(number, "%d", last);
    strcat(message, number);

    send(s, message, strlen(message) + 1, 0);
    
    memset(message, 0, sizeof(message));
    memset(buf, 0, sizeof(buf));
    memset(number, 0, sizeof(number));

    if(last - num == 2){
        close(s);
        break;
    }
  }
  /*send(s, buf, strlen("hey") + 1, 0);
  while(read(s, buf, sizeof(buf)) > 0) {
    fputs(buf, stdout);
  }*/
  //send(s, "hey", strlen("hey") + 1, 0);
  //send(s, buf, strlen(buf) + 1, 0);
  return 0;
}
