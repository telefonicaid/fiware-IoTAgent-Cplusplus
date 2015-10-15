# Deployment

 This chapter decribe how to deploy an iotagent, configure and start using it.

#### Index
1. [Introduction](#def-introduction)
2. [Using RPMs](#def-rpms)
3. [Iotagent configuration](#def-configuration)
    1. [Resources](#def-resources)
    2. [Storage](#def-storage)
        1. [File storage](#def-file-storage)
        2. [Mongodb storage](#def-mongo-storage)
    3. [Security oauth](#def-security)
    4. [Ngsi](#def-ngsi)
    5. [Logs](#def-logs)
    6. [Timezone](#def-timezone)
    7. [Schema Validation](#def-schema)
    8. [HTTP Proxy](#def-proxy)
    9. [General configuration](#def-general)
    10. [Examples](#def-examples)
        1. [Example mongo storage, example resource](#def-examples1)
        2. [Example file storage](#def-examples2)
4. [Check Instalation with HTTP request with curls](#def-check)
5. [Monitoring Events and Alarms](#def-alarms)
6. [Using SmartM2M VPNs](VPNs.md)

<a name="def-introduction"></a>
## 1. Introduction
As IoT Agent is based on PION, each HTTP Plugin (or just Plugin) is loaded as a module meaning that the an URL is associated to each of them. HTTP Plugins actually implement the set of different IoT protocols that the IoT Agent supports. In other words, one instance of IoT Agent can handle multiple protocols at the same time thanks to PION's modules loading capabilities. Therefore, the URL linked to each plugin (_resource_ field in configuration) identifies an entry point at the HTTP server and it will be used by M2M devices that use HTTP as transport protocol.

However, there are some cases where devices may use a non-HTTP based protocol. In any case, a HTTP plugin will be still used and the difference being that this module would handle its own entry points for devices (MQTT protocol requires a connection to a MQTT broker, for instance).

This is the relationship between the main elements for non-HTTP protocols.
```Device entry point``` <-> ```PION Module``` <-> ```URL into HTTP server```.

<a name="def-rpms"></a>
## 2. Using RPMs

### Iota rpms installation

View [Build](build.md) section for more information about how rpms are built from source.

Base RPM is mandatory and optionally several plugins can be installed. For example if you want to use UL20 protocol, you need to install two packages :

```
  iot-agent-base-1.0.0-95.g250ee6f.x86_64.rpm
  iot-agent-ul-1.0.0-95.g250ee6f.x86_64.rpm
```

Tag ```95.g250ee6f```  is a reference to GIT code commit and will change.

Download  rpms which name began with  ```iot-agent-base```  and  ```iot-agent-ul``` and copy then to CentOS6.5 VM in Filab.

With root privileges, install  base and ul rpms :

```
  rpm -i  iot-agent-base-1.0.0-95.g250ee6f.x86_64.rpm
  rpm -i  iot-agent-ul-1.0.0-95.g250ee6f.x86_64.rpm
```
Other protocols like mqtt or ThinkingThings have to be installed separately using their respective RPMs.
Binaries will be installated in path  ```/usr/local/iot/bin``` and libraries in path ```/usr/local/iot/lib```
After successfully installing all desired plugins, the next step will be to configure the IoTAgent. All configuration is stored in ```/etc/iot``` folder.
See [Iotagent configuration](#def-configuration) for more information.


### Mongo db installation

Download mongodb v 2.6 from:

```
https://www.mongodb.org/dl/linux/x86_64
```

You can download this file mongodb-linux-x86_64-2.6.9.tgz (or choose the one for your Linux distribution) , and copy it to your machine.

Choose an installation path for mongo,  for example :

```
 /usr/local/iot
```

With root privileges:

```
cp mongodb-linux-x86_64-2.6.9.tgz   /usr/local/iot
cd /usr/local/iot
tar xvf mongodb-linux-x86_64-2.6.9.tgz
```

Create  log  and  data  directories :

```
cd /usr/local/iot/mongodb-linux-x86_64-2.6.9
mkdir ./log
mkdir ./data
cd data
mkdir db
```

You can create a mongod start script

```
cd /usr/local/iot/mongodb-linux-x86_64-2.6.9/

vi start_mongod.sh

/usr/local/iot/mongodb-linux-x86_64-2.6.9/bin/mongod --dbpath /usr/local/iot/mongodb-linux-x86_64-2.6.9/data/db --port 27017 --logpath /usr/local/iot/mongodb-linux-x86_64-2.6.9/log/mongoc.log --pidfilepath /usr/local/iot/mongodb-linux-x86_64-2.6.9/log/mongod.pid --logappend  &
```

Ensure exceution privileges :

```
chmod a+x  start_mongod.sh
```

In order to connect to mongodb:

```
cd /usr/local/iot/mongodb-linux-x86_64-2.6.9/bin

./mongo
```


<a name="def-configuration"></a>
## 3. Iotagent configuration

Iotagent requires a configuration file,  named config.json and placed in ```/etc/iot```. The file has different sections that will be explained below. A minimal working file will look like this:

```
{
    "ngsi_url": {
        "updateContext": "/NGSI10/updateContext",
        "registerContext": "/NGSI9/registerContext",
        "queryContext": "/NGSI10/queryContext"
    },
    "timeout": 10,
    "http_proxy": "192.0.0.2:3128",
    "public_ip": "10.95.200.200:20000",
    "dir_log": "/tmp/",
    "timezones": "/etc/iot/date_time_zonespec.csv",
    "storage": {
        "host": "127.0.0.1",
        "type": "mongodb",
        "port": "27017",
        "dbname": "iot"
    },
   "resources": [
        {
            "resource": "/iot/d",
            "options": {
                "FileName": "UL20Service"
            }
         }
   ]
}
```
That file will be valid for an IoTAgent with no authentication, no load balancer, one plugin (UltraLight) and using a local mongodb as storage.

#### Starting IoTAgent

Now you can create an start script, for example  init_iotagent.sh

```
  export LD_LIBRARY_PATH=/usr/local/iot/lib

  /usr/local/iot/bin/iotagent -n qa -i  x.x.x.x -d /usr/local/iot/lib -c ./config.json -v DEBUG  &
```

#### Starting IoTAgent as Manager
For doing so, you have to include another parameter into the command line: "-m". Please be aware that it will start a new process with IP and Port as specified by command line, so if you plan to start both iotagent and iotagent-manager in the same machine, ports must be different.

Replace  x.x.x.x  with  VM IP address. By default  iotagent listen in port 8080

#### Identify an IoTAgent in Manager

If you have several iotagents, you will use an iot manager, and it is very usefull to identify iotagent with an special name, and avoid problems if your host hasn't got a fixed ip.

This is new in release 1.2.1, in older version if you ask for protocols

curl -X GET http://localhost:8081/iot/protocols -i -H "Content-Type: application/json"

```
{
  "count": 1,
  "protocols": [
    {
      "protocol": "PDI-IoTA-UltraLight",
      "description": "UL2",
      "endpoints": [
        {
          "endpoint": "http://192.0.3.25:8080/iot",
          "resource": "/iot/d"
        }
      ]
    }
  ]
}
```

If you update to release1.2.1  the response is

```
{
  "count": 1,
  "protocols": [
    {
      "protocol": "PDI-IoTA-UltraLight",
      "description": "UL2",
      "endpoints": [
        {
          "endpoint": "http://192.0.3.25:8080/iot",
          "resource": "/iot/d",
          "identifier": "IoTPlatform:8080"
        }
      ]
    }
  ]
}
```

There is a new parameter identifier, with the name and port of the iotagent, unique identifier for this iotagent, and unique identifier for this endpoint.

How can you set this identifier:

1- In command line exists another parameter to define the iota identifier
-I or --identifier set the identifier. Pay attention, you cannot put the same name in two iotagents with the same port and protocol.

2- In config file there is another way to define the identifier for iotagent.
```
{
  "identifier":"id1",
....
```
This name is only used to talk with iota manager.

3- In command line exists a paramater -n or --name  to set a name for iotagent.
 This name is used to set the file name for logs (it will be IoTAgent-<name>.log) this is the reason to create before parameter to define the iota identifier different for log files.

4- If you do not use any of the above options, idenfifier will be ip with port.

if you choose several ways to set the identifier, the more priority it is first, second, ...


To see the identifier for an iotagent execute

```
curl -X GET http://localhost:8080/iot/about
```

then you can see the identifier

```
Welcome to IoTAgents  identifier:idcl1:8080  1.2.1 commit 55.g7fd08a6 in Sep 30 2015
```

When you ask to iot manager for iotagents and protocols, you can see identifiers
[API REFERENCE MANAGER](API_REFERENCE_MANAGER.md)

#### Starting IoTAgent as a Service

After installing iot-agent-base RPM an init.d script can be found in this folder ```/usr/local/iot/init.d```. Such script will take all input parameters for starting the IoTAgent process from a config file located in ```/usr/local/iot/config``` and named __iotagent_protocol.conf__ or __iotagent_manager.conf__ (for starting the IoTAgent as manager).

You can configure the script as any other Linux service. Then you would start the IoTAgent by using
```sudo service iotagent start protocol``` for Normal IoTAgent operation.
For Manager, you would issue the following command:
```sudo service iotagent start manager```.


<a name="def-resources"></a>
### 1. Resources

An IoT Agent can manage several protocols or several instances of the same protocol. A protocl is implemented in a module that is assigned to one or more URIs.
You must define the protocol/module/resource, the URI where you should send the messages. Every _resource_ is defined in array _resources_.
- `resource` : URI assigned to a module/protocol.
- `options` : How the HTTP server (view [Architecture](architecture.md)) starts this module. It contains a mandatory field `FileName` to determine the module file (.so) that must be loaded. An optional field `ConfigFile` could be provided if module needs it, so that's depend on module implementation.

Example:

        {  "resources": [
                {
                   "resource": "/iot/res",
                   "options": {
                     "ConfigFile": "/etc/config/IoT-1.xml",
                     "FileName": "IoTAgent-1"
                    }
                }
             ]
        }

You can specify _services_ in _resource_ but this way only must be used in test or static environments. In order to understand _service_ we recommend to take a look to [API REFERENCE](API_REFERENCE.md) or [Configuration API](north_api.md).

Example:

       { "resources": [
            {
                "resource": "/iot/d",
                "services": [
                        {
                        "apikey": "apikey1",
                        "service": "service1",
                        "service_path": "/srvpath1",
                        "token": "token2",
                        "cbroker": "http://127.0.0.1:6500",
                        "entity_type": "thing"
                        }
                ],
                "options": {
                "FileName": "UL20Service"
           }
        }

<a name="def-storage"></a>
### 2. Storage configuration
This section configures the storage of data. There are two possibilities:

A file, for testing, fixed devices, fixed services, etc.
A MongoDB database, with an API to manage and configure services and devices.

<a name="def-file-storage"></a>
#### 1. File Storage configuration

Example:

        {
          "storage": {
                       "type":"file",
                       "file":"/etc/iot/devices.json"
                     }
        }
For Device Model [API Reference](API_REFERENCE.md).

<a name="def-file-storage"></a>
#### 2. Mongodb Storage configuration

Parameters to spedify a MongoDB storage

| Parameter     |description               |
| ------------- |:-------------:           |
| type   | mongodb.     |
| dbname | database name in mongo     |

Use a mongodb replica set

| Parameter     |description               |
| ------------- |:-------------:           |
| host   | "host1:27018,host2:27018"     |
| replicaset | name of replica set     |

Use a mongos or mongod

| Parameter     |description               |
| ------------- |:-------------:           |
| host   | ip host     |
| port | mongo port   |

Mongod with  utenticacion

| Parameter     |description               |
| ------------- |:-------------:           |
| user   | user name     |
| pwd | password   |

Example:

    {
        "storage": {
            "type": "mongodb",
            "dbname": "iot",
            "host": "vmongodb2-1:27018,vmongodb2-2:27018",
            "replicaset": "dca",
            "user":"iot",
            "pwd":"iot"
      }
    }

<a name="def-security"></a>
### 3. Security configuration
When IoT Agent is integrated in OAuth environment based on Keyston Openstack, there are two features related to security. You can complete information in [Authentication](pep.md) section.
- PEP function: north-bound API (services and devices management) requires authenticate/authorize every operation. Every request brings a user token.
- On behalf function: when IoT Agent publishes it acts as user and it must provides a token in every request.
```
  "oauth": {
     "on_behalf_trust_url":"http://10.95.82.233:5001/v3/auth/tokens",
     "validate_token_url": "http://10.95.82.233:5001/v3/auth/tokens",
     "get_roles_url": "http://10.95.82.233:5001/v3/role_assignments",
     "retrieve_subservices_url": "http://10.95.82.233:5001/v3/projects",
     "access_control_url": "http://10.95.82.233:7070",
     "pep_user": "pep",
     "pep_password": "pep",
     "pep_domain": "admin_domain",
     "on_behalf_user": "iotagent",
     "on_behalf_password": "iotagent",
     "timeout": 5
  }
```
Fields with _on_behalf_ prefix are related to publish. Other fields configure PEP funcionality.
There is a field to configure actions as PEP function, but default configuration (view [IoT PEP](pep.md)) applies in most cases:
```
 "pep_rules": [
      {
          "verb": "POST",
          "action": "read",
          "uri": "/ngsi/<protocol>/queryContext"
       },
        {
          "verb": "POST",
          "action": "create",
          "uri": "/ngsi/<protocol>/updateContext"
        }
     ]
```
`verb` is HTTP method, `uri` is URI in HTTP request (you can see, url-base is ommited). `<` and `>` are used to defien a variable element in uri.

<a name="def-ngsi"></a>
### 4. Ngsi configuration (context broker configuration)
This section configures how and where information is published by IoT Agent.

| Parameter     |description               |
| ------------- |:-------------:           |
| cbroker   | default cbroker uri, only used if no defined in services     |
| updateContext | uri to send observations to context broker   |
| registerContext | uri to register device    |
| queryContext | uri to ask information to context broker   |



```
"ngsi_url": {
      "cbroker": "http://127.0.0.1:1026",
      "updateContext": "/NGSI10/updateContext",
      "registerContext": "/NGSI9/registerContext",
      "queryContext": "/NGSI10/queryContext"
  }
```
Every service could define a different Context Broker in order to publish information. But a default endpoint can be defined (field _cbroker_). Other fields define paths for every operation.

#### Using a Load Balancer.

If installation requires high availability and a load balancer will be used, ip:port of balancer should be configured in a field __public_ip__ into "ngsi_url". An example should be :
```
"ngsi_url": {
        "updateContext": "/NGSI10/updateContext",
        "registerContext": "/NGSI9/registerContext",
        "queryContext": "/NGSI10/queryContext",
        "public_ip": "10.95.200.200:20000"
    },
```

<a name="def-logs"></a>
### 5. Logs configuration
Command line to start IoT Agent define the log level (option -v). The folder where log file is generated is defined by _dir_log_ field. The name of log file is _IoTAgent-{name}.log_, where _{name}_ is provided with option -n in command line. When log file reaches 10 MB, the active file is renamed to _IoTAgent-{name}.log.1_. Maximum number of saved files is 5.
You should review [Monitoring alarms](#def-alarms).

<a name="def-timezone"></a>
### 6. Timezone configuration
This information is not used in this version, but it points where database (a file) for timezones is.

<a name="def-schema"></a>
### 7. Schema Validation
In order to validate requests to northbound API, we provide Json Schema. These schema are where _schema_path_ field (by default is /etc/iot).

<a name="def-proxy"></a>
### 8. HTTP server proxy
In order to send commands to devices in VPN, IoTAgent provides an infrastructure based in _squid_ as HTTP proxy and endpoint of GRE tunnels.

<a name="def-general"></a>
### 9. General configuration
Field _timeout_ is a general timer for operations.


<a name="def-examples"></a>
### 10. Examples

Now, we put all together, and ther are two examples, one with mongodb storage and second with file storage.
You can change for your specific values and use these files.

<a name="def-examples1"></a>
####1. Mongo storage and resource**

         {
           "resources": [
              {
                "resource": "/iot/d",
                "options": {
                  "FileName": "UL20Service"
                }
           },
           {
            "resource": "/iot/mqtt",
            "options": {
                "ConfigFile": "ESPService.xml",
                "FileName": "MqttService"
            }
           }
        ],
        "timezones": "/etc/iot/date_time_zonespec.csv",
        "schema_path": "/etc/iot",
        "storage": {
          "type": "mongodb",
          "dbname": "iot"
        },
        "ngsi_url": {
           "updateContext": "/NGSI10/updateContext",
           "registerContext": "/NGSI9/registerContext",
           "queryContext": "/NGSI10/queryContext"
         },
        "oauth": {
           "on_behalf_trust_url":"http://10.95.82.233:5001/v3/auth/tokens",
           "validate_token_url": "http://10.95.82.233:5001/v3/auth/tokens",
           "get_roles_url": "http://10.95.82.233:5001/v3/role_assignments",
           "retrieve_subservices_url": "http://10.95.82.233:5001/v3/projects",
           "access_control_url": "http://10.95.82.233:7070",
           "pep_user": "pep",
           "pep_password": "pep",
           "pep_domain": "admin_domain",
           "on_behalf_user": "iotagent",
           "on_behalf_password": "iotagent",
           "timeout": 5
         },
         "timeout": 11,
         "dir_log": "/var/log/"
       }

<a name="def-examples2"></a>
####2. File storage**

         {
           "resources": [
              {
                "resource": "/iot/d",
                "options": {
                  "FileName": "UL20Service"
                },
                "services": [
                    {
                    "apikey": "apikey3",
                    "service": "service2",
                    "service_path": "/",
                    "token": "token2",
                    "cbroker": "http://127.0.0.1:1026",
                    "entity_type": "thing"
                    }
                ]
           },
           {
            "resource": "/iot/mqtt",
            "options": {
                "ConfigFile": "ESPService.xml",
                "FileName": "MqttService"
            },
            "services": [
                    {
                    "apikey": "apikey4",
                    "service": "service4",
                    "service_path": "/",
                    "token": "token4",
                    "cbroker": "http://127.0.0.1:1026",
                    "entity_type": "thing"
                    },
                    {
                    "apikey": "apikey5",
                    "service": "service4",
                    "service_path": "/subservice1",
                    "token": "token4",
                    "cbroker": "http://127.0.0.1:1026",
                    "entity_type": "thing"
                    }
            ]
           }
        ],
        "timezones": "/etc/iot/date_time_zonespec.csv",
        "schema_path": "/etc/iot",
        "storage": {
          "type": "file",
          "file": "./devices.json"
        },
        "ngsi_url": {
           "updateContext": "/NGSI10/updateContext",
           "registerContext": "/NGSI9/registerContext",
           "queryContext": "/NGSI10/queryContext"
         },
        "oauth": {
           "on_behalf_trust_url":"http://10.95.82.233:5001/v3/auth/tokens",
           "validate_token_url": "http://10.95.82.233:5001/v3/auth/tokens",
           "get_roles_url": "http://10.95.82.233:5001/v3/role_assignments",
           "retrieve_subservices_url": "http://10.95.82.233:5001/v3/projects",
           "access_control_url": "http://10.95.82.233:7070",
           "pep_user": "pep",
           "pep_password": "pep",
           "pep_domain": "admin_domain",
           "on_behalf_user": "iotagent",
           "on_behalf_password": "iotagent",
           "timeout": 5
         },
         "timeout": 11,
         "dir_log": "/var/log/"
       }

<a name="def-check"></a>
## 4. Check Instalation with HTTP request with curls


You can send some HTTP request  to iotagent in order to create an servicem and a device.

To create a service you can try:

```
curl -X POST http://x.x.x.x:8080/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: service2" \
-H "Fiware-ServicePath: /srvpath2" \
-d '{"services": [{ "apikey": "apikey2", "token": "token2", "cbroker": "http://127.0.0.1:1026", "entity_type": "thing", "resource": "/iot/d" }]}'
```

Curl IP x.x.x.x  should by replaced by VM IP, and in the url defined in cbroker,  you should replace ip and port by the ones in which  contextBroker is installed.
To create a device  you can try :

```
curl -X POST http://x.x.x.x:8080/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: service2" \
-H "Fiware-ServicePath: /srvpath2" \
-d ' { "devices": [ { "device_id": "dev_1", "entity_name": "entity_1", "entity_type": "thing", "timezone": "America/Santiago", "commands": [ { "name": "ping", "type": "command", "value": "device_id@ping|%s"} ], "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int" } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value" } ] } ] }'
```

You can check correct creation in mongodb  of  service and  device, writing:

```
cd /usr/local/iot/mongodb-linux-x86_64-2.4.11/bin

./mongo

use  iot

db.SERVICE.find()

db.DEVICE.find()
```

<a name="def-alarms"></a>
## 5. Monitoring Events and Alarms

Alarms

Alarms is logged in FATAL level, and an alarm means that a communication between an other component is broken.
When the communication is restored other error log is written with event=END-ALARM, pay attention, only ERROR logs must be consider, logs in DEBUG level are normal and cannot be considered.

Every alarm has a code to identify it.

| code     |description               |
| ------------- |:-------------:           |
| 100   | No comunication with mongo database     |
| 200 | No comunication with Context broker     |
| 300 | No comunication with IoT Agent (IoT Agent Manager throws this alarm)     |
| XXXX | Code used in modules (user alarms) |

Example of broken communications with mongo database

```
time=2015-05-25T07-56-03,971.888CEST | lvl=ERROR | comp=iota:dev | op=put | file=[140148919482400:alarm.cc:81] | msg=event=ALARM code=100 origin= 127.0.0.1:27017 info=socket exception [CONNECT_ERROR] for 127.0.0.1:27017
time=2015-05-25T07-58-03,971.888CEST | lvl=ERROR | comp=iota:dev | op=put | file=[140148919482400:alarm.cc:81] | msg=event=ALARM code=100 origin= 127.0.0.1:27017 info=socket exception [CONNECT_ERROR] for 127.0.0.1:27017

...   Connection recovered  ...

time=2015-05-25T07-59-03,971.888CEST | lvl=ERROR | comp=iota:dev | op=remove | file=[140148573378304:alarm.cc:96] | msg=event=END-ALARM code=100 origin= 127.0.0.1:27017 info=MongoConnection OK

time=2015-05-25T07-59-03,971.888CEST | lvl=DEBUG | comp=iota:dev | op=remove | file=[140148573378304:alarm.cc:96] | msg=event=END-ALARM code=100 origin= 127.0.0.1:27017 info=MongoConnection OK
```
