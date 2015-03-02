#!/bin/bash

killall -9 uusee_dfs

cd /home/dfs/bin
mkdir log

scp root@10.1.82.217:/data0/wlj/cvs/DistributeFileSystem/uusee_dfs/src/uusee_dfs ./
if [ 0 = $? ] ; then
echo "2 cp success"
else
echo "install uusee_dfs failed"
exit 1
fi

scp root@10.1.82.217:/data0/wlj/cvs/DistributeFileSystem/uusee_dfs/src/install_shell/bin/conf/uusee_dfs.conf ./
if [ 0 = $? ] ; then
echo "2 cp success"
else
echo "install uusee_dfs failed"
exit 1
fi

scp root@10.1.82.217:/data0/wlj/cvs/DistributeFileSystem/uusee_dfs/bin/run.sh ./
if [ 0 = $? ] ; then
echo "2 cp success"
else
echo "install uusee_dfs failed"
exit 1
fi

scp root@10.1.82.217:/data0/wlj/cvs/DistributeFileSystem/uusee_dfs/bin/info.sh ./
if [ 0 = $? ] ; then
echo "2 cp success"
else
echo "install uusee_dfs failed"
exit 1
fi


echo "install uusee_dfs successed"
