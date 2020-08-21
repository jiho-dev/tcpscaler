CFLAGS = -Wall -std=gnu99  #--std=c99

all: tcpclient tcpserver udpclient

tcpclient.o: tcpclient.c common.h utils.h

udpclient.o: udpclient.c common.h utils.h

tcpserver: tcpserver.o utils.h
	$(CC) -levent -o $@ $<

tcpclient: tcpclient.o poisson.o utils.o
#	$(CC) -levent -levent_openssl -lssl -lm -o $@ poisson.o utils.o $<
	$(CC) -levent  -lm -o $@ poisson.o utils.o $<

udpclient: udpclient.o utils.o
	$(CC) -levent -lm -o $@ poisson.o utils.o $<

clean:
	rm -f *.o tcpserver tcpclient udpclient
