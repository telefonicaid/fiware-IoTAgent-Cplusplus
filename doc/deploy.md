# Deployment

 This chapter decribe how to deploy an iotagent, configure and start using it.

#### Index
1. [Introduction](#def-introduction)
2. [Puppet procedure](#def-puppet)
3. [Using RPMs](#def-rpms)
4. [Iotagent configuration](#def-configuration)
    1. [Resources](#def-resources)
    2. [Storage](#def-storage)
        1. [File storage](#def-file-storage)
        2. [Mongodb storage](#def-mongo-storage)
    3. [Security oauth](#def-security)
    4. [Ngsi](#def-ngsi)
    5. [Logs](#def-logs)
    6. [Timezone](#def-timezone)
    7. [HTTP Proxy](#def-proxy)
    8. [General configuration](#def-general)
    9. [Examples](#def-examples)
        1. [Example mongo storage, example resource](#def-examples1)
        2. [Example file storage](#def-examples2)
5. [Check Instalation with HTTP request with curls](#def-check)
6. [Monitoring Events and Alarms](#def-alarms)

<a name="def-introduction"></a>
## 1. Introduction
As IoT Agent is based on PION, each HTTP Plugin (or just Plugin) is loaded as a module meaning that the an URL is associated to each of them. HTTP Plugins actually implement the set of different IoT protocols that the IoT Agent supports. In other words, one instance of IoT Agent can handle multiple protocols at the same time thanks to PION's modules loading capabilities. Therefore, the URL linked to each plugin (_resource_ field in configuration) identifies an entry point at the HTTP server and it will be used by M2M devices that use HTTP as transport protocol.

However, there are some cases where devices may use a non-HTTP based protocol. In any case, a HTTP plugin will be still used and the difference being that this module would handle its own entry points for devices (MQTT protocol requires a connection to a MQTT broker, for instance).

This is the relationship between the main elements for non-HTTP protocols.
```Device entry point``` <-> ```PION Module``` <-> ```URL into HTTP server```.

The IoT Agent can be installed following two procedures: puppet recipes, or manual installation of RPMs.

<a name="def-puppet"></a>
## 2. Puppet procedure.

Following this process, the IoT Agent will be installed and configured along with MongoDB for local use. By means of a Hieradata file, users can configure what protocols (or Plugins) are deployed in that particular instance.

We do not support puppet master infraestructure, so upon downloading the RPM with our puppet code, you have to run a ```puppet apply``` command.

The very first step is to install puppet (we recommend 3.2.7 or above). The installation process is well documented on their website and therefore, not covered in this guide.

### 1. Setting up Repositories.

All dependencies that our IoT Agent needs are stored in TID repositories. Including MongoDB RPM.

#### MongoDB Repository.

```
[mongodb]
name=mongodb Common Repository
baseurl=http://artifactory.hi.inet/artifactory/simple/common/mongodb/x86_64/
gpgcheck=0
enabled=1
metadata_expire=30
priority=1
proxy=http://prod-epg-ost-proxy-01.hi.inet:6666
```
Note: proxy may not be necessary

#### Puppet modules

```
[puppet-code]
name=Puppet-Code Repo
baseurl=http://artifactory.hi.inet/artifactory/simple/yum-puppet/x86_64/
enable=1
gpgcheck=0
proxy=http://prod-epg-ost-proxy-01.hi.inet:6666
```

#### Common dependencies
Like Monit, or Mosquitto
```
[SBC-Common]
baseurl=http://artifactory.hi.inet/artifactory/simple/yum-sbc/common6.5/x86_64/
proxy=http://prod-epg-ost-proxy-01.hi.inet:6666
metadata_expire=30
gpgcheck=0
enabled=1
name=SBC Common repository Artifactory
priority=1
```
#### Main Repository for IoT Agent packages

```
[SBC-Repo]
baseurl=http://artifactory.hi.inet/artifactory/simple/yum-sbc/iot-agent/x86_64/
proxy=http://prod-epg-ost-proxy-01.hi.inet:6666
metadata_expire=30
gpgcheck=0
enabled=1
name=SBC repository Artifactory
priority=1
```

As usual, you may wan to run ´´´yum makecache´´´ and check that all repositories are accessible.

### 2. Installing puppet code

Once repositories have been added (and checked they are accessible) and puppet installed, you can just install locally the puppet code. (As mentioned before, we don't support Puppet Master, so all puppet recipes must be downloaded locally to the host).

```
sudo yum install iot-agent-puppet
```

The expected outcome is that all puppet code were copied to ```/user/local/iot/puppet```


### 3. Local files creation.
Before running ```puppet apply``` command, two files must be created: the host's manifest and its corresponding hieradata.

#### Manifest.
Using the example file ```iota.pp``` recipe that can be found within ```/usr/local/iot/puppet/manifests/nodes``` you can just rename it to your host name and on the new file, change the name of the node.


#### Hieradata file explanation.
Same procedure has to be done for the hieradata file, which contains all main configuration parameters for the IoT Agent. The file is named ```iota.yaml``` and can be found at ```/usr/local/iot/puppet/hieradata/nodes```. Again, it has to be renamed to the host name and keeping the extension so puppet can use it for variable lookup.

Apart from rename it, configuration has to be carefully set in order to make IoT Agent run properly. Let's look at its main sections:

##### Main IoT Agent parameters
```
iota::base::mongodb_host: 127.0.0.1
iota::base::mongodb_port: 27017
iota::base::iota_server_address: 10.0.2.15
iota::base::iota_server_port: 80
iota::base::iota_server_name: qa
iota::base::iota_log_level: DEBUG
```
This will configure the IP and port of MongoDB that IoT Agent will connect to (it will be installed by default locally). It will also configure the IP and port that IoT Agent will bind to. Server Name is concatenated to "iotagent" and it will appear like that under monit status.
Log level is self explanatory.

##### NGSI Configuration
```
iota::ngsi::context_broker_endpoint: 'http://127.0.0.1:1026'
iota::ngsi::update_context_path: /NGSI10/updateContext
iota::ngsi::register_context_path: /NGSI9/registerContext
iota::ngsi::query_context_path: /NGSI10/queryContext
```
This section configures where Orion Context Broker is listening to. This will be in a separate host to IoT Agent.

##### Authorization
```
iota::authorization::trust_token_url: ''
iota::authorization::trust_token_user: iotagent
iota::authorization::trust_token_password: iotagent
iota::authorization::pep_domain: admin_domain
iota::authorization::pep_user: pep
iota::authorization::pep_password: pep
iota::authorization::user_token_url: ''
iota::authorization::user_roles_url: ''
iota::authorization::user_subservices_url: ''
iota::authorization::user_access_control_url: ''
iota::authorization::timeout: 5
```
When IoT Agent requires authentication (with integrated [PEP](pep.md) features), this set of parameters must be filled in with the right values. To disable authentication just leave them as above.

##### Plugins to be installed

```
iota_protocols:
 - iotagentul
 - iotagentmqtt
 - iotagenttt
```
The protocols that IoT Agent can handle in a particular instance can be changed using this section, they can be added or removed. So this means that when deployed, it can install one, two or all three supported protocols. As explained before, each protocol will have a separate module (or HTTP Plugin) in PION. The three protocols supported as of now (IoT Agent v1.0) are: Ultra Light 2.0 (iotagengul), MQTT (iotagentmqtt) and ThinkingThings (iotagenttt).
At least one must be specified.

### 4 Running Puppet Apply.

Final step, once hieradata has been configured accordingly, it's time to run the command for deploying the IoT Agent

```
sudo puppet apply --hiera_config /etc/hiera.yaml --modulepath=/usr/local/iot/puppet/modules/ /usr/local/iot/puppet/manifests/site.pp
```

After running it, puppet will install and configure IoT Agent and MongoDB (2.4.9).
When MQTT protocol is selected, Mosquitto broker will be also installed. You can find out more about how to configure it here [MQTT Agent](modules.md).

### Using MongoDB on a separate instance

IoT Agent relies on Mongodb for internal storage. Hence it's always installed locally. However, if there is an existing MongoDB instance on a different host and there is no need for a local installation, so for this particular scenario, the manifest for your host has to be slightly changed. Just remove the inheritance from 'default' so the manifest stays like this:

```
node 'myhost.domain' {

 class{'iotagent':

 }

}
```
The 'default' node declares the installation of MongoDB, and can be found in ```basenode.pp``` file.


<a name="def-rpms"></a>
## 3. Using RPMs

### Iota rpms installation

Iotagent  rpms can be obtained from iota artifactory:

```
  http://artifactory.hi.inet/artifactory/simple/yum-sbc/iot-agent/x86_64/
```

or view [Build](build.md) section for more information.

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

Binaries will be installated in path  ```/usr/local/iot/bin``` and libraries in path ```/usr/local/iot/lib```


### Mongo db installation

Download mongodb v 2.4 from:

```
https://www.mongodb.org/dl/linux/x86_64
```

You can download for example mongodb-linux-x86_64-2.4.11.tar , and copy it to CentOS 6.5 VM.

Choose an installation path for mongo,  for example :

```
 /usr/local/iot
```

With root privileges:

```
cp mongodb-linux-x86_64-2.4.11.tar   /usr/local/iot
cd /usr/local/iot
tar xvf mongodb-linux-x86_64-2.4.11.tar
```

Create  log  and  data  directories :

```
cd /usr/local/iot/mongodb-linux-x86_64-2.4.11
mkdir ./log
mkdir ./data
cd data
mkdir db
```

You can create a mongod start script

```
cd /usr/local/iot/mongodb-linux-x86_64-2.4.11/

vi start_mongod.sh

/usr/local/iot/mongodb-linux-x86_64-2.4.11/bin/mongod --dbpath /usr/local/iot/mongodb-linux-x86_64-2.4.11/data/db --port 27017 --logpath /usr/local/iot/mongodb-linux-x86_64-2.4.11/log/mongoc.log --pidfilepath /usr/local/iot/mongodb-linux-x86_64-2.4.11/log/mongod.pid --logappend  &
```

Ensure exceution privileges :

```
chmod a+x  start_mongod.sh
```

In order to connect to mongodb:

```
cd /usr/local/iot/mongodb-linux-x86_64-2.4.11/bin

./mongo
```


<a name="def-configuration"></a>
## 4. Iotagent configuration

Iotagent needs a configuration file,  usually named config.json, an example could be:

```
{
    "ngsi_url": {
        "updateContext": "/NGSI10/updateContext",
        "registerContext": "/NGSI9/registerContext",
        "queryContext": "/NGSI10/queryContext"
    },
    "timeout": 10,
    "http_proxy": "192.0.0.2:3128"
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
You can get more information in next sections.

Now you can create an start script, for example  init_iotagent.sh

```
  export LD_LIBRARY_PATH=/usr/local/iot/lib

  /usr/local/iot/bin/iotagent -n qa -i  x.x.x.x -d /usr/local/iot/lib -c ./config.json -v DEBUG  &
```

Replace  x.x.x.x  with  VM IP address. By default  iotagent listen in port 8080



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
<a name="def-logs"></a>
### 5. Logs configuration
Command line to start IoT Agent define the log level (option -v). The folder where log file is generated is defined by _dir_log_ field. The name of log file is _IoTAgent-{name}.log_, where _{name}_ is provided with option -n in command line. When log file reaches 10 MB, the active file is renamed to _IoTAgent-{name}.log.1_. Maximum number of saved files is 5.
You should review [Monitoring alarms](#def-alarms).

<a name="def-timezone"></a>
### 6. Timezone configuration
This information is not used in this version, but it points where database (a file) for timezones is.

<a name="def-proxy"></a>
### 7. HTTP server proxy
In order to send commands to devices in VPN, IoTAgent provides an infrastructure based in _squid_ as HTTP proxy and endpoint of GRE tunnels.

<a name="def-general"></a>
### 8. General configuration
Field _timeout_ is a general timer for operations.


<a name="def-examples"></a>
### 9. Examples

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
                "FileName": "IoTAgent-mqtt"
            }
           }
        ],
        "timezones": "/etc/iot/date_time_zonespec.csv",
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
                "FileName": "IoTAgent-mqtt"
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
## 5. Check Instalation with HTTP request with curls


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
## 6. Monitoring Events and Alarms

Alarms

Alarms is logged in FATAL level, and an alarm means that a communication between an other component is broken.
When the communication is restored other fatal log is written with event=END-ALARM, pay attention, only FATAL logs must be consider, logs in DEBUG level are normal and can't be considered.

Every alarm has a code to identify it.

| code     |description               |
| ------------- |:-------------:           |
| 100   | No comunication with mongo database     |
| 200 | No comunication with Context broker     |

Example of broken communications with mongo database

```
060315T065804,524.785UTC|lvl=FATAL|comp=iota:dev|op=put|[140148919482400:alarm.cc:81] |event=ALARM|code=100|origin= 127.0.0.1:27017|info=socket exception [CONNECT_ERROR] for 127.0.0.1:27017
060315T065804,524.785UTC|lvl=FATAL|comp=iota:dev|op=put|[140148919482400:alarm.cc:81] |event=ALARM|code=100|origin= 127.0.0.1:27017|info=socket exception [CONNECT_ERROR] for 127.0.0.1:27017

...   Connection recovered  ...

060315T123808,396.176UTC|lvl=FATAL|comp=iota:dev|op=remove|[140148573378304:alarm.cc:96] |event=END-ALARM|code=100|origin= 127.0.0.1:27017|info=MongoConnection OK

060315T123808,396.176UTC|lvl=DEBUG|comp=iota:dev|op=remove|[140148573378304:alarm.cc:96] |event=END-ALARM|code=100|origin= 127.0.0.1:27017|info=MongoConnection OK
```

