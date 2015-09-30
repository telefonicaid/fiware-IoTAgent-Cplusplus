# Resource IotAgent MQTT 


#### Index
1. [Introduction](#def-introduction)
2. [Getting started: Installation and Configuration](#def-configuration)
3. [MQTT Conventions](#def-conventions)
4. [IoTAgent API: provision of Services](north_api.md#def-service-api)
5. [IoTAgent API: provision of devices](north_api.md#def-device-api)
6. [Sending single measures MQTT](#def-measures)
7. [Sending multiple measures](#def-multi)
8. [Commands](#def-commands)


<a name="def-introduction"></a>
## 1. Introduction
This protocol is purely oriented to machine-to-machine applications and it’s very lightweight and efficient in terms of bandwidth. It follows a publish-subscribe paradigm, so that sensors will typically publish their information under topics, while listeners will subscribe to such topics to receive data. Therefore there are at least three elements: publisher, subscriber and broker, the latter is the element all other actors connect to in order to either publish or subscribe (as a summary, IoTAgent and devices connect to the MQTT broker). The protocol also features different modes of Quality of Service for message delivering. See <a href="http://whttp://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">MQTT documentation</a> for more information about MQTT.

The IoTAgent for MQTT will accept MQTT 3.1.1 compliant messages that will be translated into "entities" to get published on a Fiware's Orion Context Broker. Please also note that IotAgent-MQTT is connected to two different brokers: the Context Broker (that is part of a IoT Platform) and the MQTT broker which is a separate element. Once connected to the MQTT broker, the IoTAgent will need to subscribe to all topics on that broker in order to be able to receive all messages from any device connected to that broker. As it will be explained in the following sections, there are some conventions that must be followed in order to interact with the IoTAgent. Therefore, __devices have to publish on certain topics with a particular format__. 

#### 1.1 IoTAgent MQTT

With regards to the IoTAgent, the MQTT protocol is another plugin that is loaded by the IoTAgent core if all relevant components are installed and configured properly. There are, however, two kind of dependencies that must be resolved in order to have a fully working MQTT environment. The MQTT plugin (_MqttService.so_ file) also requires several libraries to run: libmosquitto and libmosquittopp (part of Mosquitto project). Those are needed at build time as well, hence their code has been included as third party elements in the code base. However, they have to be installed in the system. 
	The other important dependency is the MQTT broker as mentioned before. The one that has been tested with IoTAgent MQTT is Mosquitto (up to 1.4.4 version). Some Linux distributions may have it as part of their available packages to install, otherwise, it has to be built from source (if IoTAgent is installed from RPMs, Mosquitto must be resolved as RPMs dependencies). The plugin is compatible with any other MQTT broker, however, all the documentation from now on is based on configuring Mosquitto broker. 



<a name="def-configuration"></a>
## 2. Getting started: Installation and Configuration.

As any other IoTAgent protocol plugin, it consists of a shared object, some configuration files and in this case some dependencies. Let's get into deeper detail:

#### MqttService.so
This is the actual plugin and must be placed in the folder where other shared objects (plugins and main components) of the IotAgent are. There are basically two ways of getting the binary file:
##### Build from source.
You can follow more detailed instructions about how to build IoTAgent on this [link](build.md). All what you need to build it is in our repository. 
##### Install from RPM.
RPMs have been tested on Centos 6.5. The RPM follows this pattern: iot-agent-mqtt-[version].[commit].x86_64.rpm. Please note that it will require other components are also installed (like the IotAgent base or libmosquitto and libmosquittopp). 

__Note__: When IoTAgent MQTT is installed from RPMS, the Mosquitto dependencies must be installed from these RPMs that have been tested on CentOS 6.5:

- <a href="http://download.opensuse.org/repositories/home:/oojah:/mqtt/CentOS_CentOS-6/x86_64/libmosquitto1-1.4.4-1.1.x86_64.rpm">libmosquitto1-1.4.4-1.1.x86_64.rpm</a> (MQTT C Client)
- <a href="http://download.opensuse.org/repositories/home:/oojah:/mqtt/CentOS_CentOS-6/x86_64/libmosquittopp1-1.4.4-1.1.x86_64.rpm">libmosquittopp1-1.4.4-1.1.x86_64.rpm</a> (C++ Wrapper for client)
- <a href="http://download.opensuse.org/repositories/home:/oojah:/mqtt/CentOS_CentOS-6/x86_64/mosquitto-1.4.4-1.1.x86_64.rpm">mosquitto-1.4.4-1.1.x86_64.rpm</a> (MQTT Broker)


#### Libmosquitto and Libmosquittopp
These two libraries are requiered by MqttService.so. You have to use the RPMs provided above if you are installing IoTAgent from RPMs (otherwise, dependencies will not be resolved). However, if you are installing the IoTAgent from source, then you can build the Mosquitto dependency from sources <a href="http://mosquitto.org/download/">here</a> (you will find the whole Mosquitto package, including broker and client libraries). 

#### Mosquitto MQTT Broker.
When your infrastructure features a different MQTT broker, you could use that instead, (see next section). The use of Mosquitto allows us to configure a very basic but effective way of preventing devices of a particular service from publishing/subscribing to topics of other services, which can be problematic in a multi-vendor scenario. This is done by means of its ACL capabilities. If for whatever reason this convenient feature does not suit your needs, it can be turned off (any device could publish/subscribe to any topic). 

##### Mosquitto Broker config files:

- mosquitto.conf (part of Mosquitto MQTT Broker. It can be used to fine tune Mosquitto). 
- aclfile.mqtt (file used for the Access Control in Mosquitto)

__Note:__ If MQTT plugin is installed using our RPM, these two files will be provided with default configuration for IoTAgent MQTT (this includes enabling ACL). These files are placed in /etc/iot/ directory. When Mosquitto is installed in the system, it will come along with its own config files. If you want to use ACL capabilities, we recommend you use our _mosquitto.conf_ file instead. 

##### Configuring ACL in Mosquitto.

The Access Control List of Mosquitto enables certain level of privacy for MQTT messages. Anyone could publish on the MQTT Broker, and without ACL, anyone could publish on any topic. This might be fine for you, but if you prefer to have some control over who can publish what and where, you should use ACL. The official Mosquitto documentation explains this topic in more detail. The default pattern that we provide requires MQTT publishers to identify themselves (although password is not necessary) using an user id. This user-id has to be the "api-key" provisioned in the system (see following sections). By using the Api-key as user-id and the ACL, you can have many different organizations with different devices publishing on topics making sure they can't see each other. 
ACL is enabled in the mosquitto.conf file by setting the "aclfile" property to the path where "aclfile.mqtt" file is located. 
By default is __enabled__ only if you installed our MQTT plugin RPM.

__Important:__ when ACL is enabled, the user id must be included in the MQTT connection. The MQTT Plugin takes the user from the _sensormqtt.xml_ file. Any device wanting to subscribe/publish to a MQTT broker with ACL enabled has to provide the apikey as user-id. How to do it may depend on the implementation, for instance, the Mosquitto clients (mosquitto_sub and mosquitto_pub) utilize the "-u" parameter. 

##### Running Mosquitto.

Although the MQTT Plugin features reconnection to MQTT Broker, its advisable that this is running before starting the IoTAgent at all. So the command to start Mosquitto as a daemon is:
```
mosquitto -d -c <path to config file>
```
- When installed from our RPM: typically, the path to config file will be _/etc/iot/mosquitto.conf_. ACL will be enabled.
- When built from source: the path is _/etc/mosquitto.conf_ or wherever the mosquitto.conf coming with Mosquitto is placed. Alternatively, you could use this file if ACL is not required. 

__Note:__ ideally, Mosquitto broker should be always running in background, so you can check with your system administrator how to do so, using an init.d service for instance. 

#### Configuration Files.
The basic configuration file for the IoTAgent is the config.json. Here you have to explicitly state what plugins are going to be deployed. There are also other important files to configure, so let's get into each one.

##### Config.json

The "resources" section of this file controls what plugins will be running on that instance of the IoTAgent. So if MQTT is going to be used, at least this resource must be present (there might be different plugins as well):

    {
     "resource": "/iot/mqtt",
     "options": {
        "ConfigFile": "/etc/iot/MqttService.xml",
        "FileName": "MqttService"
       }
     } 

Where "MqttService" is the name of the plugin, and it's equivalent to the file that will be loaded by IoTAgent, that is to say: __MqttService.so__.

"ConfigFile" will be used by the MQTT plugin to locate other configurations file that are also needed. 

##### Additional Configuration needed by MQTT Plugin.

In this example, let's assume all files are installed in /etc/iot/ directory. So the files that must be present for a proper operation of the IoTAgent with MQTT plugin are:

- MqttService.xml (it contains the path to the following file).
- sensormqtt.xml (it contains low-level details about the MQTT plugin's behaviour. The only recommended information that can be changed by users is where MQTT broker (Mosquitto or equivalent) is listening).


##### Using a different MQTT broker.

If Mosquitto is not prefered, or it's going to run in a separate machine within your infrastructure, some changes have to be done to __sensormqtt.xml__ file. Locate the following two lines in such file:
         
         <input type="mqtt" mode="server" publish="apikey/sensorid/type" subscribe="" host="localhost" port="1883" user="admin" password="1234" name="mqttwriter"/>				
         <input type="mqtt" mode="server" publish="apikey/sensorid/type" subscribe="#" host="localhost" port="1883" user="admin" password="1234" name="mqttrunner"/>
         
Where "host" and "port" will be set accordingly. If a different MQTT broker is used, the files "aclfile.mqtt" and "mosquitto.conf" are no longer needed. If Mosquitto is still used, those files must be where Mosquitto is running. 


<a name="def-conventions"></a>
## 3. MQTT Conventions.

MQTT does not define any particular hierarchy for topics. However, IoTAgent for MQTT needs devices to follow a set of particular conventions in order to be able to handle connections from devices belonging to different organizations. Thus, with the following topic conventions, devices will publish information under different "services" that can identify a vendor or any organization who wants to separate its devices from others. This separation has been implemented following a topic structure. Regarding payload, IoTAgent is somewhat agnostic about how devices send data when one message carries plain or raw information. However, for more sophisticated cases like multiple measures per mqtt message or commands, there will be specific formats.

Prior a device can successfully publish information on the IoTAgent, a service and the device must be provisioned (see [API for services](north_api.md#def-service-api) and [API for devices](north_api.md#def-device-api)). In this case, the api-key attribute has to be supplied, as it's part of the topic structure and therefore, it will be used to filter messages. 

The structure has three (or four) different topics, being the last one used when working with commands. Topic structure:

	<api-key>/<device-id>/<type>/[cmd-name]

Where:
''api-key'': this is a unique value per service. It is provided through the provisioning API.
''device-id'': this is typically a sensor id, it has to be unique per “api-key”.  
''type'':  can take different values, it can be the actual phenomenon being measured, for example: temperature, pressure, etc... This is the name of the attribute.
There are, however, some keywords already used for different scenarios:
* "mul20": for devices sending multiple measures in one single mqtt message (special payload is also required).
* "cmdget" : for devices requesting available commands stored in the IoTAgent.
* "cmdexe" : for devices sending the result of the command execution. It will also have another topic after this to indicate the command name [cmd-name].
* "cmd" : this is what the IoTAgent will use when publishing commands to devices. Another topic with command name [cmd-name] will follow this one. 


<a name="def-measures"></a>
## 6. Devices sending single Measures.
This is the simplest and more straightforward scenario. Devices (once provisioned under a service) can publish MQTT messages to the IoTAgent. Those messages contain one piece of information each. That means that one message will be translated into one single entity on the ContexBroker domain. The information can be typically sensors' measures. 

This is the topic hierarchy that has to be used by devices:

	<api-key>/<device-id>/<type>

Where:
''api-key'': this is a unique value per service. It is provided through the provisioning API.
''device-id'': this is typically a sensor id, it has to be unique per “api-key”.  
''type'': it is the actual phenomenon being measured, for example: temperature, pressure, etc… this is the name of the attribute being published on ContextBroker.
An example would be:

	822asijn7jwb9kn367fjz235/id234/temperature

The payload can just be a number representing the temperature. The payload will be directly what comes into "value" of the published entity on ContextBroker. 

This is what is publised on ContextBroker: 

           {
                "id":"id234",
                "type" : "<entity-type>",
                "isPattern" : "false",
                "attributes" : [
                               {
                                	"name":"temperature",
                                        "type":"string",
                                        "value":"33",
                                         "metadatas":[
                                               {
                                                 "name":"TimeInstant",
						 "type":"ISO8601",
						  "value":"2015-03-19T14:39:14.924961Z"
                                               }]
                               },
			       {
				 "name":"TimeInstant",
				 "type":"ISO8601",
				  "value":"2015-03-19T14:39:14.924961Z"
				}]
	   }

This payload (value) follows [Ultra Light](UL20_protocol.md) format for value. You could use mapping features to send a location to Context Broker [Conversion to location](modules.md).

<a name="def-multi"></a>
## 7. Devices sending multiple measures per message

Another scenario can happen when devices send more than one phenomena within the payload.  That is to say, one single MQTT message carries all measures. When it comes to ContextBroker, there will be one entity publication (per device) but containing all different attributes as per measures included in the mqtt message (each phenomenon or measure will be a separate attribute). In order to be able to parse the information on the IoTAgent, devices should follow this format:

Topic:

	<api-key>/<device-id>/mul20

Where:
''api-key'': this is a unique value per service.
''device-id'': this is typically a sensor id, it has to be unique per “api-key”.
''mul20'': this is literal, it must be “mul20” to differentiate from other messages as the payload parsing will also be different. 

Payload:
	As it will carry more than one measure, it has to contain enough information to identify which phenomenon and measure are sent. 
The rules are:
Observations must be separated by this character ‘#’.
Observations are composed of name and value, separated by ‘|’. 
Name of observation (phenomenon) is what is going to be pu, so that XML Config file contains proper parsing definition.
Last character has to be ‘#’.
Example for the payload:
	temperature|33#pressure|55#

Obtained values are:
temperature = 33
pressure = 55

Example, Topic and payload:

Topic: 		822asijn7jwb9kn367fjz235/id234/mul20
payload: 	temperature|33#pressure|55#

Therefore, the entity published on ContextBroker will contain two attributes with name "temperature" and "pressure" with values "33" and "55" respectively. 
	
	{
        	        "id":"id234",
                	"type" : "<entity-type>",
                	"isPattern" : "false",
                	"attributes" : [
                               	{
                                	"name":"temperature",
                                        "type":"string",
                                        "value":"33",
                                         "metadatas":[
                                               {
                                                 "name":"TimeInstant",
						 "type":"ISO8601",
						  "value":"2015-03-19T14:39:14.924961Z"
                                               }]
                               },
			       {	
                                	"name":"pressure",
                                        "type":"string",
                                        "value":"55",
                                         "metadatas":[
                                               {
                                                 "name":"TimeInstant",
						 "type":"ISO8601",
						  "value":"2015-03-19T14:39:14.924961Z"
                                               }]
                               },
			       {
				 "name":"TimeInstant",
				 "type":"ISO8601",
				  "value":"2015-03-19T14:39:14.924961Z"
				}]
	   }

You could review [Ultra Light](UL20_protocol.md) for more information about this feature.

<a name="def-commands"></a>
## 8. Commands.

As per it's described in [IoTAgent Commands](commands.md), most protocols support two types of commands depending on what actor is requesting the sending of the commands (push and pull). However, the MQTT protocol works in an asynchrnous fashion, so it makes more sense to have only PUSH commands (the IoTAgent will publish a MQTT message with the command as soon as it's received, while the opposite behaviour, pull commands, it's the device the one that has to ask for commands and then the IoTAgent would publish the MQTT message). When sending a command, the IoTAgent will publish a mqtt message on a particular topic, and as it was described previously it will follow the "api-key"/"device-id"/cmd/"command-name" pattern. The keyword "cmd" is fixed. That is to say, devices have to subscribe to topics following that pattern: <api-key>/<device-id>/cmd/+  (as defined in the mqtt protocol, the '+' character means any topic on that level) where "api-key" and "device-id" are the identifiers for service and device provisioned. With PUSH commands, devices have to just subscribe to that particular topic and "wait" for commands. It means they are connected all the time, as a reward, they don't need to ask for new commands.
 

The IoTAgent, will then publish all available (not expired) commands for that device. 
### Push Commands. 

Devices don't need to do anything else apart from subscribing to the correct topic. IoTAgent will publish the MQTT message representing the command as soon as the command request is received from the ContextBroker. 
For example a command will be a mqtt message with this topic: 

	822asijn7jwb9kn367fjz235/id234/cmd/set_time

Note: devices must subscribe to that topic pattern only, otherwise they may receive commands addressed to other devices, causing issues. So for this example, the device must be subscribed to: 822asijn7jwb9kn367fjz235/id234/cmd/+    (including ‘+’). 


''Note'': devices have to be connected and subscribed to the mentioned topic in order to receive a command. Depending on how they interact with the broker, they may get messages delivered to them when they weren't connected. For doing this, they have to set the flag "clean-session" to false. Also note that commands will expire after some time in IoTAgent internal storage. 

#### Payload format for commands.
	
Information within payload must follow same format used in multi-measures. Command parameters will have name and value that will be separated by “|” and each parameter will be separated by “#”. There is one critical piece of information in the payload, this is the command id. That has to be returned to the IoTAgent in the response of the command using the previous format. 

Example of command payload:
	
	cmdid|82ndsj28924hnsrh2932424#param1|value1#

Where an additional “param1” parameter has been included with “value1” value. 

#### Sending responses to commands. 

For doing so, devices have to publish an MQTT message on the following topic, and the payload must include the "cmdid" previously sent as explained.

Topic:

	<api-key>/<device-id>/cmdexe/<cmd-name>

Where:
api-key: this is a unique value per vendor.
device-id: this is typically a sensor id, it has to be unique per “api-key”. 
cmdexe: this is literal, it must be “cmdexe” to indicate it is the execution (result) of the command.
cmd-name: it is the name of the command, and it has to be the same as received by subscription. 

Payload:

As it has been highlighted, cmdid must be present in the payload of any response to commands. It has to follow the format already described. Additional information can be included as well, like parameters, results, messages, etc as long as they follow the aforementioned format (‘|’ and ‘#’ separators).

Example (response to previous “set_time” command):

Topic:
	
	822asijn7jwb9kn367fjz235/id234/cmdexe/set_time
Payload:
	
	cmdid|82ndsj28924hnsrh2932424#result|OK#

In this example a specific result has been added apart from the cmdid previously sent. 



