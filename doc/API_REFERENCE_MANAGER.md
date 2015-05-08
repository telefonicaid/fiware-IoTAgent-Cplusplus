FORMAT: 1A

# IoT Manager API Documentacion
The IoT Manager API is based on REST principles. This documentation covers the resources you can manipulate on IoT Manager in order to manage  information registered by IoT Agents.

NOTE: This document is a **work in progress**.

**Allowed HTTPs requests:**

- POST: Creates a resource or list of resources.
- PUT: Updates a resource.
- GET: Retrieves a resource or list of resources.
- DELETE: Delete a resource.

**Typical Server Responses:**

- 200 OK: The request was succesful (some API calls may return 201 instead).
- 201 Created: The request was succesful and a resource or list of resources was created.
- 204 No Content: The request was succesful but there is no representation to return (that is, the response is empty).
- 400 Bad Request: The request could not be understood or was missing required parameters.
- 401 Unauthorized: Authentication failed.
- 403 Forbidden: Access denied.
- 404 Not Found: Resource was not found.
- 409 Conflict: A resource cannot be created because it already exists.
- 500 Internal Server Error: Generic error when server has a malfunction. This error will be removed.

Responses related with authentication and authorization depends on this feature is configured and a Keystone OpenStack sytem is present.

When an error is returned, a representation is returned as:

```
{
  "reason": "contains why an error is returned",
  "details": "contains specific information about the error, if possible"
}
```

## Authentication and Authorization
If IoT Agent is in authenticated environment, this API requires a token, which you obtain from authentication system. This system and its API is out of scope of present documentation. In this environment, a mandatory header is needed: `X-Auth-Token`.

## Mandatory HTTP Headers
Anyway, this API needs two headers in order to manage requests:
- `Fiware-Service` : Represents a tenant, the higher level in resources hierachy in IoT Platform environment. If this header is not present an error is returned.
- `Fiware-ServicePath` : Represents the second level. Its value must be start by character '/'. If this header is not present we assume '/'. There are operations that is allowed '/*'.

## Naming conventions
- `Fiware-Service`: a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercases.
- `Fiware-ServicePath`: a service path string must only contain underscores and alphanumeric characters and starts with character /. Maximum length is 51 characters (with /).

## API Access
All URIs are relative to an specific url, where IoT Agent is raised. For example, `http://127.0.0.1:8080/iot/`.

## Protocols [/protocols{?limit,offset}]
Protocols are the rules that define the communication with different devices. It depends on the deployed iotagents, a list will be displayed with the ID and description of the protocols.

### Protocol Model
Fields in JSON object representing a protocol are:
- `_id` (string, mandatory). It is a internal identifier.
- `protocol` (string, mandatory). It is a name or identifier provided by IoT Agent.
- `description`(string, mandatory). It is a description about protocol provided by IoT Agent.
- `enpoints` (array, mandatory). It is an array with information about where this protocol is deployed. An endpoint is defined by `endpoint` (including ip address and port) and `resource` (including path or uri where protocol is deployed into IoT Agent).

### Retrieve a protocol [GET]

It retrieves all protocols.

+ Parameters [limit, offset]

    + `limit` (optional, number). In order to specify the maximum number of services (default is 20, maximun allowed is 1000).

    + `offset` (optional, number). In order to skip a given number of elements at the beginning (default is 0) .


+ Request (application/json)


+ Response 200

    + Body

            {
              "count": 1,
              "protocols": [
                {
                    "id": "apikey3",
                    "protocol": "service2",
                    "description": "/srvpath2",
                }
              ]
            }

## Services [/services{?limit,offset,protocol}]
Services are the higher level in IoT Platform. When you manipulate a service, you use a Fiware-Service header with its name. Parameters apply to different operations.

### Service Model
Fields in JSON object representing a service are:
- `protocol` (string, mandatory). It is the unique identifier of the protocol.
- `description` (string, mandatory). It is the description of the protocol.
- `apikey` (string, mandatory). It is a key used for devices belonging to this service. If "", service does not use apikey, but it must be specified.
- `token` (string, optional). If authentication/authorization system is configured, IoT Agent works as user when it publishes information. That token allows that other components to verify the identity of IoT Agent. Depends on authentication and authorization system.
- `cbroker`(string, optional). Context Broker endpoint assigned to this service, it must be a real uri.
- `outgoing_route`(string, optional). It is an identifier for VPN/GRE tunnel. It is used when device is into a VPN and a command is sent.
- `resource` (string, mandatory). Path in IoTAgent. When protocol is HTTP a device could send information to this uri. In general, it is a uri in a HTTP server needed to load and execute a module.
- `entity_type` (string, optional). Entity type used in entity publication (overload default).
- `attributes` (optional, array). Mapping for protocol parameters to entity attributes.
`object_id` (string, mandatory): protocol parameter to be mapped.
`name` (string, mandatory): attribute name to publish.
`type`: (string, mandatory): attribute type to publish.
- `static_attributes` (optional, array). Attributes published as defined.
`name` (string, mandatory): attribute name to publish.
`type` (string, mandatory): attribute type to publish.
`value` (string, mandatory): attribute value to publish.

`static_attributes` and `attributes` are used if device has not this information.

### Retrieve a service [GET]

With Fiware-ServicePath you can retrieve a subservice or all subservices.

