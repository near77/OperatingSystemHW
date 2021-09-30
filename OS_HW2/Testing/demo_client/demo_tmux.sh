#!/bin/bash

SERVER_IP=$1
SERVER_PORT=$2
SESSION="oshw2_demo"
PYTHON=$( which python3 || which python )

if [ -z ${SERVER_IP} ] || [ -z ${SERVER_PORT} ]; then
  echo "Usage: $0 <server ip> <server port>"
  exit 1
fi

tmux set remain-on-exit on

tmux split-window -v -p 100
tmux split-window -h -p 80
tmux split-window -h -p 74
tmux split-window -h -p 65
tmux split-window -h -p 50

tmux send-keys -t 1 "clear" Enter
tmux send-keys -t 1 "${PYTHON} stream.py ${SERVER_IP} ${SERVER_PORT} 01.mp4" Enter
tmux send-keys -t 2 "clear" Enter
tmux send-keys -t 2 "${PYTHON} stream.py ${SERVER_IP} ${SERVER_PORT} 02.mp4" Enter
tmux send-keys -t 3 "clear" Enter
tmux send-keys -t 3 "${PYTHON} stream.py ${SERVER_IP} ${SERVER_PORT} 03.mp4" Enter
tmux send-keys -t 4 "clear" Enter
tmux send-keys -t 4 "${PYTHON} stream.py ${SERVER_IP} ${SERVER_PORT} 04.mp4" Enter
tmux send-keys -t 5 "clear" Enter
tmux send-keys -t 5 "${PYTHON} stream.py ${SERVER_IP} ${SERVER_PORT} 05.mp4" Enter

tmux kill-pane -t 0



