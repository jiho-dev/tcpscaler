#!/bin/bash


sudo killall tcpserver
sudo rm -f ./server.log

#sudo ./tcpserver -p 1000 -P 1199 -m 1000 -vv -l ./server.log &
#sudo ./tcpserver -p 1000 -P 1199 -m 100 -vv 

echo "Start TCPServer"
#sudo ./tcpserver -p 1000 -P 1199 -m 1000 -vv -l ./server.log &
sudo ./tcpserver -d -p 1000 -P 1199 -m 1000 -vv -l syslog 
