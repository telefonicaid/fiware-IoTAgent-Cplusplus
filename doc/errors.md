FORMAT: 1A

# Errors returned by IoTAgent and IotAgent-Manager when using their REST API.

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

### Create a service POST (IotAgent API)
With one subservice defined in Fiware-ServicePath header. From service model, mandatory fields are: apikey, resource (cbroker field is temporary mandatory).

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

    + Body

            {
              "services": [
                {
                  "apikey": "apikey3",
                  "token": "token2",
                  "cbroker": "http://127.0.0.1:1026",
                  "entity_type": "thing",
                  "resource": "/iot/d"
                }
               ]
             }


+ SUCCESS Response 201

#### Error messages, examples:

+ Response 400:

Missing property within  JSON:
```
{"reason":"The request is not well formed","details":"Missing required property: apikey [/services[0]]"}
```

Service or sub-service headers not valid (as per Context Broker constraints):
```
 {"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercase"}
```
```
{"reason": "Malformed header","details": "Fiware-ServicePath not accepted - a service path string must only contain underscores and alphanumeric characters and starts with character /"}
```
Missing service header.
```
 {"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercase"}
```

Wrong type for any of the fields (like using integer instead of string):
```
{"reason":"The request is not well formed","details":"invalid data type: UnsignedType expected: string [/services[0]/token]"}
```
Not valid URL for ContextBroker:
```
{"reason":"A parameter of the request is invalid/not allowed[http10.95.213.36:1026]","details":"http10.95.213.36:1026 is not a correct uri"}
```
Service contains badly formed attibutes:
```
{"reason":"The request is not well formed","details":"Missing required property: object_id [/services[0]/attributes[0]]"}
```
JSON is invalid or not well formed:
```
 {"reason":"The request is not well formed","details":"JSONParser: An error occurred on line 1 column 18"}
```
JSON contains unexpected fields:
```
 {"reason":"The request is not well formed","details":"Additional properties not allowed. [/services[0]/extra_data]"}#code:400#
```

+ Response 409

Duplicated service:
```
 {"reason":"There are conflicts, object already exists","details":"duplicate key: iotest.SERVICE { apikey: apikeymqtt, token: tokenmqtt, cbroker: http://10.95.213.36:1026, entity_type: thingmqtt, resource: /iot/mqtt, service: serv22, service_path: /srf }"}
```

### Create a service POST (IotManager API)

From service model, mandatory fields are:  protocol, apikey.

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

#### Error messages:

+ Response 400:

Most "bad request" errors returned by IotAgent-Manager follow the same pattern as explained for IoTAgent. This messages are returned by the IoTAgent Manager and it means that no actual REST operation 

- Missing mandatory property. 

```
{"reason":"The request is not well formed","details":"Missing required property: protocol [/services[0]]"}
```

- Wrong type for a field:

```
{"reason":"The request is not well formed","details":"invalid data type: StringType expected: array [/services[0]/protocol]"}
```
- Extra information (note that 'resource' is not required for the operation at the IoTManager)

```
{"reason":"The request is not well formed","details":"Additional properties not allowed. [/services[0]/resource]"}
```

- No existing protocol
```
{"reason":"The request is not well formed","details":"No exists protocol 55b0aca019bab552820d971b"}
```

+ Response 500:

- Whenever there are several endpoints (iotagents) where the operation will be send to, the errors  trying to add duplicate service, in this scenario, there are two endpoints (iotagents) so the error message will contain two individual responses coming from the endpoints:
```
{"errors": [{"endpoint": "http://0.0.0.0:8080/iot","code": "409","details": {"reason":"There are conflicts, object already exists","details":"duplicate key: iotest.SERVICE { apikey: apikeymqtt, token: tokenmqtt, cbroker: http://10.95.213.36:1026, entity_type: thingmqtt, resource: /iot/mqtt, service: service1, service_path: /sserv1 }"}},{"endpoint": "http://0.0.0.0:8082/iot","code": "409","details": {"reason":"There are conflicts, object already exists","details":"duplicate key: iotest.SERVICE { apikey: apikeymqtt, token: tokenmqtt, cbroker: http://10.95.213.36:1026, entity_type: thingmqtt, resource: /iot/mqtt, service: service1, service_path: /sserv1 }"}}]}
```


### Update a service/subservice PUT (IoTAgent API)
If you want modify only a field, you can do it. You cannot modify an element into an array field, but whole array. ("/*" is not allowed).

+ Parameters [apikey, resource]

    + `apikey` (optional, string). If you don't specify, apikey="" is applied.

    + `resource` (mandatory, string). URI for service into iotagent.

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

    + Body

            {
               "entity_type": "entity_type"
            }

