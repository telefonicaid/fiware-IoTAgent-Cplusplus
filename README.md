# IoT Agents
## Index

* [Overview](#overview)

* [Architecture](doc/architecture.md)
* Installation and Administration Guide
    - [Build and Install](doc/build.md), including how to make your own module-based iotagent.
    - [Deployment](doc/deploy.md)
    - [Sanity check procedures](doc/sanity_check.md)
    - [Diagnosis Procedures](doc/diagnosis_procedures.md)
    - [Authentication, Authorization and PEP Rules](doc/pep.md)
    - [Configuration API](doc/north_api.md)
    
* User & Programmers Manual
    - Introduction. This manual describes how to use the IoT Agents hosted in this repository.
    - User Manual
        * [Commands in IoT Agent](doc/commands.md)
        * [Southbound APIs](doc/modules.md)

## <a name="overview">Overview</a>
This project aims to provide a C++ platform to enable IoT Agent developers to build custom agents for their devices that can easily connect to NGSI Context Brokers (such as Orion ).

An IoT Agent is a component that lets groups of devices send their data to and be managed from a FIWARE NGSI Context Broker using their own native protocols. IoT Agents should also be able to deal with security aspects of the fiware platform (authentication and authorization of the channel) and provide other common services to the device programmer.

Each device will be mapped as an Entity following some rules: the user could provide an entity name and entity type for a device, a default entity type could be provided to use as entity type over all devices associated with an agent, by default entity name follows `<device_id>:<entity_type>` format.

Each of the measures obtained from the device will be mapped to a different attribute. The name and type of the attribute will be configured by the user. Device measures can have two different behaviors:

-     Active attributes: are measures that are pushed from the device to the IoT agent. This measure changes will be sent to the Context Broker as updateContext requests over the device entity.
-     Lazy attributes: some sensors will be passive, and will wait for the IoT Agent to request for data. For those measures, the IoT Agent will register itself in the Context Broker as a Context Provider (for all the lazy measures of that device), so if any component asks the Context Broker for the value of that sensor, its request will be redirected to the IoT Agent. Updates over this measure will be transformed into commands over the device by the IoT Agent.

A device could work as actuator executing commands. Commands are modelled as lazy attributes.

These are the features an IoT Agent is supposed to expose:

-     Device registration: multiple devices will be connected to each IoT Agent, each one of those mapped to a CB entity. The IoT Agent will register itself as a Context Provider for each device when it provides commands or lazy attributes.
-     Device information update: whenever a device have new measures to publish, it should send the information to the IoT Agent in its own native language. This message should, in turn, be should sent as an updateContext request to the Context Broker, where the measures will be updated in the device entity.
-     Device command execution and value updates: as a Context Provider, the IoT Agent should receive update operations from the Context Broker, and relay them to the corresponding device (decoding it using its ID and Type, and other possible metadata). This commands will arrive as updateContext operations from the Context Broker to the IoT Agent.
-     Device management: the IoT Agent should offer a device repository where the devices can be registered, holding data needed for the connection to the Context Broker as the following: service and subservice for the device, API Key the device will be using to connect to the IoT Agent, Trust token the device will be using to retrieve the Keystone token to connect to the Context Broker.
-    Device provisioning: the IoT Agent should offer an external API to make a preprovision of any devices. This preprovision should enable the user to customize the device`s entity name and type as well as their service information.
-     Type configuration: if a device is registered without a preregistration, only its id and type attributes are mandatory. The IoT Agent should provide a mechanism to provide default values to the device attributes based on its type.
-     PEP: provision request are authenticated and authorized against Keystsone system in order to validate this request.









## License ##
IoT Agent software is licensed under Affero General Public License (GPL) version 3.
