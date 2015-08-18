#Introduction
Dockerfile to create a container with IoTAgent (MQTT and UL20). Using Docker-compose (iota.yaml fie) you could run multiple containers with IoTAgent, Orion Context-Broker and MongoDB Database all interconnected.

#Use

Run this command:
```
docker-compose -f iota.yaml up -d iotacpp
```
This command will build the IoTAgent docker image and it will run three different containers with Orion ContextBroker, MongoDB and IoTAgent all of them linked. The IoTAgent container will expose the 8080 (HTTP) port and 1883 for MQTT protocol. 

#Build just IoTAgent docker image
```
docker build .
```

This will create an image of the IoTAgent, but in order to properly start it, you need to run other containers for ContextBroker and MongoDB, hence the convenience of having a docker-compose file that will start up all three elements together. 