+ SUCCESS Response 204

#### Error responses:

+ Response 400:

Missing parameter: 
```
 {"reason": "A parameter is missing in the request","details": "resource parameter is mandatory in PUT operation"}
```
Missing service header.
```
 {"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercase"}
```
Wrong type for any of the fields (like using integer instead of string):
```
{"reason":"The request is not well formed","details":"invalid data type: UnsignedType expected: string [/services[0]/token]"}
```
Not valid URL for ContextBroker:
```
{"reason":"A parameter of the request is invalid/not allowed[http10.95.213.36:1026]","details":"http10.95.213.36:1026 is not a correct uri"}
```
Service contains badly formed attibutes:
```
{"reason":"The request is not well formed","details":"Missing required property: object_id [/services[0]/attributes[0]]"}
```
JSON is invalid or not well formed:
```
 {"reason":"The request is not well formed","details":"JSONParser: An error occurred on line 1 column 18"}
```
JSON contains unexpected fields:
```
 {"reason":"The request is not well formed","details":"Additional properties not allowed. [/services[0]/extra_data]"}
```

+ Response 404:

```
{"reason":"The service does not exist","details":"put_service_json service=ssrv  service_path=/ssr_1 (...)"}
```

### Modify a service (PUT) (IoTManager API)
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

#### Error messages:

+ Response 404

- When there's a problem within the JSON but it still was sent to different endpoints.

```
{"errors": [{"endpoint": "http://0.0.0.0:8080/iot","code": "400","details": {"reason":"A parameter of the request is invalid/not allowed[http//10.95.213.36:1026]","details":"http//10.95.213.36:1026 is not a correct uri"}},{"endpoint": "http://0.0.0.0:8082/iot","code": "400","details": {"reason":"A parameter of the request is invalid/not allowed[http//10.95.213.36:1026]","details":"http//10.95.213.36:1026 is not a correct uri"}}]}
```

- Not found services at the endpoints:
```
{"errors": [{"endpoint": "http://0.0.0.0:8080/iot","code": "404","details": {"reason":"The service does not exist","details":" [  put_service_json service=service1 service_path=/sserv1 service_id=service1 content={ apikey : apikeymqtt-1, token : tokenmqtt } resource=/iot/mqtt]"}},{"endpoint": "http://0.0.0.0:8082/iot","code": "404","details": {"reason":"The service does not exist","details":" [  put_service_json service=service1 service_path=/sserv1 service_id=service1 content={ apikey : apikeymqtt-1, token : tokenmqtt } resource=/iot/mqtt]"}}]}
```


### Remove a subservice/service [DELETE]
You remove a subservice into a service. If Fiware-ServicePath is '/*' or '/#' remove service and all subservices.

+ Parameters [apikey, resource, device]

    + `apikey` (optional, string). If you don't specify, apikey="" is applied.

    + `resource` (mandatory, string). URI for service into iotagent.

    + `device` (optional, boolean). Default value is false. Remove devices in service/subservice. This parameter is not valid when Fiware-ServicePath is '/*' or '/#'.

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

+ Response 204

#### Error responses:

+ Response 400:

Missing parameter ('resource'): 
```
 {"reason": "A parameter is missing in the request","details": "resource parameter is mandatory"}
```

Missing service header.
```
 {"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercase"}
```

+ Response 204:

If the service does not exist, it will still return an HTTP 204 code although nothing was actually deleted.



## Devices [/devices{?limit,offset,detailed,protocol,entity}]
A device is a resource that publish information to IoT Platform and it uses the IoT Agent.
### Device Model
- `device_id`. Unique identifier into a service.
- `protocol`. Protocol assigned to device. This field is easily provided by IoTA Manager if it is used. Every module implementing a protocol has an identifier.
- `entity_name`. Entity name used for entity publication (overload default)
- `entity_type`. Entity type used for entity publication (overload entity_type defined in service).
- `timezone`. Not used in this version.
- `attributes`. Mapping for protocol parameters to entity attributes.
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

Mandatory fields are identified in every operation.

### Retrieve all devices [GET]

+ Parameters [limit, offset, detailed, entity, protocol]


    + `limit` (optional, number). In order to specify the maximum number of devices (default is 20, maximun allowed is 1000).

    + `offset` (optional, number). In order to skip a given number of elements at the beginning (default is 0) .

    + `detailed` (optional, string). `on` return all device information, `off` (default) return only name.

    + `entity` (optional, string). It allows get a device from entity name.

    + `protocol` (optional, string). It allows get devices with this protocol.

+ Request (application/json)

    + Headers

            Fiware-Service: testService
            Fiware-ServicePath: /TestSubservice

