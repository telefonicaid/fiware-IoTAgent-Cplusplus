#<a name="top"></a>Sanity check procedures

* [Sanity check iotagent](#iotagent-testing)
* [Sanity check iota manager](#iota-manager-testing)
* [Network Interfaces Up and Open](#network-interfaces-up-and-open)
* [Databases](#databases)
* [More tests ](#end-to-end-checks)

The Sanity Check Procedures are the steps that a System Administrator will take to verify that an installation is
ready to be tested. This is therefore a preliminary set of tests to ensure that obvious or basic malfunctioning
is fixed before proceeding to unit tests, integration tests and user validation.
You can test your installation by hand, executing the curls of this document, or executing a linux script [sanity_check.sh](../scripts/sanity_check.sh)

## <a name="iotagent-testing">Sanity check iotagent</a>

-   Start iotagent in default port (8080)

-   Check that process is alive, execute in your console
   
```
$ps -ef | grep iot
```
 
 You must see something like that

```
iotagent 19682     1  0 10:32 ?        00:00:00 /usr/local/iot/bin/iotagent -n IoTPlatform -v INFO -i 192.0.3.25 -p 8080 -d /usr/local/iot/lib -c /etc/iot/config.json
```

-   Check version

```
$curl --header 'Accept: application/json' localhost:8080/iot/about
```

You must see something like that

```
Welcome to IoTAgents 1.2.1 commit 38.g6e65976 in Sep 24 2015
```

-   Check that you can create a service

```
$curl -X POST localhost:8080/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "resource": "/iot/d" }]}'
```

response code must be HTTP/1.1 201 Created

-   Check that service is created

```
$curl -X GET localhost:8080/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" 
```

response HTTP/1.1 200 OK

```
{ "count": 1,"services": [{ "apikey" : "apikey", "token" : "token", "entity_type" : "thingsrv", "resource" : "/iot/d", "service" : "sanitysrv", "service_path" : "/sanitysspath" }]}
```

-   Create a device is created

```
$curl -X POST localhost:8080/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' 
```

HTTP/1.1 201 Created

-   Check that device is created

```
$curl -X GET localhost:8080/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" 
```

response HTTP/1.1 200 OK

```
{ "count": 1,"devices": [{ "device_id" : "sensor_ul" }]}
```

-   Send an observation

```
$curl -X POST 'localhost:8080/iot/d?i=sensor_ul&k=apikey' \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '2014-02-18T16:41:20Z|t|23'
```

response HTTP/1.1 200 OK

-   Delete service

```
$curl -X DELETE "localhost:8080/iot/services?resource=/iot/d&device=true" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath"  
```

response HTTP/1.1 204 No Content


[Top](#top)

## <a name="iota-manager-testing">Sanity check iota manager</a>

-   Start iotagent manager in default port (8081)

-   Check that process is alive, execute in your console
   
```
$ps -ef | grep iot
```
 
 You must see something like that

```
iotagent 19998     1  0 10:47 ?        00:00:00 /usr/local/iot/bin/iotagent -m -n Manager -v INFO -i 192.0.3.25 -p 8081 -d /usr/local/iot/lib -c /etc/iot/config.json
```

-   Check version

```
$curl --header 'Accept: application/json' localhost:8081/iot/about
```

You must see something like that

```
Welcome to IoTAgents  [working as manager] 1.2.1 commit 38.g6e65976 in Sep 24 2015
```

-   Check that you can create a service

```
$curl -X POST localhost:8081/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "protocol": ["PDI-IoTA-UltraLight"] }]}'
```

response code must be HTTP/1.1 201 Created

-   Check that service is created

```
$curl -X GET localhost:8081/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" 
```

response HTTP/1.1 200 OK

```
{ "count": 1,"services": [{ "iotagent" : "http://192.0.3.25:8080/iot", "protocol" : "PDI-IoTA-UltraLight", "service" : "sanitysrvm", "service_path" : "/sanitysspath", "apikey" : "apikey", "token" : "token", "entity_type" : "thingsrv", "description" : "UL2" }]}
```

-   Create a device is created

```
$curl -X POST localhost:8081/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' 
```

HTTP/1.1 201 Created

-   Check that device is created

```
$curl -X GET localhost:8081/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" 
```

response HTTP/1.1 200 OK

```
{ "count": 1,"devices": [{ "device_id" : "sensor_ul" }]}
```

-   Delete device

```
$curl -X DELETE "localhost:8081/iot/devices/sensor_ul?protocol=PDI-IoTA-UltraLight" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath"
```

HTTP/1.1 204 No Content

-   Delete service

```
$curl -X DELETE "localhost:8081/iot/services?protocol=PDI-IoTA-UltraLight&device=true" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath"  
```

response HTTP/1.1 204 No Content

[Top](#top)

## <a name="network-interfaces-up-and-open">Network Interfaces Up and Open</a>


Important ports must be accessible.

Iot agent http tcp 8080 as default port, although it can be changed using the -p command line option.

Iot manager http 8081 as default port, although it can be changed using the -p command line option.

mongo database 1027 default port, in /etc/iot/config.json storage parameter.

[Top](#top)

## <a name="databases">Databases</a>

Iota uses a MongoDB database, whose parameters are
provided in config file used by iotagent -c /etc/iot/config.json 
```
"storage": {
        "type": "mongodb",
        "dbname": "iot",
        "host": "127.0.0.1",
        "port": "27017",
        "replicaset": ""
  },
```

You can check that the database is working using the mongo console:

```
mongo <host>/<dbname>
```

You can check the different collections  using the
following commands in the mongo console. 

```
> db.PROTOCOL.count()
> 
> db.SERVICE_MGMT.count()
> 
> db.SERVICE.count()
> 
> db.DEVICE.count()
```

[Top](#top)

## <a name="end-to-end-checks">More tests</a>


To allow automatic check iot configuration and integration with Context Broker, there are several shell scripts, and there are more scripts to check more features .

to see documentation

``` 
./sanity_check.sh --help
```

You can use environment variables to set iotagent uri (export HOST_IOT=127.0.0.1:8080) or iot manager uri (export HOST_IOT=127.0.0.1:8081) or context broker uri (export HOST_CB=127.0.0.1:1026) 

[sanity_check.sh](../scripts/sanity_check.sh) same tests in this document

[check_iotagent.sh](../scripts/check_iotagent.sh) check iotagent integration with context broker (observations saved in cb)

[check_manager.sh](../scripts/check_manager.sh)
check iot manager integration with context broker 
