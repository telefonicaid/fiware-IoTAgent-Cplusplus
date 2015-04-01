# Architecture
* [Components](#Components)
* [Data flow](#Dataflow)
* [High availability](#HighAvailability)

These IoT Agents are based on Pion Network Library (http://pion.sourceforge.ne/).

## Components

![IoT Agent architecture](imgs/architecture.png) 

- IoTAgent-Core: it could be seen like an C++ platform to build IoT Agents. It provides a HTTP server with common functionalities (provision and configuration, PEP, utilities to publish and future common features). In this server, you can configure URLs. Each url is associated with a protocol and its functionality is implemented as a module or plugin. You can see [Features](#../README.md) in order to learn what this component provides.
- Plugins or Modules:  Functionally, a module is an IoT Agent because it understands a protocol and it builds Orion Context Broker's entity with protocol information.
- MongoDB: provisioned information is stored in Mongo database. In [Deploy](deploy.md) section you can see how MongoDB is configured. Collections used by IoTAgent are (a field is explain in API section).

  + SERVICE: stores service configuration ({`apikey`, `cbroker`, `entity_type`, `resource`, `service`, `service_path`}). This collection is indexed by service, service_path and resource.
  + DEVICE: stores device configuration ({`device_id`, `entity_type`, `entity_name`, `timezone`, `attributes`, `commands`, `service`, `service_path`}). This collection is indexed by device_id, service and service_path.

IoT Agent based on PION loads modules as HTTP plugins associated with an URL. This URL (_resource_ field in configuration) identifies a point of entry into HTTP server and could be the point of entry for devices using HTTP protocol.

When devices use a protocol no HTTP, a module associated with an URL into HTTP server implements that protocol and defines its own points of entry for device communications. Module establishes the relationship:
```Device Point of Entry``` <-> ```PION Module``` <-> ```URL into HTTP server```.


## Definitions
###### Service
A service is a concept like tenant. It is the most high level in hierarchy of resources. Every request has a Fiware-Service header indicating the service.
###### Subservice
A service could have subservices (next level). Every request has a Fiware-ServicePath indicatin the subservice. If a request affects to level service, Fiware--ServicePath is considered "/".

## Data Flow
Flows may be classified in provision, publication and commands.
### Provision Flow
1. An user can request add a service or device. In request, Fiware-Service and Fiware-ServicePath identify the request target. If PEP is not configured, the next action is (6).
2. If PEP is configured, the header X-Auth-Token is validated against Identity Manager.
3. Identity Manager returns a role for user. (If user token is not valid or user has not a role for subservice then request is unauthenticated)
4. If PEP is configured and user has a role, agent request to Access Control authorization for that request.
5. Access Control authorizes or unauthorizes the request.
6. Data (service or device) is stored in database.

### Publication Flow
This flow has a _m_ subscript in image.

1. A device send data. For that, it uses the url identifying its protocol.
2. Agent checks if device has provisioned information. This information takes precedence over default information.
3. As user _iotagent_, agent request a valid token to Identity Manager (it uses the token configured by service/subservice in order to send this request).
4. Identity Manager returns a token. This token will be used if Orion Context Broker uses a PEP.
5. Agent sends _updateContext_ (publish) to Orion Context Broker.

### Command Flow
This flow has a _c_ subscript and flows to get a _iotagent_ token is repeated.  Additional information in [Commands](commands.md).

1. Only provisioned devices with commands are considered. A command is published like an entity attribute and IoT Agent is its provider.
2. IoT Agent send _registerContext_.
3. A command is _updateContext_ operation. This operation has entity and attribute (command to execute).
4. Command is stored in database (a command could be an asynchronous operation).
5. Agent sends command to device.


## High Availability 
If high availability is required, architecture should be redundant and several iotagents  will be running.  Then should exits a load balancer to manage communications between several iotagent and contextBroker  processes.

Load balancer url must be defined in a field "public_ip" in "ngsi_url"  tag of configuration file (config.json). This url will be used to build "providingApplication" which will be sent in context registrations to contextBroker.


