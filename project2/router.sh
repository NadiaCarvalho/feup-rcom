#!/bin/bash

command_router () {
	echo $1 > /dev/ttyS0	
	#cat < /dev/ttyS0
}

getopts n nat 

hostname=$(hostname | tr -d 'tux') 
stand=$(echo $hostname | head -c 1)

command_router "configure terminal"
command_router "interface gigabitethernet 0/0"
command_router "ip address 172.16.${stand}1.254 255.255.255.0"
command_router "no shutdown"

if [ "$nat" != "?" ]
then
	command_router "ip nat inside"
fi

command_router "exit"

command_router "interface gigabitethernet 0/1"
command_router "ip address 172.16.1.${stand}9 255.255.255.0"
command_router "no shutdown"

if [ "$nat" != "?" ]
then
	command_router "ip nat outside"
fi

command_router "exit"

if [ "$nat" != "?" ]
then
command_router "ip nat pool ovrld 172.16.1.${stand}9 172.16.1.${stand}9 prefix 24"
command_router "ip nat inside source list 1 pool ovrld overload"

command_router "access-list 1 permit 172.16.${stand}0.0 0.0.0.7"
command_router "access-list 1 permit 172.16.${stand}1.0 0.0.0.7"
fi

command_router "ip route 0.0.0.0 0.0.0.0 172.16.1.254"
command_router "ip route 172.16.${stand}0.0 255.255.255.0 172.16.${stand}1.253"
command_router "end"

command_router "show running-config"
