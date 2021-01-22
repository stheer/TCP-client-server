#If no arguments are passed to make, it will attempt the 'client' and 'server' targets
default: tcpclient multi-tcpserver

#This creates all binaries
all: tcpclient multi-tcpserver

#Will construct the client and server binaries
tcpclient: tcpclient.c
	gcc -o tcpclient tcpclient.c -l pthread

multi-tcpserver: multi-tcpserver.c
	gcc -o multi-tcpserver multi-tcpserver.c -l pthread

clean:
	rm -f *.o tcpclient multi-tcpserver
