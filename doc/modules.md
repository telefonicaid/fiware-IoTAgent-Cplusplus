# Southbound API
Every module has an identifier and description in order to be identified in applications. Identifier is hard-coded and this information nust not be changed. But `description` field
might change if you provide `ProtocolDescription` field as an option in module configuration. For example:
```
     {
      "resource": "/iot/mqtt",
      "options": {
         "ConfigFile": "/etc/iot/MqttService.xml",
         "FileName": "MqttService",
         "ProtocolDescription": "MQTT for testing purpose"
        }
      }
```
## Ultra Light Agent ###
Ultra-Light (UL20) is a HTTP protocol to send short data.

```
Identifier: PDI-IoTA-UltraLight
Description: UL2
```

Previously, this module is configured in _/iot/d_ URI and the service provisioned has _apikey_ ('abc' in curl example). Parameters are:
You can send data with HTTP GET (it use query parameters):
```
curl -X GET 'http://127.0.0.1:8080/iot/d?i=dev_agus&d=t|15&k=abc' -i
```
- _i_: device identifier.
- _d_: data as key value separated by character |.
- _k_: apikey assigned to service.

Published entity is:
```
{"id":"thing:dev_agus","type":"thing","isPattern":"false","attributes":[{"name":"t","type":"","value":"15","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2015-02-12T08:31:07.661697Z"}]},{"name":"TimeInstant","type":"ISO8601","value":"2015-02-12T08:31:07.661697Z"}]}
```

A value could be a paired value (like GPS position) following this format _a_/_b_.
You can send data with HTTP POST. With this option you can send multiple _data_ groups. Every group mus be separated by character #. One group is composed by _key_|_value_. Each group updates entity.
```
curl -X POST 'http://127.0.0.1:8080/iot/d?i=dev_agus&k=abc' \
     -i \
     -d 'gps|1.2/3.4#t|10'

```
Published entity is:
```
{"id":"thing:dev_agus","type":"thing","isPattern":"false","attributes":[{"name":"gps","type":"","value":"1.2/3.4","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2015-02-12T08:34:21.738847Z"}]},{"name":"TimeInstant","type":"ISO8601","value":"2015-02-12T08:34:21.738847Z"}]}
```
and
```
{"id":"thing:dev_agus","type":"thing","isPattern":"false","attributes":[{"name":"t","type":"","value":"10","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2015-02-12T08:41:54.864249Z"}]},{"name":"TimeInstant","type":"ISO8601","value":"2015-02-12T08:41:54.864249Z"}]}
```

<a name="location"></a>
### Conversion to Location Entity for ContextBroker ###

When a device is provisioned, some basic conversions can be set using the device's "attributes". Those can be configured using the devices provisioning API. ContextBroker supports a native attribute format for location measures. Thus it makes sense to offer this type of conversion as built-in. Both UltraLight and MQTT plugins can benefit from this by just having their devices provisioned with an specific attribute as detailed below:


```
	{
           "object_id":"l",
           "type": "coords",
           "name":"position"
   	}
```

That is part of the device's attributes. If this is not added to the JSON when creating a device, the conversion will not happen. The  __"object_id"__ is the name of the original attribute coming in the message. UL will typically use one letter, but MQTT can have a longer name. This is whatever identifies the actual location information sent by the device. The __"type"__ field has to be "coords" as the conversion is built-in. Lastly, __"name"__ is the name of the attribute sent to the ContextBroker.

For a message coming from a device like this:

UltraLight:
```
l|33.000/-3.234234
```

The Attribute sent to the ContextBroker will be:

```
{
	"name":"position",
	"type":"coords",
	"value":"33.000,-3.234234",
	"metadatas":[
	{
		"name":"location",
		"type":"string",
		"value":"WGS84"
	}]
}
```


You can review protocol reference in [Ultra-Light](UL20_protocol.md).

## MQTT Agent ###

```
Identifier: PDI-IoTA-MQTT-UltraLight
Description: MQTT
```

MQTT is a M2M oriented protocol based on TCP that requires a broker for publishing and subscribing to messages based on topics hiearchy. In order to be able to publish and receive on IoTAgent-MQTT, services must be provisioned providing an api-key that will be used in the topics sent by devices belonging to that service as explained here [MQTT](MQTT_protocol.md). The typical port used is 1883 (no SSL).

As a reminder, the __config.json__ file has to contain at least one "resource" for MQTT like this:

     {
      "resource": "/iot/mqtt",
      "options": {
         "ConfigFile": "/etc/iot/MqttService.xml",
         "FileName": "MqttService"
        }
      }

