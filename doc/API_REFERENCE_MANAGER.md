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

Responses related with authentication and authorization depends on this feature is configured and a Keystone OpenStack system is present.

Manager could manage more than one agent and errors depends on when this error is detected.
When an error is detected before manager communicates with agent a representation is returned as:

```
{
  "reason": "contains why an error is returned",
  "details": "contains specific information about the error, if possible"
}
```

When manager requests every agent for information, errors are a field in response. GET operation (for devices) returns 200 OK.
When manager receives from agent a successfully response, global response has a succesfully status code. If DELETE/PUT operations this response would be 204 (if no errors) or 200 (if errors). POST operation returns 500 when all agents are failed. An example:
```
{ "count" : 1, "devices" : [{ "protocol" : "UL20", "device_id" : "device_id", "entity_name" : "entity_name", "entity_type" : "entity_type", "endpoint" : "ht     p://device_endpoint", "timezone" : "America/Santiago", "commands" : [ { "name" : "ping", "type" : "command", "value" : "device_i     d@ping|%s" } ], "attributes" : [ { "object_id" : "temp", "name" : "temperature", "type" : "int" } ], "static_attributes" : [ { "     name" : "humidity", "type" : "int", "value" : "50" } ] }] ,"errors": [{"endpoint": "http://127.0.0.1:1000/iot","code": "-1","details": "Connection refused"}]}
```

For more detailed information about what particular error messages are returned per operation, check this [document](errors.md)


## Authentication and Authorization
If IoT Agent is in authenticated environment, this API requires a token, which you obtain from authentication system. This system and its API is out of scope of present documentation. In this environment, a mandatory header is needed: `X-Auth-Token`.

## Mandatory HTTP Headers
Anyway, this API needs two headers in order to manage requests:
- `Fiware-Service` : Represents a tenant, the higher level in resources hierachy in IoT Platform environment. If this header is not present an error is returned.
- `Fiware-ServicePath` : Represents the second level. Its value must be start by character '/'. If this header is not present we assume '/'. There are operations that is allowed '/*'.

## Naming conventions
- `Fiware-Service`: a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercases.
- `Fiware-ServicePath`: a service path string must only contain underscores and alphanumeric characters and starts with character /. Maximum length is 51 characters (with /).

## URL Encode/Decode characters
URL encoding stands for encoding certain characters in a URL by replacing them with one or more character triplets that consist of the percent character "%" followed by two hexadecimal digits. The two hexadecimal digits of the triplet(s) represent the numeric value of the replaced character.
Iot Agent API accepts encoded uri to understand reserved characters in names, except the translation of space for +, to avoid misunderstanding.

## API Access
All URIs are relative to an specific url, where IoT Agent is raised. For example, `http://127.0.0.1:8080/iot/`.

## Protocols [/protocols{?limit,offset}]
Protocols are the rules that define the communication with different devices. It depends on the deployed iotagents, a list will be displayed with the ID and description of the protocols.

### Protocol Model
Fields in JSON object representing a protocol are:
- `_id`. It is a internal identifier.
- `protocol`. It is a name or identifier provided by IoT Agent.
- `description`. It is a description about protocol provided by IoT Agent.
- `enpoints`. It is an array with information about where this protocol is deployed. An endpoint is defined by `endpoint` (including ip address and port) and `resource` (including path or uri where protocol is deployed into IoT Agent).

All fields are mandatory.

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
                  "protocol" : "PDI-IoTA-UltraLight",
                  "description" : "UL2",
                  "endpoints" : [
                     { "endpoint" : "http://127.0.0.1:8080/iot",
                       "identifier" : "idcl1:8080",
                       "resource" : "/iot/d"
                     }
                   ]
                }
               ]
            }


## Services [/services{?limit,offset,protocol,apikey}]
Services are the higher level in IoT Platform. When you manipulate a service, you use a Fiware-Service header with its name. Parameters apply to different operations.

### Service Model
Fields in JSON object representing a service are:
- `protocol`. It is the unique identifier of the protocol.
- `description`. It is the description of the protocol.
- `apikey`. It is a key used for devices belonging to this service. If "", service does not use apikey, but it must be specified.
- `token`. If authentication/authorization system is configured, IoT Agent works as user when it publishes information. That token allows that other components to verify the identity of IoT Agent. Depends on authentication and authorization system.
- `cbroker`. Context Broker endpoint assigned to this service, it must be a real uri.
- `outgoing_route`. It is an identifier for VPN/GRE tunnel. It is used when device is into a VPN and a command is sent.
- `resource`. Path in IoTAgent. When protocol is HTTP a device could send information to this uri. In general, it is a uri in a HTTP server needed to load and execute a module.
- `entity_type`. Entity type used in entity publication (overload default).
- `attributes`. Mapping for protocol parameters to entity attributes.
`object_id` (string, mandatory): protocol parameter to be mapped.
`name` (string, mandatory): attribute name to publish.
`type`: (string, mandatory): attribute type to publish.
- `static_attributes`. Attributes published as defined.
`name` (string, mandatory): attribute name to publish.
`type` (string, mandatory): attribute type to publish.
`value` (string, mandatory): attribute value to publish.

`static_attributes` and `attributes` are used if device has not this information.

Mandatory fields are identified in every operation.

### Retrieve a service [GET]

With Fiware-ServicePath you can retrieve a subservice or all subservices.

+ Parameters [limit, offset, protocol]

    + `limit` (optional, number). In order to specify the maximum number of services (default is 20, maximun allowed is 1000).

    + `offset` (optional, number). In order to skip a given number of elements at the beginning (default is 0) .

    + `protocol` (optional, string). Get a service using a specific protocol.

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
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

            Fiware-Service: testservice
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
With one subservice defined in Fiware-ServicePath header. From service model, mandatory fields are:  protocol, apikey.

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
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
With one subservice defined in Fiware-ServicePath header. In order to modify _apikey_ you can define the apikey as parameter in query to identify, if needed,
the service (in body you set the new apikey).

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
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

+ Response 204


## Devices [/devices{?protocol,limit,offset,detailed}]
A device is a resource that publish information to IoT Platform and it uses the IoT Agent.
### Device Model
- `protocol`. Unique identifier for the protocol of the device. Available protocols could be retrieved from IoTA Manager.
- `device_id`. Unique identifier into a service.
- `entity_name`. Entity name used for entity publication (overload default)
- `entity_type`. Entity type used for entity publication (overload entity_type defined in service).
- `timezone`. Not used in this version.
- `attributes`. Mapping for protocol parameters to entity attributes.
`object_id` (string, mandatory): protocol parameter to be mapped.
`name` (string, mandatory): attribute name to publish.
`type`: (string, mandatory): attribute type to publish.
- `static_attributes`. Attributes published as defined.
`name` (string, mandatory): attribute name to publish.
`type` (string, mandatory): attribute type to publish.
`value` (string, mandatory): attribute value to publish.
- `endpoint`. when a device uses push commands.
- `commands`. Attributes working as commands.
`name` (string, mandatory): command identifier.
`type` (string, mandatory). It must be 'command'.
`value` (string, mandatory): command representation depends on protocol.


### Create a device [POST]
From device model, mandatory fields are:  device_id and protocol.

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
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

            Fiware-Service: testservice
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

            Fiware-Service: testservice
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

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

    + Body

            {
               "entity_name": "entity_name"
            }


+ Response 204





