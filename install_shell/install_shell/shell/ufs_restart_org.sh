
ip_list_len=${#ip_list[@]}
echo "the length is ${ip_list_len}"

index=0
while [ $index -lt $ip_list_len ];
do

#print the ip
echo "ufs_restart shell is running the current ip is ${ip_list[$index]}"

ssh ${ip_list[$index]} < ufs_restart_cmd.sh

index=`expr $index + 1`

done
