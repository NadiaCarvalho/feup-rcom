#!/bin/bash

expNo=$1
hostname=$(hostname | tr -d 'tux') 
stand=$(echo $hostname | head -c 1)
tuxno=$(echo $hostname | tail -c 2)

if [ "$expNo" = "" ]; then
	expNo=7
fi

## Reset network service
/etc/init.d/networking restart

## eth0 is used in all tuxes
ifconfig eth0 down
ifconfig eth0 up 

## Shut eth1 down to avoid messing with previous experiments.
ifconfig eth1 down

if [ "$tuxno" -eq "1" ]
then
	ifconfig eth0 172.16.${stand}0.1/24
	
	## Add routing via tux4 for other packets 
	if [ "$expNo" -ge "3" ]; then
		route add -net default gw 172.16.${stand}0.254
	fi	
elif [ "$tuxno" -eq "2" ] && [ "$expNo" -ge "2" ]
then
	ifconfig eth0 172.16.${stand}1.1/24

	## Add routing via tux4 for vlan 0 packets
	if [ "$expNo" -ge "3" ]; then
		route add -net 172.16.${stand}0.0/24 gw 172.16.${stand}1.253
	fi

	## Add routing via commercial router for other packets
	if [ "$expNo" -ge "4" ]; then
		route add -net default gw 172.16.${stand}1.254
	fi	
elif [ "$tuxno" -eq "4" ]
then
	ifconfig eth0 172.16.${stand}0.254/24

	## Delete the default route added to the lab network
	route del -net 172.16.1.0/24 

	if [ "$expNo" -ge "3" ]; then	
		ifconfig eth1 up 
		ifconfig eth1 172.16.${stand}1.253/24
	fi
	
	if [ "$expNo" -ge "4" ]; then
		route add -net default gw 172.16.${stand}1.254
	fi
fi

echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

## Adds DNS
if [ "$expNo" -ge "5" ]; then
	printf "search lixa.fe.up.pt\nnameserver 172.16.1.1" > /etc/resolv.conf
fi

ifconfig
route -n
