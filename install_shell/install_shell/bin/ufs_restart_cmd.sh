#!/bin/bash
cd /home/dfs/bin


killall -9 uusee_dfs

rm -rf ./log/uusee_dfs*.log
rm -rf ./log/uusee_status.rec

nohup ./uusee_dfs uusee_dfs.conf >/dev/null 2>&1 &
