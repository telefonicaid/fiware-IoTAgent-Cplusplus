#Introduction
Dockerfile to create a container with IoTAgent (MQTT and UL20). Using Docker-compose (iota.yaml fie) you could run multiple containers with IoTAgent, Orion Context-Broker and MongoDB Database all interconnected.

#Use

Run this command:
```
docker-compose -f iota.yaml up -d iotacpp
```

#Build just IoTAgent docker image
```
docker build .
```

This will create an image of the IoTAgent, but in order to properly start it, you need to run other containers for ContextBroker and MongoDB, hence the convenience of having a docker-compose file that will start up all three elements together. 
