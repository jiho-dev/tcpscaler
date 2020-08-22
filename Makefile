CFLAGS = -g -Wall -std=gnu99  #--std=c99

all: tcpclient tcpserver udpclient
	ctags -R

run:
#	./tcpclient -R -p 1000 -P 1020 -c 10 -n 10 -r 1 -vv 10.1.1.2
	./tcpclient -l syslog -R -d -p 1000 -P 1020 -c 50000 -n 1000000 -r 1000 -m 1000 -v 10.1.1.2
#	gdb -ex run --args ./tcpclient -R -p 1000 -c 10 -n 10 -r 10 -vv 10.1.1.2
#	gdb -ex run --args ./tcpclient -R -p 1000 -c 10 -n 5 -r 10 -vv 10.1.1.2

tcpclient.o: tcpclient.c common.h utils.h

udpclient.o: udpclient.c common.h utils.h

tcpserver: tcpserver.o utils.h
	$(CC) -levent -lm -o $@ utils.o poisson.o $<

tcpclient: tcpclient.o poisson.o utils.o
#	$(CC) -levent -levent_openssl -lssl -lm -o $@ poisson.o utils.o $<
	$(CC) -levent  -lm -o $@ poisson.o utils.o $<

udpclient: udpclient.o utils.o
	$(CC) -levent -lm -o $@ utils.o poisson.o $<

clean:
	rm -f *.o tcpserver tcpclient udpclient
