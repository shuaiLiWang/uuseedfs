#!/bin/bash
ip_list=( 
10.1.102.160
10.1.102.161
10.1.102.162
10.1.102.163
 )

ip_list_len=${#ip_list[@]}
echo "the length is ${ip_list_len}"

index=0
while [ $index -lt $ip_list_len ];
do

#print the ip
echo "ufs_start shell is running the current ip is ${ip_list[$index]}"

ssh ${ip_list[$index]} < ufs_start_cmd.sh

index=`expr $index + 1`

done
