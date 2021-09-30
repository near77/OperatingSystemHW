#!/bin/bash

USER_NAME=$1
DES_IP=$2
KEY_PATH=$3


if [ -z ${USER_NAME} ] || [ -z ${DES_IP} ] || [ -z ${KEY_PATH} ]; then
  echo "Usage: $0 <User name of destination instance> <destination public ip> <destination pubkey path>"
  exit 1
fi

scp -i ${KEY_PATH} -r answer accuracy.sh ${USER_NAME}@${DES_IP}:~/
ssh -i ${KEY_PATH} ${USER_NAME}@${DES_IP}