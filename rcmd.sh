#!/bin/bash


server_host_ip="172.24.50.21"
client_host_ip="172.24.50.19 172.24.50.20 172.24.52.20 172.24.52.21"
host_ip="$server_host_ip $client_host_ip"

client_ip="10.0.5.63 10.0.5.59 10.0.5.151 10.0.5.42 10.0.0.10 10.0.5.69 10.0.6.138 10.0.6.62 10.0.6.222 10.0.6.196"

run_test() {
	for ip in ${client_ip}; do
		echo "Start tcpclient: $ip"
#		scp -F tf-ssh-bastion.conf run_iperf.sh count_tcp.sh run_client.sh setup-client.sh tcpclient ubuntu@$ip:~/
#		scp -F tf-ssh-bastion.conf run_client.sh ubuntu@$ip:~/
#		scp -F tf-ssh-bastion.conf tcpclient ubuntu@$ip:~/
#		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo ./setup-client.sh"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "./run_client.sh"
	done

}

setup_client_vm() {
	for ip in ${client_ip}; do
		echo "Setup tcpclient: $ip"
		scp -F tf-ssh-bastion.conf setup-client.sh ubuntu@$ip:~/
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo ./setup-client.sh"
	done

}

stop_test() {
	for ip in ${client_ip}; do
		echo "Stop tcpclient: $ip"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo killall tcpclient"
	done
}

restart_server() {
	ssh -F tf-ssh-bastion.conf ubuntu@bastion-01 "sudo killall tcpserver; sudo ./setup-server.sh; ./run_server.sh"
}

reset_ct() {
	for ip in ${host_ip}; do
		echo "Show host max ct: $ip"
		ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@$ip "sudo conntrack -F"
	done
}

show_max_ct() {
	for ip in ${host_ip}; do
		echo "Show host max ct: $ip"
		ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@$ip "cat /proc/sys/net/netfilter/nf_conntrack_max"
	done
}

set_max_ct() {
	for ip in ${host_ip}; do
		echo "Set host max ct: $ip"
		ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@$ip "sudo bash -c 'echo 20000000 > /proc/sys/net/netfilter/nf_conntrack_max'"
	done
}

show_ct() {
	for ip in ${host_ip}; do
		echo "Show host currnet ct: $ip"
		ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@$ip "cat /proc/sys/net/netfilter/nf_conntrack_count"
	done
}

show_syslog() {
	ips="$client_ip"
	local line=3

	for ip in ${ips}; do
		echo "Show syslog: $ip"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo tail -n $line /var/log/syslog"
	done

}

show_free() {
	local i=0

	for ip in ${client_ip}; do
		i=$((i+1))
		echo "** Show free on client $i : $ip **"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "free"
	done
}

show_host_free() {
	local i=0

	for ip in ${host_ip}; do
		i=$((i+1))
		echo "** Show free on client $i : $ip **"
		ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@$ip "free"
	done
}

show_top() {
	local i=0

	for ip in ${client_ip}; do
		i=$((i+1))
		echo "** Show top on client $i : $ip **"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "top -b -n 1 | head"
	done
}

show_tcp() {
	local i=0

	for ip in ${client_ip}; do
		i=$((i+1))
		echo "** Show TCP conn on client $i : $ip **"
		ssh -F tf-ssh-bastion.conf ubuntu@$ip "netstat -an | grep ESTABLISHED | wc -l"
	done
}

install_iperf() {
	local ips="$client_ip bastion-01"

	for ip in ${ips}; do
		echo "install iperf "

		ssh -F tf-ssh-bastion.conf ubuntu@$ip "sudo apt-get --yes install iperf"
	done
}

show_iperf() {
	local ips=$client_ip
	local i=0

	iperf_cmd="iperf -c 10.0.0.4  -t 30 -P 16"
	echo "***** iperf cmd on every host: $iperf_cmd *****"
	echo

	for ip in ${ips}; do
		i=$((i+1))
		echo -n "Run client $i iperf: $ip - "

		ret=$(ssh -F tf-ssh-bastion.conf ubuntu@$ip "$iperf_cmd | grep SUM")
		perf=$(echo $ret | awk '{ print $6 }')
		echo "$perf Gbps"
	done
}

install_perf() {
	ssh -F ~/ssh-keys/lab-eu-ssh.conf centos@172.24.50.21 "sudo yum -y install linux-tools-common perf linux-tools-generic"
}

#######################

$1

