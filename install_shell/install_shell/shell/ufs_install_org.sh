
port=8970
ip_list_len=${#ip_list[@]}

echo "the length is ${ip_list_len}"

index=0
while [ $index -lt $ip_list_len ];
do

#print the ip
echo "the ip is ${ip_list[$index]}"

echo "[local]" > uusee_dfs.conf.tmp
echo "ip=${ip_list[$index]}"  >> uusee_dfs.conf.tmp
echo "port=$port"  >> uusee_dfs.conf.tmp
echo "mid=${ip_list[$index]}:$port" >> uusee_dfs.conf.tmp
echo "watchdir=/home/dfs/bin/watchdir/" >> uusee_dfs.conf.tmp
echo "logdir=/home/dfs/bin/log/" >> uusee_dfs.conf.tmp
echo "scanfileperiod=172800" >> uusee_dfs.conf.tmp

echo "[peer1]" >> uusee_dfs.conf.tmp
echo "ip1=${ip_list[0]}" >> uusee_dfs.conf.tmp
echo "port1=$port" >> uusee_dfs.conf.tmp

echo "[peer2]" >> uusee_dfs.conf.tmp
echo "ip2=${ip_list[1]}" >> uusee_dfs.conf.tmp
echo "port2=$port" >> uusee_dfs.conf.tmp

mv uusee_dfs.conf.tmp "./conf/uusee_dfs.conf"
ssh ${ip_list[$index]} < ufs_install_cmd.sh 

index=`expr $index + 1`

done
