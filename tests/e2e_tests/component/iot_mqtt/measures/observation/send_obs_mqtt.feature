Feature: Deliver measures through MQTT
	In order to include MQTT protocol like an IoTAgent
	As a MQTT publisher
	I want to deliver measures to MQTT IoTAgent
	
	@mqtt @IDAS-18304
	Scenario Outline: Send one measure
		Given a service with name "<service>" and protocol "<protocol>" created
		When I publish a MQTT message with device_id "<device_id>", alias "<alias>" and payload "<value>"
		And I Wait some time
		Then the measure of asset "<device_id>" with phenom "<alias>" and value "<value>" is received by context broker
		
		Examples:
			| phenomenon			|service		|protocol	| type		| device_id	| alias					| value 		|
			| temperature			|servicemqtt	|IoTMqtt	| Quantity	| mqtt10	| t						| 35			|
			| presence				|servicemqtt	|IoTMqtt	| Boolean	| mqtt11	| p						| false			|
			| location				|servicemqtt	|IoTMqtt	| Location	| mqtt12	| l						| 125.9,3.1		|
			| alarm					|servicemqtt	|IoTMqtt	| Text		| mqtt13	| a						| Danger		|
			| temperaturainterior	|servicemqtt	|IoTMqtt	| Quantity	| mqtt14	| tint					| -2.5			|			
#			| presence				| Boolean	| mqtt1		| presence				| true			|
#			| presence				| Boolean	| mqtt2		| presence				| false			|
#			| temperature			| Quantity	| mqtt3		| temperature			| 23			|
#			| temperature			| Quantity	| mqtt4		| temperature			| -12.0			|
#			| temperature			| Quantity	| mqtt5		| temperature			| 0.12			|
#			| location				| Location	| mqtt6		| location				| 40.1,23.4		|
#			| temperaturainterior	| Quantity	| mqtt7		| temperaturainterior	| 44			|
#			| location				| Location	| mqtt8		| location				| 5.0,123.0		|
#			| alarm					| Text		| mqtt9		| alarm					| Alarma		|
	

	@mqtt @IDAS-19969
	Scenario Outline: Send one wrong measure
		Given a service with name "<service>" and protocol "<protocol>" created
		When I publish a MQTT message with device_id "<device_id>", alias "<alias>", payload "<value>" and wrong field "<field>" 
		And I Wait some time
		Then the measure of asset "<device_id>" with phenom "<alias>" and value "<value>" is received or NOT by context broker
		
		Examples:
			| phenomenon	|service		|protocol	| type		| device_id	| alias	| value	| field		|
			| temperature	|servicemqtt	|IoTMqtt	| Quantity	| mqtt15	| t		| 5		| apikey	|
			| temperature	|servicemqtt	|IoTMqtt	| Quantity	| 			| t		| 35	| device_id	|
			| presence		|servicemqtt	|IoTMqtt	| Boolean	| mqtt16	| 		| false	| alias		|
			| location		|servicemqtt	|IoTMqtt	| Location	| mqtt17	| l		| 		| payload	|
			| temperature	|servicemqtt	|IoTMqtt	| Quantity	| mqtt19	| t		| 90	| topic		|

	
	@mqtt @IDAS-18329	
	Scenario Outline: Send several measures
		Given a service with name "<service>" and protocol "<protocol>" created
		When I publish a MQTT message with device_id "<device_id>", tag "<tag>" and payload "<alias1>|<value1>#<alias2>|<value2>#<alias3>|<value3>#<alias4>|<value4>"
		And I Wait some time
		Then "<num_measures>" measures of asset "<device_id>" are received by context broker
			|value 		|alias		|
			|<value1>	|<alias1>	|
			|<value2>	|<alias2>	|
			|<value3>	|<alias3>	|
			|<value4>	|<alias4>	|
			
		Examples:
			|device_id		|service		|protocol	| tag	| value1	| alias1	| value2	| alias2		| value3	| alias3	| value4	| alias4	| num_measures	|
			|mqtt20			|servicemqtt	|IoTMqtt	| mul20	| bye		| a			| 53.2		| t				| false		| p			| 5.4,-4.1	| l			| 4				|
			|mqtt21			|servicemqtt	|IoTMqtt	| mul20	| 7.8		| t			| 3.2		| t				| hello		| alarm		| bye		| alarm		| 4				|
#			|AssetCompleto	| multi	| hello		| alarm		| 27.9		| temperature	| true		| presence	| -1.3,2.4	| location	| 4				|
			