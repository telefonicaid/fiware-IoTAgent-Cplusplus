# Simple Modbus  


#### Index
1. [Introduction](#def-introduction)


<a name="def-introduction"></a>
## 1. Introduction
**IoT Agent for Modbus implements master side in Modbus protocol, but it is the Modbus slave who establishes the connection with master (IoT Agent). That is a workaround when slaves use dynamic IP address. Only FC03 y FC06 are available.**
Communication process in this implemetation is:
1- Slave open a TCP connection and IoT Agent can accept or reject.
2- If connection is accepted, IoT Agent begin to send Modbus frame in order to get information from slave.
3- Slave answers.
4- IoT Agent process answers.
5- When IoT Agent has all information, close connection. 

What the IoTAgent for Modbus is going to do is get information from Modbus Devices and publish those in NGSI-compatible format on the ContextBroker. This document is aimed at developers wanting to use Modbus devices within the IoT Platform realm, as it describes the particularities of the IoTAgent for Modbus including the entities format that are published on ContextBroker. 

Prior any Modbus device can connect to the IoTAgent, it has to be correctly provisioned through the IoTAgent API. And in order to do that, you have to be given some credentials (the trust token) as operations with ContextBroker are authenticated. All your devices will be associated to a sub-service so that only you have access to them. That sub-service is also part of a greater service on ContextBroker, so that is one of the pieces of information you need to provide in order to provisioning your devices. 
The service is represented by this header:

	Fiware-Service

And the sub-service by this one:

	Fiware-ServicePath

Please refer to this section to find out more about how to work with services: [IoTAgent API: provision of Services](north_api.md#def-service-api)
''Note'': The API exposed by the IoTAgent is somewhat generic and used with other protocols different to Modbus, that's why some of the fields don't apply to Modbus. For instance, 'apikey' has to be left empty as the services hierarchy is provided by service/sub-service relationship only. 

Every Modbus device defines what a memory position means. In order to publish, you have several options:
1- IoT Agent use memory position number to name an attribute.
2- In configuration file, a position has a name.
3- Custom attributes per device, then, next thing to do is provisioning the device (this assumes you already have provisioned a sub-service). Please refer to [IoTAgent API: provision of devices](north_api.md#def-device-api) for more specific information on how devices API works.

