#!/bin/sh
# Should be run as root

#sysctl net.ipv4.ip_local_port_range="1024 65535"
sysctl net.ipv4.ip_local_port_range="1000 65000"
sysctl net.ipv4.tcp_tw_reuse=1

#sysctl net.ipv4.tcp_syncookies=0
sysctl net.core.somaxconn=8192
sysctl fs.file-max=12582912

#vi /etc/security/limits.conf
#jiho.jung soft nofile 1048576
#jiho.jung hard nofile 1048576
# all user
#* soft nofile 1048576
#* hard nofile 1048576


#iptables -t raw -A PREROUTING -p tcp -j NOTRACK
#iptables -t raw -A OUTPUT -p tcp -j NOTRACK

# for ubuntu
#sudo apt-get install libevent-openssl-2.0-5
#sudo apt-get install -y libssl-dev