+ Parameters [limit, offset, protocol]

    + `limit` (optional, number). In order to specify the maximum number of services (default is 20, maximun allowed is 1000).

    + `offset` (optional, number). In order to skip a given number of elements at the beginning (default is 0) .

    + `protocol` (optional, string). Get a service using a specific protocol.

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /*

+ Response 200

    + Body

            {
              "count": 1,
              "services": [
                {
                    "protocol": "55261958d31fc2151cc44c70",
                    "description": "protocol description",
                    "apikey": "apikey3",
                    "service": "service2",
                    "service_path": "/srvpath2",
                    "token": "token2",
                    "cbroker": "http://127.0.0.1:1026",
                    "entity_type": "thing",
                    "resource": "/iot/d"
                }
              ]
            }

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

+ Response 200

    + Body

            {
              "count": 1,
              "services": [
                {
                    "apikey": "apikey3",
                    "service": "service2",
                    "service_path": "/srvpath2",
                    "token": "token2",
                    "cbroker": "http://127.0.0.1:1026",
                    "entity_type": "thing",
                    "resource": "/iot/d"
                }
              ]
            }


### Create a service [POST]
With one subservice defined in Fiware-ServicePath header.

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

    + Body

            {
              "services": [
                {
                  "protocol": ["55261958d31fc2151cc44c70", "55261958d31fc2151cc44c74"],
                  "apikey": "apikey3",
                  "token": "token2",
                  "cbroker": "http://127.0.0.1:1026",
                  "entity_type": "thing"
                }
               ]
             }

+ Response 201

### Modify a service [PUT]
With one subservice defined in Fiware-ServicePath header.

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

    + Body

            {
              "services": [
                {
                  "protocol": ["55261958d31fc2151cc44c70", "1234"],
                  "apikey": "apikey3",
                  "token": "token2",
                  "cbroker": "http://127.0.0.1:1026",
                  "entity_type": "thing"
                }
               ]
             }

+ Response 201


## Devices [/devices{?protocol,limit,offset,detailed}]
A device is a resource that publish information to IoT Platform and it uses the IoT Agent.
### Device Model
- `protocol` (string, mandatory). Unique identifier for the protocol of the device. Available protocols could be retrieved from IoTA Manager.
- `device_id` (string, mandatory). Unique identifier into a service.
- `entity_name` (string, optional). Entity name used for entity publication (overload default)
- `entity_type` (string, optional). Entity type used for entity publication (overload entity_type defined in service).
- `timezone` (optional, string). Not used in this version.
- `attributes` (optional, array). Mapping for protocol parameters to entity attributes.
`object_id` (string, mandatory): protocol parameter to be mapped.
`name` (string, mandatory): attribute name to publish.
`type`: (string, mandatory): attribute type to publish.
- `static_attributes` (optional, array). Attributes published as defined.
`name` (string, mandatory): attribute name to publish.
`type` (string, mandatory): attribute type to publish.
`value` (string, mandatory): attribute value to publish.
- `endpoint` (optional, string): when a device uses push commands.
- `commands` (optional, array). Attributes working as commands.
`name` (string, mandatory): command identifier.
`type` (string, mandatory). It must be 'command'.
`value` (string, mandatory): command representation depends on protocol.


### Create a device [POST]

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

    + Body

            {
                "devices": [
                {
                  "protocol": "55261958d31fc2151cc44c70",
                  "device_id": "device_id",
                  "entity_name": "entity_name",
                  "entity_type": "entity_type",
                  "timezone": "America/Santiago",
                  "attributes": [
                    {
                      "object_id": "source_data",
                      "name": "attr_name",
                      "type": "int"
                    }
                  ],
                  "static_attributes": [
                    {
                      "name": "att_name",
                      "type": "string",
                      "value": "value"
                    }
                  ]
                }
                ]
            }


+ Response 201


+ Response 400

            {
                "reason": "parameter limit must be an integer"
            }

+ Response 404

### Retrieve all devices [GET]

+ Parameters [protocol, limit, offset, detailed]


    + `protocol` (optional, string). It allows get a device using a specific protocol.
    + `limit` (optional, number). In order to specify the maximum number of devices (default is 20, maximum allowed is 1000).
    + `offset` (optional, number). In order to skip a given number of elements at the beginning (default is 0).
    + `detailed` (optional, string). _on_ returns all device information, _off_ returns only name.

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

+ Response 200

    + Body

            {
              "count": 1,
              "devices": [
                {
                  "protocol": "55261958d31fc2151cc44c70",
                  "device_id": "device_id",
                  "entity_name": "entity_name",
                  "entity_type": "entity_type",
                  "timezone": "America/Santiago",
                  "attributes": [
                    {
                      "object_id": "source_data",
                      "name": "attr_name",
                      "type": "int"
                    }
                  ],
                  "static_attributes": [
                    {
                      "name": "att_name",
                      "type": "string",
                      "value": "value"
                    }
                  ]
                }
              ]
            }


## Device [/devices/{device_id}]

### Retrieve a device [GET]
If there are more than one IoT Agent (that is different endpoint), device name could be repeated.

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

+ Response 200

    + Body

            {
              "count": 1,
              "devices": [
                {
                  "protocol": "55261958d31fc2151cc44c70",
                  "device_id": "device_id",
                  "entity_name": "entity_name",
                  "entity_type": "entity_type",
                  "timezone": "America/Santiago",
                  "attributes": [
                    {
                      "object_id": "source_data",
                      "name": "attr_name",
                      "type": "int"
                    }
                  ],
                  "static_attributes": [
                    {
                      "name": "att_name",
                      "type": "string",
                      "value": "value"
                    }
                  ]
                }
              ]
            }


### Update a device [PUT]
If you want modify only a field, you can do it, except if field is `protocol`.

+ Parameters [protocol]

    + `protocol` (mandatory, string). Modify a devices with a specific protocol.

+ Request (application/json)

    + Headers

            Fiware-Service: TestService
            Fiware-ServicePath: /TestSubservice

    + Body

            {
               "entity_name": "entity_name"
            }


+ Response 204





