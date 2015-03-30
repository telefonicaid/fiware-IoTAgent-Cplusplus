# Build and Install

## Building IoT Agents

These IoT Agents are successfully built in CentOS 6.5 distribution.

### Support to build dependencies

Pion Network Library recommends as log library *log4cplus*.
Common dependencies when you build these IoT Agents:
- boost: 1.55.0 (http://www.boost.org/)
- Pion Network Library: 5.0.6 (http://pion.sourceforge.net/) (depends on boost and log4cplus).
- MongoDB driver: legacy-1.0.0 (http://docs.mongodb.org/ecosystem/drivers/cpp/)
- rapidjson: 0.11 (https://code.google.com/p/rapidjson/downloads/list)
- log4cplus: 1.1.3-rc4 (http://sourceforge.net/p/log4cplus/wiki/Home/)
- cppunit: 1.12.1 (for unit test) (http://sourceforge.net/projects/cppunit/)
- gmock: 1.7.0 (for unit test) (https://code.google.com/p/googlemock/)
- libvariant: 1.0.1 (for json schema validation) (https://bitbucket.org/gallen/libvariant/)

Other dependencies only applied to specific IoT Agent.

We are built most dependencies from source. You might need to visit dependency web site in order to build such dependenies.

#### boost
It provides a script `bootstrap.sh` to build them.
#### log4cplus
The log format used in IoTAgent writes function name. This functionality is provided by log4cplus when it finds a macro: \_\_FUNCTION\_\_ and \_\_PRETTY\_FUNCTION\_\_. You need disable \_\_PRETTY\_FUNCTION\_\_ macro modifying _configure_ script, **removing** these lines:

```
{ $as_echo "$as_me:${as_lineno-$LINENO}: checking for __PRETTY_FUNCTION__ macro" >&5
$as_echo_n "checking for __PRETTY_FUNCTION__ macro... " >&6; }
if ${ac_cv_have___pretty_function___macro+:} false; then :
  $as_echo_n "(cached) " >&6
else

  cat confdefs.h - <<_ACEOF >conftest.$ac_ext
/* end confdefs.h.  */

int
main ()
{
        char const * func = __PRETTY_FUNCTION__;
  return 0;
}
_ACEOF
if ac_fn_cxx_try_compile "$LINENO"; then :
  ac_cv_have___pretty_function___macro=yes
else
  ac_cv_have___pretty_function___macro=no
fi
rm -f core conftest.err conftest.$ac_objext conftest.$ac_ext

fi
{ $as_echo "$as_me:${as_lineno-$LINENO}: result: $ac_cv_have___pretty_function___macro" >&5
$as_echo "$ac_cv_have___pretty_function___macro" >&6; }

if test "x$ac_cv_have___pretty_function___macro" = "xyes"; then :
  $as_echo "#define LOG4CPLUS_HAVE_PRETTY_FUNCTION_MACRO 1" >>confdefs.h

fi
```
After that, you need execute something like that from directory where source is(in this example, we install log4cplus in directory /home/develop/iot/log4cplus):
```
./configure --enable-release-version=yes --enable-so-version=no --prefix=/home/develop/iot/log4cplus113
```
With this command, you get a shared library named _liblog4cplus-1.1.so_.

#### Pion
From directory where Pion source is located, you need execute something like that (if you execute _configure --help_, you get all options to build Pion):
```
./configure --with-boost=/home/develop/iot/boost_1_55_0 --with-log4cplus=/home/develop/iot/log4cplus113 --without-bzlib --prefix=/home/develop/iot/pion506
```
Pion will be installed (in this example) in /home/develop/iot/pion506.

#### MongoDB

Instuctions for download and compile  the C++ driver for mongodb (http://docs.mongodb.org/ecosystem/drivers/cpp/).

repository  https://github.com/mongodb/mongo-cxx-driver

To get a repository that you can build, you can clone the sources.

git clone https://github.com/mongodb/mongo-cxx-driver.git

on 28 Jan legacy-1.0.0 was the offical branch.

The driver is compiled with these options (note boost path):

scons --prefix="/home/develop/iot/mongo-cxx-driver" --extrapath=/home/develop/iot/boost_1_55_0/ --sharedclient  install

And the result is the static and dynamic library (libmongoclient.so)

#### LibVariant

Instuctions for download and compile https://bitbucket.org/gallen/libvariant/

```
wget https://bitbucket.org/gallen/libvariant/get/055488b93468.zip
unzip 055488b93468.zip
cd gallen-libvariant-055488b9346
cmake .
make
```

as a result we obtain  ./src/libVariant.a


### Configure build environment
To build IoT Agents you need cmake 2.8.12 (minimum version). In order to find dependencies following environment variables can be used:
- BOOST_ROOT: path where *boost* are installed.
- PION_ROOT: path where *Pion Network Library* is installed.
- LOG4CPLUS_ROOT: where *log4cplus* is installed.
- MONGODB_ROOT: where *mongodb cplusplus driver* is installed.

Take a look to cmake directory. You can find there cmake scripts to find dependencies. Dependencies are not searched in default directories. You can set IOTAGENT_DEPENDENCIES_ROOT in CMakeList.txt in order to set where cmake looks for dependencies ++(cmake includes a module to look for boost libraries. You need to use environment variable to help cmake to find boost)++. For example:

`set(IOTAGENT_DEPENDENCIES_ROOT /home/develop/iot/pion-5.0.6
                                /home/develop/iot/log4cplus
                                /home/develop/iot/mongo-cxx-driver
                                /home/develop/iot/json
                                /home/develop/iot/cppunit/cppunit-1.12.1
                                /home/develop/iot/gmock-1.7.0
                                /home/develop/iot/libVariant
                                /home/develop/iot/mosquitto-1.3.5)`

If you need modify the behavior of build environment, you can do it modifying CMakeLists.txt file.

### Build
We use out-of-source building, so you must build out of source directory.
The directory where source is downloaded will be called _SourcePath_. From this directory let'us make a directory build/*build_type* (*build_type* must be Debug, Release or DebugCoverage) and execute (in this example we use Debug as *build_type* and you must replace <path-to-boost> with path to your boost installation):

```
$ pwd
$ /home/develop/dca-core
$ mkdir -p build/Debug
$ cd build/Debug
$ cmake -DGIT_VERSION=<version> -DGIT_COMMIT=<release> -DBOOST_ROOT=<path-to-boost> -DCMAKE_BUILD_TYPE=Debug ../../
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
- iot-agent-puppet-[version]-[release].x86_64.rpm (view [Deploy](deploy.md) section in order to know how build you own puppet recipes).
- iot-agent-base-[version]-[release].x86_64.rpm: IoT Agent core.
- iot-agent-ul-[version]-[release].x86_64.rpm: IoT Agent Ultra-Light module.
- iot-agent-mqtt-[version]-[release].x86_64.rpm: IoT Agent MQTT module.
- iot-agent-tt-[version]-[release].x86_64.rpm: IoT Agent Thinking Things module.

### Documentation

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

Examples:

         iotagent -p 80 -n mqtt -c /etc/iot/config.json

Port is 80 (modifying the default port), log file is IoTAgent-mqtt.log, url for this module is /iot/mqtt (root url is /iot).
Configuration as iotagent is in /etc/iot/config.json file.

         iotagent -p 80 -n mqtt -v ERROR -c /etc/iot/config.json

It defines a log level to ERROR.

## Making your module-based iotagent
[Sample Module](test_service.md)
