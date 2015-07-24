# Build and Install

## Building IoT Agents

These IoT Agents are successfully built in CentOS 6.5 distribution.

### Support to build dependencies

Some dependencies are included in _third_party_ directory in order to make easy this building process.
CMakeLists.txt add external projects in order to download and build following dependencies:

- boost: 1.55.0 (recommended) (http://www.boost.org/)
- Pion Network Library: 5.0.6 (http://pion.sourceforge.net/) (depends on boost and log4cplus).
- MongoDB driver: legacy-1.0.1 (http://docs.mongodb.org/ecosystem/drivers/cpp/)
- log4cplus: 1.1.3-rc4 (http://sourceforge.net/p/log4cplus/wiki/Home/)
- cppunit 1.12.1 (http://sourceforge.net/projects/cppunit) only for unit test.

#####IMPORTANT
If you want to  modify the URL from dependencies are downloaded, you must modify CMakeLists.txt

*Note*: our CMakeLists.txt requires `cmake 2.8.12` or higher to build the project. 

IoT Agents need MongoDB as persistent storage. Be sure that MongoDB is started in your host in order to execute unit tests.


### Get the source 

Run: 
```
git clone https://github.com/telefonicaid/fiware-IoTAgent-Cplusplus.git
```



### Configure build environment
To build IoT Agents you need cmake 2.8.12 (minimum version). If you want to build our MQTT module you must add -DMQTT=ON to command line.


### Build
We use out-of-source building, so you must build out of source directory.
The directory where source is downloaded will be called _SourcePath_. From this directory let'us make a directory build/*build_type* (*build_type* must be Debug, Release or DebugCoverage) and execute (in this example we use Debug as *build_type* and you must replace <path-to-boost> with path to your boost installation):
(target install only copy built libraries in a directory to concentrate them).

```
$ pwd
$ /home/develop/dca-core
$ mkdir -p build/Debug
$ cd build/Debug
$ cmake -DGIT_VERSION=<version> -DGIT_COMMIT=<release> -DMQTT=ON -DCMAKE_BUILD_TYPE=Debug ../../
$ make check install
```
If you want use environment variables add -D option:
`-DPION_ROOT=<path-to-pion>`
The target _check_ execute unit tests and target _install_ copies libraries in _SourcePath_/lib/*build_type* and executable in _SourcePath_/bin/*build_type*. For example, for _DebugCoverage_ target:

```
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/bin/DebugCoverage/iotagent
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libiota.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libiota.a

-- Up-to-date: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libEsp.a
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/MqttService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libMqttService.a

-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libMqttService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/TTService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libTTService.a
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libTTService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/RepsolService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libRepsolService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/UL20Service.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libUL20Service.a
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libUL20Service.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/TestService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libTestService.a
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libTestService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/EvadtsService.so
-- Installing: /home/develop/JENKINS/workspace/Rebuild_iotagent/lib/DebugCoverage/libEvadtsService.so
```

You must know that shared objects without prefix _lib_ are modules. These modules are loaded by IoT Agent based on PION as _plugins_ implementing protocols.
If you need _rpms_ execute `make package` in order to build:
- iot-agent-base-[version]-[release].x86_64.rpm: IoT Agent core.
- iot-agent-ul-[version]-[release].x86_64.rpm: IoT Agent Ultra-Light module.
- iot-agent-mqtt-[version]-[release].x86_64.rpm: IoT Agent MQTT module.
- iot-agent-tt-[version]-[release].x86_64.rpm: IoT Agent Thinking Things module.

Some dependencies are include in rpm (boost, pion, log library).

An unit test called _testPUSHCommandProxyAndOutgoingRoute_ needs tinyproxy in port 8888 (this test checks sending command through http proxy).
 

### Documentation
TBC.
If you have doxygen installed, you can generate html documentation of the libvariant public interface from the source code with:

```
$ cd build/Debug
$ cmake -DGIT_VERSION=<version> -DGIT_COMMIT=<release> -DBOOST_ROOT=<path-to-boost> -DCMAKE_BUILD_TYPE=Debug ../../
$ make doc
```

The output html is in build/Debug/doc

### How to start agents ###

The general procedure to start an IoT Agent needs needs that libraries generated or
deployed can be loaded by executable (LD_LIBRARY_PATH environment variable). The executable is named _iotagent_.


You can start an iotagent by following procedure:

         iotagent -i IP [OPTIONS (except -o)] -c SERVICE_CONFIG_FILE

This way starts all RESOURCE configured in CONFIG_FILE.

         options:  [-p PORT] [-u URL_BASE] [-n IOTAGENT_NAME] [-d PLUGINS_DIR] [-v LOG_LEVEL]

* -i IP: It defines the ip address where server is started (Default is 0.0.0.0). If no provided we cannot register for commands. Thus, this parameter is recommended to provide, any way.
* -p PORT: It defines the port where server is listening (Default is 8080)
* -u URL_BASE: It defines a root url (Default is _/iot_)
* -n IOTAGENT_NAME: The name assigned. This name is used as part of log file (IoTAgent-<name>.log)
* -d PLUGINS_DIR: You can specify a directory where modules are located.
* -v LOG_LEVEL: log level (DEBUG, INFO, WARNING, ERROR, FATAL).
* -ipv4 server is started in 0.0.0.0
* -ipv6 server is started in [::]

Examples:

         iotagent -p 80 -n mqtt -c /etc/iot/config.json

Port is 80 (modifying the default port), log file is IoTAgent-mqtt.log, url for this module is /iot/mqtt (root url is /iot).
Configuration as iotagent is in /etc/iot/config.json file.

         iotagent -p 80 -n mqtt -v ERROR -c /etc/iot/config.json

It defines a log level to ERROR.

To check iotagent, you can use curl

         curl -g -X GET http://127.0.0.1:80/iot/about 
the response

         Welcome to IoTAgents 1.1.0 commit 134.g5b92683 in Jul 23 2015

if you start with -ipv6 you can use

        curl -g -X GET http://[::1]:80/iot/about 

## Making your module-based iotagent
[Sample Module](test_service.md)
