# Provision API
This section only applies when storage type is NOT _file_. Provision API does not apply to information stored in file that is considered as static.
If you want manage services or devices, you must enable mongo as storage type.
These examples does not use authorization configuration.
There is a section for [commands](commands.md) that may be interesting in order to add devices.

<a name="def-service-api"></a>
## Service API
### Add service
Add a service and subservice defined by Fiware-Service and Fiware-ServicePath.
Request:
```
curl -X POST http://127.0.0.1:8080/iot/services \
  -i \
  -H "Content-Type: application/json" \
  -H "Fiware-Service: TestService" \
  -H "Fiware-ServicePath: /TestSubservice" \
  -d '{"services": [{ "apikey": "apikey3", "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/test" }]}'
```
Response:
```
HTTP/1.1 201 Created
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 0
```

### Get all subservices
Actually, this request retrieves all subservices into service defined by Fiware-ServicePath.
Request:
```
curl http://127.0.0.1:8080/iot/services \
     -i \
     -H "Fiware-Service: TestService" \
     -H "Fiware-ServicePath: /*"
```
Response:
```
HTTP/1.1 200 OK
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 57

{ "count": 1,"services": [{ "apikey" : "apikey3", "token" : "token2", "cbroker" : "http://127.0.0.1:1026", "entity_type" : "thing", "resource" : "/iot/test", "service" : "TestService", "service_path" : "/TestSubservice1" }]}
```

### Get service and subservice
This request retrieves service with subservice information defined by Fiware-Service and Fiware-ServicePath.
Request:
```
curl http://127.0.0.1:8080/iot/services \
      -i \
      -H "Fiware-Service: TestService" \
      -H "Fiware-ServicePath: /TestSubservice"
```
Response:
```
HTTP/1.1 200 OK
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 194

{ "count": 1,"services": [{ "apikey" : "apikey", "cbroker" : "http://127.0.0.1:1026", "entity_type" : "thing", "resource" : "/iot/test", "service" : "TestService", "service_path" : "TestSubservice", "token" : "token2" }]}
```
### Modify service
You only could modify a subservice into a service defined by Fiware-ServicePath and Fiware-Service.
Request:
```
curl -X PUT 'http://127.0.0.1:8080/iot/services/TestService?apikey=apikey3&resource=/iot/test' \
  -i \
  -H "Content-Type: application/json" \
  -H "Fiware-Service: TestService" \
  -H "Fiware-ServicePath: /TestSubservice" \
  -d '{ "apikey": "NEWapikey", "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/test" }'
```
Response:
```
HTTP/1.1 204 No Content
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 0
```
### Delete service
You only could delete a subservice into a service defined by Fiware-ServicePath and Fiware-Service.
Request:
```
curl -X DELETE 'http://127.0.0.1:8080/iot/services?apikey=apikey3&resource=/iot/test' \
     -i \
     -H "Fiware-Service: TestService" \
     -H "Fiware-ServicePath: /TestSubservice"
```
Response:
```
HTTP/1.1 204 No Content
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 0
```
<a name="def-device-api"></a>
## Device API
You could check out [Commands](commands.md) for more information about how commands work.
### Add device
Request:
```
curl -X POST http://127.0.0.1:8080/iot/devices \
  -i \
  -H "Content-Type: application/json" \
  -H "Fiware-Service: TestService" \
  -H "Fiware-ServicePath: /TestSubservice" \
  -d ' { "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type", "timezone": "America/Santiago", "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int" } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value" } ] } ] }'
```
Response:
```
HTTP/1.1 201 Created
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 0
Location: /iot/devices/device_id
```
### Get all devices
Request:
```
curl http://127.0.0.1:8080/iot/devices \
     -i \
     -H "Fiware-Service: TestService" \
     -H "Fiware-ServicePath: /TestSubservice"
```
Response:
```
HTTP/1.1 200 OK
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 56

{ "count": 1,"devices": [{ "device_id" : "device_id" }]}
```
If you use detailed=on parameter, every device is retrieved with additional information.
Request:
```
curl 'http://127.0.0.1:8080/iot/devices?detailed=on' \
      -i \
      -H "Fiware-Service: TestService" \
      -H "Fiware-ServicePath: /TestSubservice"
```
Response:
```
HTTP/1.1 200 OK
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 393

{ "count": 1,"devices": [{ "device_id" : "device_id", "entity_name" : "entity_name", "entity_type" : "entity_type", "timezone" : "America/Santiago", "attributes" : [ { "object_id" : "source_data", "name" : "attr_name", "type" : "int" } ], "static_attributes" : [ { "name" : "att_name", "type" : "string", "value" : "value" } ], "service" : "TestService", "service_path" : "/TestSubservice" }]}
```
### Modify device
Request: (modifying timezone)
```
curl -X PUT http://127.0.0.1:8080/iot/devices/device_id \
   -i \
   -H "Content-Type: application/json" \
   -H "Fiware-Service: TestService" \
   -H "Fiware-ServicePath: /TestSubservice" \
   -d '{ "timezone": "Europe/Madrid" }'
```
Response:
```
HTTP/1.1 204 No Content
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 0
```

### Delete device
Request:
```
curl -X DELETE http://127.0.0.1:8080/iot/devices/device_id \
     -i \
     -H "Fiware-Service: TestService" \
     -H "Fiware-ServicePath: /TestSubservice"
```
Response:
```
HTTP/1.1 204 No Content
Connection: Keep-Alive
Host: 127.0.0.1:8080
Content-Length: 0
```
See complete API [API_REFERENCE](API_REFERENCE.md)
