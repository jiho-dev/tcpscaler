#!/bin/bash

set -ex

port=1000
max_port=1020

num_conn=50000
conn_rate=1000000
#send_rate=1000
send_rate=500
per_msg=1000
host="10.0.0.2"


if [ $# -eq 1 ]; then
	port=$1
	max_port=$1
elif [ $# -ge 2 ]; then
	port=$1
fi

run_multi_client() {
	for ((i=$port;i<$max_port;i++)); do
		echo "Port: $i"
		logfile="./client_$i.log"
		sudo rm -f $logfile

		./tcpclient -l $logfile -d -R -p $i -m $per_msg -c $num_conn -n $conn_rate -r $send_rate $host -v 
	done
}

run_single_client() {
	#logfile="./client.log"
	logfile="syslog"
	sudo rm -f $logfile
	sudo killall tcpclient || true

	echo "Start TCPClient"
	./tcpclient -l $logfile -d -R -p $port -P $max_port -c $num_conn -n $conn_rate -r $send_rate -m $per_msg -v $host 
}


run_single_client
