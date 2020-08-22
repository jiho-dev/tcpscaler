#!/bin/bash

# for server
scp -F tf-ssh-bastion.conf run_server.sh setup-server.sh tcpserver ubuntu@bastion-01:~/

# for client
client_ip="10.0.5.162 10.0.6.11 10.0.5.31 10.0.6.4 10.0.5.87 10.0.6.207 10.0.5.90 10.0.6.58 10.0.0.3 10.0.5.78"

for ip in ${client_ip}; do
	echo "Uplaod file: $ip"
	scp -F tf-ssh-bastion.conf run_iperf.sh count_tcp.sh run_client.sh setup-client.sh tcpclient ubuntu@$ip:~/
#	scp -F tf-ssh-bastion.conf run_client.sh ubuntu@$ip:~/
done

