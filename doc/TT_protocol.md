# Resource IotAgent Thinking Things 


#### Index
1. [Introduction](#def-introduction)
2. [IoTAgent API: provision of Services](north_api.md#def-service-api)
3. [IoTAgent API: provision of devices](north_api.md#def-device-api)
4. [Sending measures (TT Open protocol)](#def-measures)
5. [Supported TT Modules](#def-modules)
6. [Changing generic configuration](#def-configuration)


<a name="def-introduction"></a>
## 1. Introduction
Thinking Things is an open Internet of Things system composed by modules. In Thinking Thing you combine different plastic "cubes" to form a “stack”. Every stack has a Core module that communicates with the Internet through the mobile network. Every module gets information and sends information according to its characteristics (for example, temperature, location, etc...)
Thinking Things has its own back-end which is widely documented here: (<a href="http://www.thinkingthings.telefonica.com/support/#api">API documentation</a>). However, ThinkingThing devices (or TT devices) can also get connected to IoT Platform by means of an IoTAgent. For the devices, there has to be no difference between their natural backend and the IoTAgent. 

What the IoTAgent for Thinking Things is going to do is receive HTTP requests from TT Devices and publish those in NGSI-compatible format on the ContextBroker. This document is aimed at developers wanting to use TT devices within the IoT Platform realm, as it describes the particularities of the IoTAgent for Thinking Things including the entities format that are published on ContextBroker. 

Prior any TT device can connect to the IoTAgent, it has to be correctly provisioned through the IoTAgent API. And in order to do that, you have to be given some credentials (the trust token) as operations with ContextBroker are authenticated. All your devices will be associated to a sub-service so that only you have access to them. That sub-service is also part of a greater service on ContextBroker, so that is one of the pieces of information you need to provide in order to provisioning your devices. 
The service is represented by this header:

	Fiware-Service

And the sub-service by this one:

	Fiware-ServicePath

Please refer to this section to find out more about how to work with services: [IoTAgent API: provision of Services](north_api.md#def-service-api)
''Note'': The API exposed by the IoTAgent is somewhat generic and used with other protocols different to Thinking Things, that's why some of the fields don't apply to TT. For instance, 'apikey' has to be left empty as the services hierarchy is provided by service/sub-service relationship only. 

Next thing to do is provisioning the device (this assumes you already have provisioned a sub-service). The device id needed on the API is going to be your "stack-id" that should be written on your device or given by your Thinking Things supplier. Devices-id must be unique for a given sub-service. Please refer to [IoTAgent API: provision of devices](north_api.md#def-device-api) for more specific information on how devices API works. 
Please also note that "commands" on Thinking Things work differently to other IoTAgent supported protocols, so effectively you don't need to provide them. This section will describe this feature [Changing generic configuration](#def-configuration)

Finally, it's important to note that devices must post their requests to the URL where the IoTAgent is listening to. Contact your Thinking Things supplier in order to find out how to do it with your devices. 


<a name="def-measures"></a>
## 4. Sending measures (TT Open protocol)

TT Open protocol is the evolution of TT Close protocol in Thinking Things. The new version allows to send custom (or generic) measures and also enables the reception of configuration fields on the TT Device. This section will explain what kind of modules are supported by IoTAgent TT and how they will be published on ContextBroker. 

### TT Open protocol, frame format.
TT devices will send to IoTAgent URL a HTTP POST request where the body has this format:

    cadena=<TTOpen URL encoded frame>

And TTOpen Frame follows this pattern:

    #<stack-id>,#<bus-id>,<module-specific fields>

Where there might be any number of #<bus-id>,<module-specific fields> section (comma separated). As explained before, stack-id is given with any TT device. "bus-id" is an internal logical representation where modules are connected (0, 1, 2, etc...)

The section <module-specific fields> will depend on the type of module or if it's a generic measure or configuration. Some modules from TT Close are supported in this implementation of TT Open. See below all supported modules and the kind of response they can send back to your TT Device.

Example of a TT Open frame (not URL encoded for clarity), including some TT Close modules as well:

     cadena=#85552223334444,#0,GC,config_time,34,#0,GM,pos,33.000/-3.234234,#0,P1,214,07,33f,633c,#0,K1,0$

This frame is handled by IoTAgent in this way:

     device-id (aka stack-id): 85552223334444
     Module GC (Generic Configuration): config_time
     Value GC: 34

     Module GM (Generic Measure): pos
     value GM: 33.000/-3.234234

     Module P1 (GSM cell information)
     Value: 214,07,33f,633c

     Module K1 (Core system)
     Value: N/A
     SleepTime: 0
     SleepCondition: N/A

### TTOpen protocol, response format.
As TT Devices use a HTTP request, they will expect some kind of response in the body. The device will deal with it in a module-basis, which means that each module will interpret its part of the response. The following section will describe how responses are built for individual modules. The general format is like this:

     #<bus-id>,<module-specific response fields>,#<bus-id>,<module-specific response fields>, (...)

It's a comma-separated array of individual responses to modules. There is also no need for including the stack-id. 

Responses can have three different meanings for modules:

#### 1. Measure was correct and processed with no issue.
In this case all modules will receive their expected response as it will explained below.

#### 2. There was an error with a particular module.
If a module is malfunctioning and sending incorrect module information, that section will be ignored (and not published on ContextBroker) and no response will be included for that particular device.

#### 3. Module has to accept some changes.
TT Close protocol uses "sleep condition" and "sleep time" to configure basic behaviour on the modules. TT Open adds Generic Configuration values that can be configured at user level so that devices will take that new value. This will obviously included in the response for that GC module. 


<a name="def-modules"></a>
### 5. Supported TT Modules.

This section will detail all TT supported modules and how they will be represented as entities when published on ContextBroker. In the following examples, each module is published as a single entity, but this might not be the usual scenario as TT devices have more than one modules, so it's common to find several attributes (each of them representing a different module) in a single entity. Combinations of P1,B,GPS,GM are common. 


#### Core Module: K1
This module will never get published on ContextBroker as it does not represent any measurable attribute, but just the core system of the Thinking Things stacks. That is to say, it is the communication element of the group.

Example of module:
	#0,K1,0$


What is responded to the device:
	#0,K1,0$



#### GSM location: P1
P1 represents the GSM cells information. Users can use this information to query a separate system and get the geolocation of the device. Current version of IoTAgent does not support more than one cell sent per request.
Therefore the response is fixed (while original TT Close protocol supports sending an specific command in the response to tell the device to send more cells).

Example of TT measure:

     #0,P1,214,07,33f,633c,99

What is responded to the device:

     #0,P1,$

What is published on ContextBroker:

		{
                "id":"<entity-name>",
                "type" : "<entity-type>",
                "isPattern" : "false",
                "attributes" : [
                               {
                                	"name":"P1",
                                        "type":"compound",
                                        "value":[
                                              {
                                                "name":"mcc",
                                                "type":"string",
                                                "value":"214"
                                               },
                                               {
                                                 "name":"mnc",
						 "type":"string",
                                                 "value":"07"
                                               },
                                               {
                                                 "name":"cell-id",
                                                  "type":"string",
                                                  "value":"33f"
                                                },
						{
                                                  "name":"lac",
                                                  "type":"string",
                                                   "value":"633c"
                                                 },{
                                                   "name":"dbm",
                                                   "type":"string",
                                                   "value":"99"
                                               }]
                                               ,"metadatas":[
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

Please note that both "entity-name" and "entity-type" are fields you can provide when adding a new device. This example has only one module, but the usual scenario implies TT devices sending more than one module that will be converted in individual attributes. 

##### Mandatory fields
P1 is made of up to five fields, being the first four mandatory. This means that if three or fewer fields are received, the measure will be ignored (and therefore nor published on ContextBroker neither sent in the HTTP response to the device).  

#### Battery information: B
This module collects all information about the status of the battery, if it's being charged, how long will it take to disconnect, etc... Please refer to Thinking Things main documentation to find out more about what each individual field means. 

Example of B module (these are just random values to illustrate the format):

    #0,B,12,50,true,45,batt,56

What is responded to the device (extracted from the original request):

    #0,B,50,batt,56


What is published on ContextBroker:

                {
                "id":"<entity-name>",
                "type" : "<entity-type>",
                "isPattern" : "false",
                "attributes" : [
                               {
                                	"name":"B",
                                        "type":"compound",
                                        "value":[
                                              {
                                                "name":"voltaje",
                                                "type":"string",
                                                "value":"12"
                                               },
                                               {
                                                 "name":"estado_activacion",
						 "type":"string",
                                                 "value":"50"
                                               },
                                               {
                                                 "name":"hay-cargador",
                                                  "type":"string",
                                                  "value":"true"
                                                },
						{
                                                  "name":"estado_carga",
                                                  "type":"string",
                                                   "value":"45"
                                                 },{
                                                   "name":"modo_carga",
                                                   "type":"string",
                                                   "value":"batt"
                                               },{
                                                   "name":"tiempo_desco_stack",
                                                   "type":"string",
                                                   "value":"56"
                                               }]
                                               ,"metadatas":[
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


##### Mandatory fields
All fields are mandatory for Battery module.

#### Geolocation based on GPS
This module will send its location as read from the GPS. 

Example of measure:

     #0,GPS,40.418889,-3.691944,11,33,45,$

What is responded to the device:
    
    #0,GPS,$

What is published on ContextBroker:

              {
                "id":"<entity-name>",
                "type" : "<entity-type>",
                "isPattern" : "false",
                "attributes" : [
                               {
                               "name":"position",
                                "type":"coords",
                                "value":"40.418889,-3.691944",
                                "metadatas":[{
                                     "name":"location",
                                     "type":"string",
                                     "value":"WGS84"
                                     },
                                     {
                                       "name":"TimeInstant",
                                       "type":"ISO8601",
                                       "value":"2015-03-19T14:39:14.924961Z"
                                      }
                                      ]
                              },
                              {
                                 "name":"TimeInstant",
                                 "type":"ISO8601",
                                 "value":"2015-03-19T14:39:14.924961Z"
                               }]
                    }





#### Mandatory fields
As the information sent to ContextBroker needs only longitude and latitude, therefore only the first two fields are mandatory.

### Generic Measures: GM
This is new in TT Open. GM module is any measure defined by the user. It has name and value and the format is as follows:

Example

   #0,GM,temp,23

Response to device

   #0,GM

Published to ContextBroker:

               {
                "id":"<entity-name>",
                "type" : "<entity-type>",
                "isPattern" : "false",
                "attributes" : [
                               {
				 "name":"temp",
				 "type":"string",
				 "value":"23",
				 "metadatas":[
					{
					 "name":"TimeInstant",
					 "type":"ISO8601",
					 "value":"2015-03-19T14:39:14.924961Z"
					}
					]
				},
			       {
				 "name":"TimeInstant",
				 "type":"ISO8601",
				  "value":"2015-03-19T14:39:14.924961Z"
				}]
		}

In this case, the name "temp" is what is defined by the user.


### Generic Configuration: GC
Another TT Open feature. This module is used to report the value of a user defined attribute being also able to modify it by publishing on ContextBroker (next section).

Example of a config value:
   
   #0,GC,conf,33

Response to device (new value is provided):
   
   #0,GC,conf,44

Device will take the new value for "conf".

What is published on ContextBroker:

               {
                "id":"<entity-name>",
                "type" : "<entity-type>",
                "isPattern" : "false",
                "attributes" : [
                               {
				 "name":"_TTcurrent_conf",
				 "type":"string",
				 "value":"44",
				 "metadatas":[
					{
					 "name":"TimeInstant",
					 "type":"ISO8601",
					 "value":"2015-03-19T14:39:14.924961Z"
					}
					]
				},
			       {
				 "name":"TimeInstant",
				 "type":"ISO8601",
				  "value":"2015-03-19T14:39:14.924961Z"
				}]
		}

Note that the name of the attribute has been changed to "_TTcurrent_conf", this is done so that there's a differentiation between what IoTAgent publishes on behalf of the device and what the user can publish on ContextBroker to change that particular value.



<a name="def-configuration"></a>
## 6. Changing Generic Configuration
TT Open protocol allows users to change values of custom attributes defined in their TT devices. This is done by means of the GC modules. Each module represent in fact a property of the device that can be changed. How this is achieved is rather simple. IoTAgent will add a prefix to the name of the attribute (or GC) when it comes to the TT Device. This prefix is "_TTcurrent_". Prior sending the response to the device, it will do a queryContext operation on the ContextBroker to get the desired value set by the user. The user has to published on the device's entity (same entity_name, service, sub-service) an attribute named as the GC and with the desired value. Let's take a look at this process step by step.

### Example of changing GC "polling_rate" attribute.

The user has defined "polling_rate" attribute on his device. Original value was "20" but now he wants to set "30". So he publishes the following entity on ContextBroker:

               {
                "id":"my_device",
                "type" : "tt",
                "isPattern" : "false",
                "attributes" : [
                               {
				 "name":"polling_rate",
				 "type":"string",
				 "value":"30",
				 "metadatas":[
					{
					 "name":"TimeInstant",
					 "type":"ISO8601",
					 "value":"2015-03-19T14:39:14.924961Z"
					}
					]
				},
			       {
				 "name":"TimeInstant",
				 "type":"ISO8601",
				  "value":"2015-03-19T14:39:14.924961Z"
				}]
		}

After a while (depending on how TT Device is configured to send information to IoT Platform) the device will send all its modules:
       
       cadena=#80000110000,#0,P1,214,07,33f,633c,#0,GC,polling_rate,20,#0,K1,0$

The incoming value for "polling_rate" is still "20". Upon the reception of this request, the IoTAgent will query ContextBroker searching for new values for GC. It will get previous entity with new value "30". However, it will published on ContextBroker what the device has sent, that's including "20".
            
            {
		"id":"my_device",
		"type":"tt",
		"isPattern":"false",
		"attributes":[
		{
			"name":"_TTcurrent_polling_rate",
			"type":"string",
			"value":"20",
			"metadatas":[
			{
				"name":"TimeInstant",
				"type":"ISO8601",
				"value":"2015-03-19T15:16:52.038833Z"
			}
			]
		},
		{
			"name":"P1",
			"type":"compound",
			"value":[
			{
				"name":"mcc",
				"type":"string",
				"value":"214"
			},
			{
				"name":"mnc",
				"type":"string",
				"value":"07"
			},
			{
				"name":"cell-id",
				"type":"string",
				"value":"33f"
			},
			{
				"name":"lac",
				"type":"string",
				"value":"633c"
			}],
			"metadatas":[
			{
				"name":"TimeInstant",
				"type":"ISO8601",
				"value":"value":"2015-03-19T15:16:52.038833Z"
			}]
		},
		{
			"name":"TimeInstant",
			"type":"ISO8601",
			"value":"value":"2015-03-19T15:16:52.038833Z"
		}]
	}

However, the HTTP response sent to the device now it includes the new value:

    #0,GC,polling_rate,30,#0,P1,#0,K1,0$

Next time the device sends its modules information, it will indeed contain "30" as the value for "polling_rate" and that's how it will be published on ContextBroker.


