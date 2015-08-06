#!/bin/sh

if [[ ${MONGODB_PORT} =~ ^tcp://(.*):(.*)+$ ]] ; then
    export MONGODB_PORT=${BASH_REMATCH[2]}
    export MONGODB_HOSTNAME=${BASH_REMATCH[1]}
    
fi

if [[ ${ORION_PORT} =~ ^tcp://(.*):(.*)+$ ]] ; then
    export ORION_PORT=${BASH_REMATCH[2]}
    export ORION_HOSTNAME=${BASH_REMATCH[1]}
fi

echo $PUBLIC_PROXY_PORT
echo $PUBLIC_IP_PORT
echo $MONGODB_HOSTNAME
echo $MONGODB_PORT
echo $ORION_HOSTNAME
echo $ORION_PORT


sed -i /etc/iot/config.json \
	-e "s|ORION_HOSTNAME|${ORION_HOSTNAME}|g" \
	-e "s|ORION_PORT|${ORION_PORT}|g" \
	-e "s|MONGODB_HOSTNAME|${MONGODB_HOSTNAME}|g" \
    -e "s|MONGODB_PORT|${MONGODB_PORT}|g" \
    -e "s|PUBLIC_PROXY_PORT|${PUBLIC_PROXY_PORT}|g" \
    -e "s|PUBLIC_IP_PORT|${PUBLIC_IP_PORT}|g"

#cat /etc/iot/config.json

exec /sbin/init

#/bin/bash

