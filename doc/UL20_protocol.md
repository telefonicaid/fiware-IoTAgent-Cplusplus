FORMAT: 1A

# UL2 IotAgent API


#### Index
1. [Introduction](#def-introduction)
2. [Iot configuration](deploy.md#def-storage)
3. [Security Considerations](deploy.md#def-security)
4. [Send an observation](#def-observation)
5. [Receive a command and send response](#def-command)
6. [Calls from device](#def-API_ul20)

<a name="def-introduction"></a>
## 1. Introduction
This documents is the specification of the Ultra light Protocol for Iot Platform.
Ultra-Light protocol (UL20) is a lightweight HTTP protocol designed to be open, simple, lightweight and easy to implement, send short data in a short way.

There are two types of message: measurements of sensors and commands (actions that can make a device, [More information about comands](https://github.com/telefonicaid/fiware-IoTAgent-Cplusplus/blob/develop/doc/commands.md) )

A device can receive commands (from an Iot Agent), and produces measurements and response commands.

A device is defined by an apikey (the key that defines the service),
and by a name, this name must be unique for a service.


<a name="def-observation"></a>
## 4. Send an observation

#### Example: a single observation

    t|10
    <alias>|<value>

Device uses an alias "t"  and sends a value "10".

Device configuration includes a field _attributes_ used for mapping.

For example, alias "t" will be mapped to "temperature" when measure is published.

#### Example: several observations

    t|10#m|15#gps|12.1/14.2
    <alias>|<value>#<alias>|<value1>/<value2>

In this example, a device sends three measures (t|10, m|15 and gps|12.1/14.2).

Different observations are separated with #
Multivalue are separated with /. You could use mapping features to send a location to Context Broker [Conversion to location](modules.md).


<a name="def-command"></a>
## 5. Receive a command and send response

#### Example: a single command

Commands in ultra light must have a specific format.

    device_name@ping|param1=1|param2=2

    <device name>@<command name>|<param name>=<value>|....

  if a command doesn't have got this format, Iot agent will return an error.

#### Example: a single response

    device_name@ping|Pink ok

    <device name>@<command name>|result

   The answer of a command have a specific format too.


<a name="def-API_ul20"></a>
## 6. Calls from device

# Group UL20 protocol for IotAgent Platform
Group of  requisites, conventions for this http protocol for Telefonica Iot Platform.

## POST [/{iot}/{plugin_uri}{?i,d,k,t,getCmd,ip}]
A device can send to Iot Platform, an observation ( one or several sensor measurements), or a command response.
These operations must be implemented in the device,

### POST an observation to Iot Platform [POST]

+ Parameters

    + iot (required, string, `iot`) ... iotagent base uri, the uri where iotagent has been risen.

    + plugin_uri (required, string, `d`) ... plugin base uri, the uri where iotagent rise the ul20 protocol.

    + i (required, string, `device_name`) ... parameter to define the device.

    + d (required, string, `key|value`) ... parameter to define the data. It is a key and a value separated by |.

    + k (required, string, `apikey`) ... apikey of the service, reference to the service to which the device belongs .

    + t (optional, string, `2014-02-18T16:41:20Z`)   (UTC ISO8601) for the observation

    + ip (optional, string, `127.0.0.1`) ...   to define IP address.

    + getCmd (optional, string, `1`) ... if getCmd=1, in the response add all commands for this device

+ Response 200

+ Response 400

## GET  [/{iot}/{plugin_uri}{?i,d,k}]
A device can ask for pending commands.
These operations must be implemented in the device,

### GET pending commands from Iot Platform [GET]

+ Parameters

    + iot (required, string, `iot`) ... iotagent base uri, the uri where iotagent has been risen.

    + plugin_uri (required, string, `d`) ... plugin base uri, the uri where iotagent rise the ul20 protocol.

    + i (required, string, `device_name`) ... the name of the device.

    + k (required, string, `apikey`) ... apikey of the service, reference to the service to which the device belongs .



+ Request (application/json)

+ Response 200

    returns all pending commands separated by #
    no json, only the command text
    <device_name>@<command_name>|parameters

    + Body

            devicef22@ping2#devicef22

+ Response 400
