#!/bin/sh

killall -9 uusee_dfs

rm -rf ./log/uusee_dfs*.log
rm -rf ./log/uusee_status.rec

./uusee_dfs uusee_dfs.conf&
./info.sh
echo "uusee_dfs is running."
