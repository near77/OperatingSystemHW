#!/bin/bash

USER_NAME=$1
CLIENT_IP=$2
KEY_PATH=$3


if [ -z ${USER_NAME} ] || [ -z ${CLIENT_IP} ] || [ -z ${KEY_PATH} ]; then
  echo "Usage: $0 <User name of client instance> <client public ip> <client pubkey path>"
  exit 1
fi

scp -i ${KEY_PATH} -r demo_client ${USER_NAME}@${CLIENT_IP}:~/
ssh -i ${KEY_PATH} ${USER_NAME}@${CLIENT_IP}