+ Response 200

    + Body

           {
              "count": 1,
              "devices": [
                {
                  "device_id": "device_id",
                  "protocol": "12345",
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

### Create a device  POST  (IoTAgent API)
From device model, mandatory fields are: device_id and protocol.

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

    + Body

           {
                "devices": [
                {
                  "device_id": "device_id",
                  "protocol": "12345",
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

    + Headers (only if ONE device is in request)

            Location: /iot/devices/device_id

#### Error responses:

+ Response 400:

Missing property within  JSON:
```
{"reason":"The request is not well formed","details":"Missing required property: device_id [/devices[0]]"}
```

Service or sub-service headers not valid (as per Context Broker constraints):
```
 {"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercase"}
```
```
{"reason": "Malformed header","details": "Fiware-ServicePath not accepted - a service path string must only contain underscores and alphanumeric characters and starts with character /"}
```
Missing service header.
```
 {"reason": "Malformed header","details": "Fiware-Service not accepted - a service string must not be longer than 50 characters and may only contain underscores and alphanumeric characters and lowercase"}
```

Missing mandatory field within JSON:
```
{"reason":"The request is not well formed","details":"Missing required property: device_id [/devices[0]]"}
```

When device's protocol is not supported by that instance of IotAgent (the plugin supporting that protocol is not running), or the service that is being used in the request was not provisioned for that protocol (resource field links protocols and services when they are provisioned) or the protocol is misspelled and was not identified. 

```
{"reason":"There are conflicts, protocol is not correct","details":" [ protocol: PDI-IoTA-UltraLight]"}
```

Wrong type for any of the fields (like using integer instead of string):
```
{"reason":"The request is not well formed","details":"invalid data type: UnsignedType expected: string [/devices[0]/entity_type]"}
```

Device contains badly formed attributes:
```
{"reason":"The request is not well formed","details":"Missing required property: name [/devices[0]/static_attributes[0]]"}
```

JSON is invalid or not well formed:
```
 {"reason":"The request is not well formed","details":"JSONParser: An error occurred on line 1 column 18"}
```

JSON contains unexpected fields:
```
 {"reason":"The request is not well formed","details":"Additional properties not allowed. [/devices[0]/extra]"}
```

+ Response 409

Duplicated device:
```
 {"reason":"There are conflicts, entity already exists","details":" [ entity_name: entity_name]"}
```

+ Response 404

When service does not exist:
```
{"reason":"The service does not exist","details":" service:serv22 service_path:/srf"}
```

### Create a device  POST  (IoTManager API)
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

- No protocol is available at any of the endpoints, so the request is not forwarded to any endpoint:
```
{"reason":"The request is not well formed","details":"[protocol:PDI-IoTA-UltraLight service: service1 service_path:/sserv1]"}
```

+ Response 500
If the request is forwarded to the endpoints but all of them fail (for whatever reason), then the response code is 500 and the body will contain the individual reasons for each error:

- Existing Device. 
```
{"errors": [{"endpoint": "http://0.0.0.0:8080/iot/devices","code": "409","details": {"reason":"There are conflicts, entity already exists","details":" [ entity_name: entity_name]"}},
{"endpoint": "http://0.0.0.0:8082/iot/devices","code": "409","details": {"reason":"There are conflicts, entity already exists","details":" [ entity_name: entity_name]"}}]}
```


## Device [/devices/{device_id}]

### Retrieve a device [GET]

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

+ Response 200

    + Body

            {
              "device_id": "device_id",
              "protocol": "121345",
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

#### Error messages:

+ Response 404:

When the device is not found, either device or service does not exist.

```
{"reason":"The device does not exist","details":"2"}
```


### Update a device [PUT]
If you want modify only a field, you can do it, except field `protocol` (this field, if provided it is removed from request).

+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

    + Body

            {
               "entity_name": "entity_name"
            }


+ Response 204

#### Error messages:

+ Response 404:

When device or service doesn't exist:
```
{"reason":"The device does not exist","details":" [ device: put_device_json service=servicesss1 service_path=/sserv1 device=device_id content={ entity_name: entity_name_dos}]"}
```

+ Response 400:

When trying to change unexpected fields:
```
{"reason":"The request is not well formed","details":"Additional properties not allowed. [/entity_]"}
```



### Remove a device [DELETE]
If specific device is not found, we work as deleted.
+ Request (application/json)

    + Headers

            Fiware-Service: testservice
            Fiware-ServicePath: /TestSubservice

+ Response 204


#### Error messages:

+ Response 204:

When device or service doesn't exist, nothing is deleted, but 204 http code is returned. 





