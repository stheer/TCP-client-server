CC= gcc

all: tcpclient async-tcpserver

tcpclient: tcpclient.c
	$(CC) -g -Wall -o tcpclient tcpclient.c

async-tcpserver: async-tcpserver.c
	$(CC) -g -Wall -o async-tcpserver async-tcpserver.c

clean:
	rm -f *.o tcpclient async-tcpserver
