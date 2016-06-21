#!/bin/sh

if [[ ${MONGODB_PORT} =~ ^tcp://(.*):(.*)+$ ]] ; then
    export MONGODB_PORT=${BASH_REMATCH[2]}
    export MONGODB_HOSTNAME=${BASH_REMATCH[1]}
    
fi

if [[ ${ORION_PORT} =~ ^tcp://(.*):(.*)+$ ]] ; then
    export ORION_PORT=${BASH_REMATCH[2]}
    export ORION_HOSTNAME=${BASH_REMATCH[1]}
fi

export HOST_IP=`awk 'NR==1{print $1}' /etc/hosts`

echo "HOST IP: $HOST_IP"
echo "PROXY : $PUBLIC_PROXY_PORT"
echo "PUBLIC IP : $PUBLIC_IP_PORT"
echo "MONGODB HOST: $MONGODB_HOSTNAME"
echo "MONGODB PORT: $MONGODB_PORT"
echo "ORION CB HOST: $ORION_HOSTNAME"
echo "ORION CB PORT: $ORION_PORT"


sed -i /etc/init.d/mosquitto \
    -e "s|etc/mosquitto/mosquitto.conf|etc/iot/mosquitto.conf|g"

sed -i /etc/iot/mosquitto.conf \
    -e "s|user root|user iotagent|g"

sed -i /etc/iot/config.json \
        -e "s|ORION_HOSTNAME|${ORION_HOSTNAME}|g" \
        -e "s|ORION_PORT|${ORION_PORT}|g" \
        -e "s|MONGODB_HOSTNAME|${MONGODB_HOSTNAME}|g" \
    -e "s|MONGODB_PORT|${MONGODB_PORT}|g" \
    -e "s|PUBLIC_PROXY_PORT|${PUBLIC_PROXY_PORT}|g" \
    -e "s|PUBLIC_IP_PORT|${PUBLIC_IP_PORT}|g" \
    -e "s|KEYSTONE_HOSTNAME|${KEYSTONE_HOSTNAME}|g" \
    -e "s|KEYSTONE_PORT|${KEYSTONE_PORT}|g" \
    -e "s|KEYPASS_HOSTNAME|${KEYPASS_HOSTNAME}|g" \
    -e "s|KEYPASS_PORT|${KEYPASS_PORT}|g" \
    -e "s|PEP_PASSWORD|${PEP_PASSWORD}|g" \
    -e "s|IOTAGENT_PASSWORD|${IOTAGENT_PASSWORD}|g"



sed -i /usr/local/iot/config/iotagent_protocol.conf \
       -e "s|IOTAGENT_HOST|${HOST_IP}|g"

sed -i /usr/local/iot/config/iotagent_manager.conf \
       -e "s|IOTAGENT_HOST|${HOST_IP}|g"


#cat /etc/iot/config.json

exec /sbin/init




#/bin/bash

