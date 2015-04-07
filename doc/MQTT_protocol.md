# Resource IotAgent MQTT 


#### Index
1. [Introduction](#def-introduction)
2. [MQTT Conventions](#def-conventions)
3. [IoTAgent API: provision of Services](north_api.md#def-service-api)
4. [IoTAgent API: provision of devices](north_api.md#def-device-api)
5. [Sending single measures MQTT](#def-measures)
6. [Sending multiple measures](#def-multi)
7. [Commands](#def-commands)


<a name="def-introduction"></a>
## 1. Introduction
This protocol is purely oriented to machine-to-machine applications and it’s very lightweight and efficient in terms of bandwidth. It follows a publish-subscribe paradigm, so that sensors will typically publish their information under topics, while listeners will subscribe to such topics to receive data. Therefore there are at least three elements: publisher, subscriber and broker, the latter is the element all other actors connect to in order to either publish or subscribe (as a summary, IoTAgent and devices connect to the MQTT broker). The protocol also features different modes of Quality of Service for message delivering. See <a href="http://whttp://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html">MQTT documentation</a> for more information about MQTT.

The IoTAgent for MQTT will accept MQTT 3.1.1 compliant messages that will be translated into "entities" to get published on a Fiware's Orion Context Broker. Please also note that IotAgent-MQTT is connected to two different brokers: the Context Broker (that is part of a IoT Platform) and the MQTT broker which is a separate element. Once connected to the MQTT broker, the IoTAgent will need to subscribe to all topics on that broker in order to be able to receive all messages from any device connected to that broker. As it will be explained in the following sections, there are some conventions that must be followed in order to interact with the IoTAgent. Therefore, devices have to publish on certain topics with a particular format. 

<a name="def-conventions"></a>
## 2. MQTT Conventions.

MQTT does not define any particular hierarchy for topics. However, IoTAgent for MQTT needs devices to follow a set of particular conventions in order to be able to handle connections from devices belonging to different organizations. Thus, with the following topic conventions, devices will publish information under different "services" that can identify a vendor or any organization who wants to separate its devices from others. This separation has been implemented following a topic structure. Regarding payload, IoTAgent is somewhat agnostic about how devices send data when one message carries plain or raw information. However, for more sophisticated cases like multiple measures per mqtt message or commands, there will be specific formats.

Prior a device can successfully publish information on the IoTAgent, a service and the device must be provisioned (see (north_api.md#def-service-api) and (north_api.md#def-device-api)). In this case, the api-key attribute has to be supplied, as it's part of the topic structure and therefore, it will be used to filter messages. 

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
## 5 Devices sending single Measures.
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


<a name="def-multi"></a>
## 6 Devices sending multiple measures per message

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

<a name="def-commands"></a>
## 7. Commands.

As per it's described in [IoTAgent Commands](commands.md), there are two kind of commands depending on what actor is requesting the sending of the commands. The IoTAgent for MQTT supports both. When sending a command, the IoTAgent will publish a mqtt message on a particular topic, and as it was described previously it will follow the "api-key"/"device-id"/cmd/"command-name" pattern. The keyword "cmd" is fixed. That is to say, devices have to subscribe to topics following that pattern: <api-key>/<device-id>/cmd/+  (as defined in the mqtt protocol, the '+' character means any topic on that level) where "api-key" and "device-id" are the identifiers for service and device provisioned. 

### Polling commands. 

For this kind of commands, devices are the ones to request the sending of any available command by polling the IoTAgent. To do so they have to publish a message on this topic:

	<api-key>/<device-id>/cmdget

Payload is not relevant (it can be null)

The IoTAgent, will then publish all available (not expired) commands for that device. For example a command will be a mqtt message with this topic: 

	822asijn7jwb9kn367fjz235/id234/cmd/set_time

Note: devices must subscribe to that topic pattern only, otherwise they may receive commands addressed to other devices, causing issues. So for this example, the device must be subscribed to: 822asijn7jwb9kn367fjz235/id234/cmd/+    (including ‘+’). 

### Push Commands. 

In this case, devices don't need to do anything else apart from subscribing to the correct topic. IoTAgent will publish the MQTT message representing the command as soon as the command request is received from the ContextBroker. So the message will look exactly the same as the one explained. 

''Note'': devices have to be connected and subscribed to the mentioned topic in order to receive a command. Depending on how they interact with the broker, they may get messages delivered to them when they weren't connected. For doing this, they have to set the flag "clean-session" to false. Commands can expire after some time in IoTAgent storage, so if devices connect periodically to send measures (for instance) the expiration period must be set to be greater than the time between connections. 

#### Payload format for commands.
	
Regardless of the type of command (push or polling) information within payload must follow same format used in multi-measures. Command parameters will have name and value that will be separated by “|” and each parameter will be separated by “#”. There is one critical piece of information in the payload, this is the command id. That has to be returned to the IoTAgent in the response of the command using the previous format. 

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



