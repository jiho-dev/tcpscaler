#!/bin/bash

port=1000
max_port=1000

#num_conn=64000
num_conn=10
conn_rate=1000
send_rate=1
host="10.1.1.1"

if [ $# -eq 1 ]; then
	port=$1
	max_port=$1
elif [ $# -ge 2 ]; then
	port=$1
	max_port=$2
fi

for ((i=$port;i<=$max_port;i++)); do
	echo "Port: $i"
	logfile="./client_$i.log"

	./tcpclient -l $logfile -R -p $i -c $num_conn -n $conn_rate -r $send_rate $host -vv &
done

