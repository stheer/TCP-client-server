#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

/* This is a reference socket server implementation that prints out the messages
 * received from clients. */

#define MAX_PENDING 100
#define MAX_LINE 25

int handle_first_shake(int fd, int* state_array_fd, int* state_array_val, socklen_t len);
int handle_second_shake(int index, int* state_array_fd, int* state_array_val, socklen_t len);

int main(int argc, char* argv[])
{
    int port = atoi(argv[1]);
    fd_set master;
    fd_set readfds;
    int i, j, h, state_index, s, new_s, fdmax, fd;
    int client_socket[100];
    int state_array_fd[100];
    int state_array_val[100];
    memset(client_socket, 0, sizeof(client_socket));
    memset(state_array_fd, 0, sizeof(state_array_fd));
    memset(state_array_val, 0, sizeof(state_array_val));

    /*setup passive open*/
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("cannot create socket");
        exit(1);
    }

    FD_ZERO(&master);

    /* Config the server address */
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin.sin_port = htons(port);
    // Set all bits of the padding field to 0
    memset(sin.sin_zero, '\0', sizeof(sin.sin_zero));

    /* Bind the socket to the address */
    if ((bind(s, (struct sockaddr*)&sin, sizeof(sin))) < 0) {
        perror("cannot bind socket to localhost");
        exit(1);
    }
    FD_SET(s, &master);

    // connections can be pending if many concurrent client requests
    listen(s, MAX_PENDING);

    /* wait for connection, then receive and print text */
    socklen_t len = sizeof(sin);
    fdmax = s;

    while (1) {
        FD_ZERO(&readfds);
        readfds = master;
        int res_select = select(fdmax + 1, &readfds, NULL, NULL, NULL);
        if(res_select < 0){
            perror("select");
            exit(1);
        }else{
                if(FD_ISSET(s, &readfds)) {
                    if ((new_s = accept(s, (struct sockaddr*)&sin, &len)) < 0) {
                        perror("simplex-talk: accept");
                        exit(1);
                    } else {
                        fcntl(new_s, F_SETFL, O_NONBLOCK);
                        FD_SET(new_s, &master);
                        if (new_s > fdmax) {
                            fdmax = new_s;
                        }
                        for (h = 0; h < MAX_PENDING; h++){    
                            if(client_socket[h] == 0)   
                            {   
                                client_socket[h] = new_s; 
                                h = MAX_PENDING; 
                            }   
                        }
                    }
                }else{ 
                    for(i = 0; i < MAX_PENDING; i++){
                        fd = client_socket[i];
                        if(FD_ISSET(fd, &readfds)) {
                            state_index = -1;
                            for (j = 0; j < sizeof(state_array_fd) / sizeof(int); j++) {
                                if (state_array_fd[j] == fd) {
                                    state_index = j;
                                }
                            }
                            if (state_index == -1) {
                                if (handle_first_shake(fd, state_array_fd, state_array_val, len) < 0) {
                                    FD_CLR(fd, &master);
                                }
                            } else {
                                handle_second_shake(state_index, state_array_fd, state_array_val, len);
                                FD_CLR(state_array_fd[state_index], &master);
                            } 
                        }
                    }
                }
            }              
        
    }
    close(s);
    return 0;
}

int handle_first_shake(int fd, int* state_array_fd, int* state_array_val, socklen_t len)
{
    int num = 0;
    char buf[MAX_LINE];
    char message[MAX_LINE];
    char number[10];
    int i = 0;

    if((len = recv(fd, buf, sizeof(buf), 0))){
        printf("%s\n", buf);
        memcpy(message, &buf[0], 6);
        memcpy(number, &buf[6], strlen(buf) - 5);

        while (state_array_fd[i] != 0) {
            i++;
        }

        num = atoi(number);
        state_array_fd[i] = fd;
        state_array_val[i] = num;
        sprintf(number, "%d", num + 1);
        strcat(message, number);
        send(fd, message, strlen(message) + 1, 0);
        memset(message, 0, sizeof(message));
        memset(number, 0, sizeof(number));
        memset(buf, 0, sizeof(buf));
    }
    return(0);
}

int handle_second_shake(int index, int* state_array_fd, int* state_array_val, socklen_t len)
{
    int num = 0;
    char buf[MAX_LINE];
    char message[MAX_LINE];
    char number[10];

    if((len = recv(state_array_fd[index], buf, sizeof(buf), 0))){
        printf("%s\n", buf);
        memcpy(message, &buf[0], 6);
        memcpy(number, &buf[6], strlen(buf) - 5);
        num = atoi(number);

        if(state_array_val[index] + 2 != num){
            printf("%s\n", "ERROR");
            close(state_array_fd[index]);
            memset(message, 0, sizeof(message));
            memset(number, 0, sizeof(number));
            memset(buf, 0, sizeof(buf));
        }

        int arrayLength = sizeof(state_array_fd) / sizeof(int);
        int j = 0;
        if(index < arrayLength){
            for (j=index; j < arrayLength; j++){
                state_array_fd[j] = state_array_fd[j+1];
                state_array_val[j] = state_array_val[j+1];
            }
        }

        memset(message, 0, sizeof(message));
        memset(number, 0, sizeof(number));
        memset(buf, 0, sizeof(buf));
    }
    return (0);
}
