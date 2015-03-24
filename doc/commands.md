FORMAT: 1A

# Resource IotAgent Commands

NOTE: This document is a **work in progress**.


#### Index
1. [Introduction](#def-introduction)
2. [Provisioning comands](#def-prerequisites)
3. [Send an command](#def-sendCommand)
4. [Check results](#def-result)
5. [polling](#def-polling)
6. [Calls to Context Broker](#def-API_cb)

<a name="def-introduction"></a>
## 1. Introduction

This chapter describes how to configure and send commands to devices.
A command is an operation or action that a device can do.
This command is composed of a name (identifier) and a value (all data send to the device).

Provisioning a device is mandatory to send a command. You can do this with with Iotagent API
[Iotagent API](API_REFERENCE.md) or [Iotagent config file](deploy.md#def-resources)


A command has different parts:


- Provisioned command (data to create the command text that the device is able to understand )

| Parameter     |description               |
| ------------- |:-------------:           |
| name   | identifier command for users     |
| type  | command      |
| value | result of transformation, protocol dependent   |

- specific command data (when you send a single command, you can send specified data in the value of attribute)

This data can be the entire text send to device, or parameters used as a part of this.
 [More information](##def-prerequisites)

![IoT Agent architecture](imgs/regDevice.png)

Provisioning a device is mandatory to send a command. 
A user can provision a device with commands. IotAgent registers the device and the commands in Contest Broker, so you can send and
[Updatecontext](https://forge.fiware.org/plugins/mediawiki/wiki/fiware/index.php/Publish/Subscribe_Broker_-_Orion_Context_Broker_-_User_and_Programmers_Guide#Update_context_elements) (how to send a command)


![IoT Agent architecture](imgs/synccommands.png)


Example: execute the command PING with data 22
```
curl -X POST http://$CB:$PORT/iot/ngsi/updateContext \
     -i \
-H "Content-Type: application/json" \
-H "Fiware-Service: TestServiceF" \
-H "Fiware-ServicePath: /TestSubserviceF" \
-d ' {"updateAction":"UPDATE","contextElements":[{"id":"entity_name","type":"entity_type","isPattern":"false","attributes":[{"name":"PING","type":"command","value":"22" } ]} ]}'
```

Context broker sends the updateContext to Iotagent, Iotagent check provisoned device and  send to the device "device_id@ping6|22", in the same http, the device returns the result.

There are two type of commands

- Push command, the command is sent to device, and the the device responds immediately. 

- Poling commands, these actions can spend too much time or the device is sleeping, so Iotagent doesn't send the commands to iotagent, waits for the device to ask for.
User can check the status of the command because of IotAgent update an special attribute (command name + "_status") so
user can subscribe or consult this.

![IoT Agent architecture](imgs/pollingccommands.png)

[more information about polling commans](#def-polling)

<a name="def-prerequisites"></a>
## 2. Provisioning commands

Example a command named PING, this command is ul20
```
curl -X POST http://$HOST:$PORT/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: TestServiceF" \
-H "Fiware-ServicePath: /TestSubserviceF" \
-d ' { "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type", "timezone": "America/Santiago", "commands": [{ "name": "PING", "type": "command", "value": "device_id@ping6|%s"}]  } ] }'
```


<a name="def-sendCommand"></a>
## 3. Send a Command

special attributes to check command resolution
<command_name>_status

| status     |description               |
| ------------- |:-------------:           |
| OK   | command completed, see <command name>_info attribute to see the result    |
| not delivered   | name of the service_path (must start with /)    |

<command_name>_info
comman result

Error message:

{ "errorCode" : { "code":404, "reasonPhrase":"The service does not exist"}}

{ "errorCode" : { "code":404, "reasonPhrase":"The device does not exist"}}

{ "errorCode" : { "code":404, "reasonPhrase":"the device does not have implemented this command"}}

{ "errorCode" : { "code":400, "reasonPhrase":"this is not a valid command"}}

{ "errorCode" : { "code":404, "reasonPhrase":"the device does not have implemented this command"}}


<a name="def-result"></a>
## 4. Check results



<a name="def-polling"></a>
## 5. Polling

### Special attributes

Special attributes to check command resolution

"command_name"_status :

| status     |description               |
| ------------- |:-------------:           |
| pending     | command ready, but the device still has not asked for|
| expired read   | command expired, the device didn't ask for it.    |
| delivered   | the device has received the command, but did not send the result    |
| delivered but no respond   | command delivered, but no command result from device     |
| OK   | command completed, see <command name>_info attribute to see the result  |

"command_name"_info : 
comman result

### Polling commands scenario

To manage polling commands you shuold create a device witout endpoint. Below two are provided, the first to create a service and the second one to create a device without commands endpoint. In this example command attributa name is "ping"

```
curl -X POST http://x.x.x.x:8080/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: TestService" \
-H "Fiware-ServicePath: /ssrvTest" \
-d '{"services": [{ "apikey": "apikey3", "token": "token2", "cbroker": "http://y.y.y.y:1026", "entity_type": "thing", "resource": "/iot/d" }]}'


curl -X POST http://x.x.x.x:8080/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: TestService" \
-H "Fiware-ServicePath: /ssrvTest" \
-d ' { "devices": [ { "device_id": "device_id_2", "entity_name": "entity_name_2", "entity_type": "entity_type", "timezone": "America/Santiago", "commands": [ { "name": "ping", "type": "command", "value": "device_id_2@ping|%s"} ], "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int" } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value" } ] } ] }'
```


To generate a polling command, contextBroker should send a updateContextRequest to URL in which iotagent is listening for NGSI messages by this resource, in this case "http://x.x.x.x:8080/iot/ngsi/d/updateContext" with  "Fiware-Service" and "Fiware-ServicePath" header. This updateContest from contextBroker could be simulates with the curl: 

```
curl --request POST  http://x.x.x.x:8080/iot/ngsi/d/updateContext --header 'Fiware-Service: TestService' --header 'Fiware-ServicePath: /ssrvTest'  --header 'Content-Type: application/json' $CURL_VERBOSE --data-binary @ngsi_update_cmd_p.json
```

In ngsi_update_cmd_p.json

```
{"updateAction":"UPDATE","contextElements":[{"id":"entity_name_2","type":"entity_type","isPattern":"false","attributes":[{"name":"ping","type":"command","value":"OK"}]}]}
```

Iotagent proccess the command, detects that is a polling command because device does not have command endpoint, and store command in a cache with the expiration "timeout" defined un config.json. Iotagent to contextBroker an updateContextRequest with attribute _status "pending".

```
{
  "updateAction": "APPEND",
  "contextElements": [
    {
      "id": "entity_name_2",
      "type": "entity_type",
      "isPattern": "",
      "attributes": [
        {
          "name": "ping_status",
          "type": "string",
          "value": "pending"
```              


You can simulate a command response from the device, using the curl :

```
curl -v --data "device_id_2@ping|MADE_OK"  --request POST  "http://192.168.179.191:8023/iot/d?k=apikey3&i=device_id_2"
```

If this command response arrives before command timeout expires, iotagent will send to contextBroker two updateContextRequest, on with an attribute  _status to "OK"

```
{
  "updateAction": "APPEND",
  "contextElements": [
    {
      "id": "entity_name_2",
      "type": "entity_type",
      "isPattern": "",
      "attributes": [
        {
          "name": "ping_status",
          "type": "string",
          "value": "OK"
```


And other one with an attribute _info containing the command response received from device.

```
{
  "updateAction": "APPEND",
  "contextElements": [
    {
      "id": "entity_name_2",
      "type": "entity_type",
      "isPattern": "",
      "attributes": [
        {
      "name": "ping_info",
      "type": "string",
      "value": "device_id_2@ping|MADE_OK",
```



If command timeout expires and IotAgnet has not received command response from device, an updateContextRequest will be sent to contextBroker two updateContextRequest with an attribute  _status to "expired read"

```
{
  "updateAction": "APPEND",
  "contextElements": [
    {
      "id": "entity_name_2",
      "type": "entity_type",
      "isPattern": "",
      "attributes": [
        {
          "name": "ping_status",
          "type": "string",
          "value": "expired read"
```




<a name="def-API_cb"></a>
## 6. Calls to Context Broker

Here is described a complete command scenario as described in pictures Device Registration and command  of Introduction.

### Device Registration

You can send some HTTP request  to iotagent in order to create an service and a device.

To create a service you can try:

```

curl -X POST http://x.x.x.x:8080/iot/services \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: service2" \
-H "Fiware-ServicePath: /srvpath2" \
-d '{"services": [{ "apikey": "apikey2", "token": "token2", "cbroker": "http://y.y.y.y:1026", "entity_type": "thing", "resource": "/iot/d" }]}'

```

Curl IP x.x.x.x  should by replaced by VM IP, and in the url defined in cbroker,  you should replace ip and port by the ones in which  contextBroker is installed.

To create a device  you can try :

```
curl -X POST http://x.x.x.x:8080/iot/devices \
-i \
-H "Content-Type: application/json" \
-H "Fiware-Service: service2" \
-H "Fiware-ServicePath: /srvpath2" \
-d ' { "devices": [ { "device_id": "device_id", "entity_name": "entity_name", "entity_type": "entity_type", "timezone": "America/Santiago", "commands": [ { "name": "ping", "type": "command", "value": "device_id@ping|%s"} ], "attributes": [ { "object_id": "source_data", "name": "attr_name", "type": "int" } ], "static_attributes": [ { "name": "att_name", "type": "string", "value": "value" } ] } ] }'

```

Then iotagent will send to contextBroker a NGSI9 registerContextRequest, with this payload:

```
{
  "contextRegistrations": [
    {
      "entities": [
        {
          "id": "entity_name",
          "type": "entity_type",
          "isPattern": "false"
        }
      ],
      "attributes": [
        {
          "name": "ping",
          "type": "command",
          "isDomain": "false"
        }
      ],
      "providingApplication": "http://x.x.x.x:8080/iot/ngsi/d"
    }
  ]
}
```

ContextBroker will response with a NGSI9 registerContextResponse such as :

```
{
  "duration" : "PT24H",
  "registrationId" : "54e9f89998505ce54adc79b7"
}
```


### Command

You can send command to contextBroker by sending an NGSI10 updateContextRequest with updateAction "UPDATE", you can try  the curl:

```
curl --request POST  http://y.y.y.y:1026/v1/updateContext --header 'Fiware-Service: service2' --header 'Fiware-ServicePath: /srvpath2'    --header 'Accept: application/json'    --header 'Content-Type: application/json' $CURL_VERBOSE --data-binary @ngsi_update_cmd.json

```

In ngsi_update_cmd.json  file:

```
{
  "updateAction": "UPDATE",
  "contextElements": [
    {
      "id": "entity_name",
      "type": "entity_type",
      "isPattern": "false",
      "attributes": [
        {
          "name": "ping",
          "type": "command",
          "value": "OK"
        }
      ]
    }
  ]
}
```

