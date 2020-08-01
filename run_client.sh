#!/bin/bash

port=1000

if [ $# -ge 1 ]; then
	port=$1
fi

#num_conn=64000
num_conn=10
conn_rate=1000
send_rate=1
host="10.1.1.1"

./tcpclient -R -p $port -c $num_conn -n $conn_rate -r $send_rate $host -vv

