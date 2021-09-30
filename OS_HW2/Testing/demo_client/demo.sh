#!/bin/bash

SERVER_IP=$1
SERVER_PORT=$2
SESSION="oshw2_demo"

if [ -z ${SERVER_IP} ] || [ -z ${SERVER_PORT} ]; then
  echo "Usage: $0 <server ip> <server port>"
  exit 1
fi

if [ -n "`tmux ls | grep ${SESSION}`" ]; then
  tmux kill-session -t $SESSION
fi

tmux new-session -s ${SESSION} "./demo_tmux.sh $1 $2"


