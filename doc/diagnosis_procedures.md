#<a name="top"></a>Diagnosis procedures

* [Resource availability](#resource-availability)
* [Remote Service Access](#remote-access)
* [Resource consumption](#resource-consumption)
* [I/O Flows](#io-flows)


## <a name="resource-availability">Resource availability</a>

To be filled soon by DevOps

[Top](#top)

## <a name="remote-access">Remote Service Access</a>

- IoT Agents ADMIN API should be available at the launching-specified TCP port. 
- If active, Ultralight2.0 southbound (device) API should be available at the same  launching-specified TCP port as the ADMIn API.
- If active, MQTT  southbound (device) API should be available at the well-known MQTT ports except if others than default were specified.

[Top](#top)

## <a name="resource-consumption">Resource consumption</a>

To be filled soon by DevOps

[Top](#top)


## <a name="io-flows">I/O Flows</a>


- IoT Agents (UL2.0 and MQTT)

	- Input flows: IoT Agents listen in their respective southbound ports (UL2: specified TCP port, MQTT: well-known ports) and the provision API listens in the specified TCP port.The IoT Agents 
IoT Agents also listen for commands sent by a ContextBroker.

	- Output flows: IoT Agents send devices input to the configured IP and port of the IoT Agent.

- IoT Manager
 
	- Input flows: If enabled, IoT Manager receives provision requests at the TCP specified port.

	- Output flows: If enabled, IoT Manager sends device provision details to the configured IP and TCP port of a ContextBroker.

[Top](#top)