Besides, the IoTAgent has to be told where to find the MqttService.so file. This is done by using the option "-d" when starting it:

     export LD_LIBRARY_PATH=/usr/local/iot/lib
     /usr/local/iot/bin/iotagent -n mqtt -v DEBUG  -p 80 -d /usr/local/iot/lib -c /etc/iot/config.json &



As MQTT works over TCP, there is no curl command to test it. However, anyone could use <a href=http://mosquitto.org>Mosquitto</a> implemenation to test it (it requires bulding it). It comes with both clients: publisher and subscriber, so once a service is provisioned, you can issue the following commands to test that is working:

For subscribing to topics coming to your recently provisioned device (replace api-key and device-id with actual ids). "Server.name" is the IP or hostname where the broker is (typically is the same as IotAgent's)
```
     mosquitto_sub -h sever.name -t /api-key/device-id/+ -d
```

Then you can publish a test (on a separate shell):

```
     mosquitto_pub -h server.name -t /api-key/device-id/test -m 44
```


__Note__: when ACL is used, mqtt clients have to use user-id that must be the same as the Api-key. In this example, by providing "-u <api-key>" on both commands you can do so.

```
     mosquitto_pub -h server.name -t /api-key/device-id/test -m 44 -u api-key
```
ACL is enabled by default, and it's recommendable to leave it enabled (as it will make sure devices publish on their topics only). However, this could be disabled by editing "mosquitto.conf" file and commenting out the line where path to ACL file is set. 

You can check that the message has come to the subscriber and also to the ContextBroker, as IoTAgent will be listening to all publications.

```
{"id":"my_device","type":"thing","isPattern":"false","attributes":[{"name":"test","type":"string","value":"44","metadatas":[{"name":"TimeInstant","type":"ISO8601","value":"2015-03-20T08:52:22.235908Z"}]},{"name":"TimeInstant","type":"ISO8601","value":"2015-03-20T08:52:22.235908Z"}]}
```

### Conversion to Location Entity ####
As it has been explained [here](#location) a device can be provisioned with the built-in location conversion for ContextBroker. In this case the name "object_id" will be the topic that represents the measure. Following a similar example to the UltraLight scenario, for a device provisioned with:


```
	{
           "object_id":"location",
           "type": "coords",
           "name":"position"
   	}
```

And a MQTT message of this kind:

Topic:
```
 /<api-key>/<device-id>/location
```
Payload:
```
33.000/-3.234234
```

The Attribute sent to the ContextBroker will be:

```
{
	"name":"position",
	"type":"coords",
	"value":"33.000,-3.234234",
	"metadatas":[
	{
		"name":"location",
		"type":"string",
		"value":"WGS84"
	}]
}
```


## Thinking Things Agent ###

```
Identifier: PDI-IoTA-ThinkingThings
Description: Thinking Things Protocol
```

Thinking Things devices use their own frame format over HTTP requests to send measures and take configuration changes. Thinking Things module for IoTAgent will transform those HTTP request into ContextBroker entities following the mapping described here: [Thinking Things](TT_protocol.md). One key thing that differentiates Thinking Things from others protocols is that there is no "apikey" for identifying services. Users will need to provision a "sub-service" within the given Service. For doing so, they have to provide a "trust_token" as credentials.

Header for the service:
```
Fiware-Service:
```

And Sub-service:
```
Fiware-ServicePath:
```

Once the sub-service is successfully created, devices can be added to that service. The name of the device can be personalized by providing an "entity_name" header, same for "entity_type". These two fields will appear on the entity published on ContextBroker (see example below).

All devices will target the same URL and the stack-id that is included in the request is used internally to obtain the service and sub-service where that particular device belongs. The following example shows an example of the request sent by a device (note that part of the body is URL encoded) and how the entity published on ContextBroker looks like.

```
curl -v http://10.0.2.15:8080/iot/tt -d cadena=%238934075379000039321%2C%230%2CGM%2Ctemp%2C25%2C20%24wakeUP
```

The body has the keyword "cadena=" and after that, the TT frame is URL encoded. For clarity this is what has been sent:
Stack-id: 8934075379000039321
Module: GM
Name: temp
value: 25
Sleep Condition: "wakeUp"
Sleep Time: 20



Published entity is:
```
{"id":"my_room","type":"thing","isPattern":"false","attributes":[{"name":"temp","type":"string","value":"25","metadatas":[{"name":"sleepcondition","type":"string","value":"wakeUP"},{"name":"sleeptime","type":"string","value":"20"},{"name":"TimeInstant","type":"ISO8601","value":"2015-03-20T08:52:22.235908Z"}]},{"name":"TimeInstant","type":"ISO8601","value":"2015-03-20T08:52:22.235908Z"}]}
```
Note that the stack-id is not directly published on ContextBroker but the entity_name given by the user when the device was provisioned, in this example :"my_room"

