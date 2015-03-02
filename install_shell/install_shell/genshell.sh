#!/bin/bash

# remove all old shell scripts
rm -rf ./bin/*
mkdir -p bin/conf

#generate install shell script
echo "#!/bin/bash" > bin/ufs_install.sh
echo "ip_list=( " >> bin/ufs_install.sh
cat iplist.txt >> bin/ufs_install.sh
echo " )" >> bin/ufs_install.sh
cat shell/ufs_install_org.sh >> bin/ufs_install.sh
chmod a+x bin/ufs_install.sh
cp ./shell/ufs_install_cmd.sh ./bin

#generate restart shell script
echo "#!/bin/bash" > bin/ufs_restart.sh
echo "ip_list=( " >> bin/ufs_restart.sh
cat iplist.txt >> bin/ufs_restart.sh
echo " )" >> bin/ufs_restart.sh
cat shell/ufs_restart_org.sh >> bin/ufs_restart.sh
chmod a+x bin/ufs_restart.sh
cp ./shell/ufs_restart_cmd.sh ./bin

#generate start shell script
echo "#!/bin/bash" > bin/ufs_start.sh
echo "ip_list=( " >> bin/ufs_start.sh
cat iplist.txt >> bin/ufs_start.sh
echo " )" >> bin/ufs_start.sh
cat shell/ufs_start_org.sh >> bin/ufs_start.sh
chmod a+x bin/ufs_start.sh
cp ./shell/ufs_start_cmd.sh ./bin


#generate stop shell script
echo "#!/bin/bash" > bin/ufs_stop.sh
echo "ip_list=( " >> bin/ufs_stop.sh
cat iplist.txt >> bin/ufs_stop.sh
echo " )" >> bin/ufs_stop.sh
cat shell/ufs_stop_org.sh >> bin/ufs_stop.sh
chmod a+x bin/ufs_stop.sh
cp ./shell/ufs_stop_cmd.sh ./bin


