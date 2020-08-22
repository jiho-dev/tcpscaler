#!/bin/bash

#scp -F tf-ssh-bastion.conf run_server.sh setup-server.sh tcpserver ubuntu@bastion-01:~/

#client_ip="10.0.5.162 10.0.6.11 10.0.5.31 10.0.6.4 10.0.5.87 10.0.6.207 10.0.5.90 10.0.6.58 10.0.0.3 10.0.5.78"

client_ip1="10.0.5.162 10.0.6.11 10.0.5.31 10.0.6.4 10.0.5.87" 
client_ip2="10.0.6.207 10.0.5.90 10.0.6.58 10.0.0.3 10.0.5.78"

client_ip="$client_ip1 $client_ip2"

run_test() {
	for ip in ${client_ip}; do
		echo "Start tcpclient: $ip"
#		scp -F tf-ssh-bastion.conf run_iperf.sh count_tcp.sh run_client.sh setup-client.sh tcpclient ubuntu@$ip:~/

#		scp -F tf-ssh-bastion.conf run_client.sh ubuntu@$ip:~/
#		scp -F tf-ssh-bastion.conf tcpclient ubuntu@$ip:~/
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo ./setup-client.sh"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "./run_client.sh"
	done

}

stop_test() {
	for ip in ${client_ip}; do
		echo "Stop tcpclient: $ip"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo killall tcpclient"
	done
}

restart_server() {
	ssh -F tf-ssh-bastion.conf ubuntu@bastion-01 "sudo killall tcpserver; ./run_server.sh"
}

reset_ct() {
	ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@172.24.50.18 "sudo conntrack -F"
}

show_syslog() {
	ip1="10.0.5.162 10.0.6.11 10.0.5.31 10.0.6.4 10.0.5.87" 
	ip2="10.0.6.207 10.0.5.90 10.0.6.58 10.0.0.3 10.0.5.78"
	ips="$ip1 $ip2"
	local line=3

	for ip in ${ips}; do
		echo "Show syslog: $ip"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo tail -n $line /var/log/syslog"
	done

}

#######################

$1

