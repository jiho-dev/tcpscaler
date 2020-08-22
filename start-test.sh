#!/bin/bash

#scp -F tf-ssh-bastion.conf run_server.sh setup-server.sh tcpserver ubuntu@bastion-01:~/

client_ip="10.0.5.162 10.0.6.11" 

for ip in ${client_ip}; do
	echo "Uplaod file: $ip"
#	scp -F tf-ssh-bastion.conf run_iperf.sh count_tcp.sh run_client.sh setup-client.sh tcpclient ubuntu@$ip:~/
#	scp -F tf-ssh-bastion.conf run_client.sh ubuntu@$ip:~/
	ssh -F tf-ssh-bastion.conf ubuntu@$ip "./run_client.sh"
done

