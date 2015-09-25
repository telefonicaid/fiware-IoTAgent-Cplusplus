#<a name="top"></a>Sanity check procedures

* [Sanity check iotagent](#iotagent-sanity)
* [End to end testing iotagent](#iotagent-testing)
* [Sanity check iota manager](#iota-manager-sanity)
* [End to end testing iota manager](#iota-manager-testing)
* [List of running processes](#list-processes)
* [Network Interfaces Up and Open](#network-interfaces-up-and-open)
* [Databases](#databases)
* [Script tests ](#end-to-end-checks)

The Sanity Check Procedures are the steps that a System Administrator will take to verify that an installation is
ready to be tested. This is therefore a preliminary set of tests to ensure that obvious or basic malfunctioning
is fixed before proceeding to unit tests, integration tests and user validation.


## <a name="iotagent-sanity">Sanity check iotagent</a>

-   Start iotagent in default port (8080), or change 8080 with your configured port (-p option in command line)

```
export HOST_IOT=127.0.0.1:8080
```
This environment variable is used in the rest of the document, to avoid rewrite the port

-   Check that process is alive, execute in your console
   
```
ps -ef | grep iot
```
 
 You must see something like that

```
iotagent 19682     1  0 10:32 ?        00:00:00 /usr/local/iot/bin/iotagent -n IoTPlatform -v INFO -i 192.0.3.25 -p 8080 -d /usr/local/iot/lib -c /etc/iot/config.json
```

If there is not a process like that, see logs in /var/log/iot (dir_log parameter in config file).

-   Check version

```
curl --header 'Accept: application/json' $HOST_IOT/iot/about
```

You must see something like that

```
Welcome to IoTAgents 1.2.1 commit 38.g6e65976 in Sep 24 2015
```

-   Check that admin iota uri is active
   
```
curl -X POST $HOST_IOT/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"services": [{ "token": "token", "entity_type": "thingsrv", "resource": "/iot/d" }]}'
```
 Response  HTTP/1.1 400 Bad Request
```
{"reason":"The request is not well formed","details":"Missing required property: apikey [/services[0]]"}
``` 

## <a name="iotagent-testing">End to end testing iotagent</a>

-   Check that you can create a service

```
curl -X POST $HOST_IOT/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "resource": "/iot/d" }]}'
```

response code must be HTTP/1.1 201 Created

-   Check that service is created

```
curl -X GET $HOST_IOT/iot/services \
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
curl -X POST $HOST_IOT/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' 
```

HTTP/1.1 201 Created

-   Check that device is created

```
curl -X GET $HOST_IOT/iot/devices \
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
curl -X POST '$HOST_IOT/iot/d?i=sensor_ul&k=apikey' \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '2014-02-18T16:41:20Z|t|23'
```

response HTTP/1.1 200 OK

-   Delete service

```
curl -X DELETE "$HOST_IOT/iot/services?resource=/iot/d&device=true" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrv" \
-H "Fiware-ServicePath: /sanitysspath"  
```

response HTTP/1.1 204 No Content


[Top](#top)

## <a name="iota-manager-testing">Sanity check iota manager</a>

-   Start iotagent manager in default port (8081), or change 8081 with your configured port (-p option in command line)

```
export HOST_MAN=127.0.0.1:8081
```
This environment variable is used in the rest of the document, to avoid rewrite the port

-   Check that process is alive, execute in your console
   
```
ps -ef | grep iot
```
 
 You must see something like that

```
iotagent 19998     1  0 10:47 ?        00:00:00 /usr/local/iot/bin/iotagent -m -n Manager -v INFO -i 192.0.3.25 -p 8081 -d /usr/local/iot/lib -c /etc/iot/config.json
```

-   Check version

```
curl --header 'Accept: application/json' $HOST_MAN/iot/about
```

You must see something like that

```
Welcome to IoTAgents  [working as manager] 1.2.1 commit 38.g6e65976 in Sep 24 2015
```

-   Check that admin uri is active

```
curl -X POST $HOST_MAN/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"services": [{ "token": "token", "entity_type": "thingsrv", "protocol": ["PDI-IoTA-UltraLight"] }]}'
```

response HTTP/1.1 400 Bad Request

```
{"reason":"The request is not well formed","details":"Missing required property: apikey [/services[0]]"}
```

## <a name="iota-manager-testing">End to end testing iota manager</a>

-   Check that you can create a service

```
curl -X POST $HOST_MAN/iot/services \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"services": [{ "apikey": "apikey", "token": "token", "entity_type": "thingsrv", "protocol": ["PDI-IoTA-UltraLight"] }]}'
```

response code must be HTTP/1.1 201 Created

-   Check that service is created

```
curl -X GET $HOST_MAN/iot/services \
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
curl -X POST $HOST_MAN/iot/devices \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath" \
-d '{"devices":[{"device_id":"sensor_ul","protocol":"PDI-IoTA-UltraLight", "commands": [{"name": "PING","type": "command","value": "" }]}]}' 
```

HTTP/1.1 201 Created

-   Check that device is created

```
curl -X GET $HOST_MAN/iot/devices \
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
curl -X DELETE "$HOST_MAN/iot/devices/sensor_ul?protocol=PDI-IoTA-UltraLight" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath"
```

HTTP/1.1 204 No Content

-   Delete service

```
curl -X DELETE "$HOST_MAN/iot/services?protocol=PDI-IoTA-UltraLight&device=true" \
-i  \
-H "Content-Type: application/json" \
-H "Fiware-Service: sanitysrvm" \
-H "Fiware-ServicePath: /sanitysspath"  
```

response HTTP/1.1 204 No Content

[Top](#top)


## <a name="list-processes">List of running processes</a>

check that all processes are up, to check it execute

```
ps -ef | grep iot
```
- If the iot agent is active, the following processes should be active:
```
iotagent 19682     1  0 10:32 ?        00:00:00 /usr/local/iot/bin/iotagent -n IoTPlatform -v INFO -i 192.0.3.25 -p 8080 -d /usr/local/iot/lib -c /etc/iot/config.json
```
- If storage type is "mongodb" you need a mongodb database up.   
``` 
mongod    1173     1  0 Jul09 ?        08:55:40 /usr/bin/mongod -f /etc/mongod.conf
```
- If the MQTT protocol is active, the following processes should be active:  Mosquitto
```
root      8098     1  0 Jul13 ?        00:47:26 /usr/sbin/mosquitto -c /etc/iot/mosquitto.conf
``` 
- If the IoT Manager module  is active, the following processes should be active: MongoDB
```
iotagent 20291     1  0 Sep24 ?        00:00:42 /usr/local/iot/bin/iotagent -m -n Manager -v INFO -i 192.0.3.25 -p 8081 -d /usr/local/iot/lib -c /etc/iot/config.json

```

[Top](#top)

## <a name="network-interfaces-up-and-open">Network Interfaces Up and Open</a>


Important ports must be accessible.

- If the iot agent is active, http tcp 8080 as default port, although it can be changed using the -p command line option.

- If the iot manager http 8081 as default port, although it can be changed using the -p command line option.

- If storage type is "mongodb", mongo database 1027 default port, in /etc/iot/config.json storage parameter.

- If the MQTT protocol is active, mosquitto port is 1883, or check configuration file for mqtt plugin (/etc/iot/sensormqtt.xml  <input type="mqtt" mode="server".. host="localhost" port="1883")

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

## <a name="end-to-end-checks">Script tests</a>


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
