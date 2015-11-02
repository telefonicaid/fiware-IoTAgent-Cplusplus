Feature: MQTT Measure Deliver  
	In order to include MQTT protocol like an IoTAgent
	As a MQTT publisher
	I want to deliver measures to MQTT IoTAgent
	
	@iot_mqtt @IDAS-18304
	Scenario Outline: Send one measure
		Given a Service with name "<service>" and protocol "<protocol>" created
		When I publish a MQTT message with device_id "<device_id>", alias "<alias>" and payload "<value>"
		And I Wait some time
		Then the measure of asset "<device_id>" with measures "<generated_measures>" is received by context broker
		
		Examples:
			| phenomenon			|service		|protocol	| type		| device_id	| alias	| value 		|generated_measures	|
			| temperature			|servicemqtt	|IoTMqtt	| Quantity	| mqtt10	| t		| 35			|<alias>:<value>	|
			| presence				|servicemqtt	|IoTMqtt	| Boolean	| mqtt11	| p		| false			|<alias>:<value>	|
			| location				|servicemqtt	|IoTMqtt	| Location	| mqtt12	| l		| 125.9,3.1		|<alias>:<value>	|
			| alarm					|servicemqtt	|IoTMqtt	| Text		| mqtt13	| a		| Danger		|<alias>:<value>	|
			| temperaturainterior	|servicemqtt	|IoTMqtt	| Quantity	| mqtt14	| tint	| -2.5			|<alias>:<value>	|			
	

	@iot_mqtt @IDAS-19969
	Scenario Outline: Send one wrong measure
		Given a Service with name "<service>" and protocol "<protocol>" created
		When I publish a MQTT message with device_id "<device_id>", alias "<alias>", payload "<value>" and wrong field "<field>" 
		And I Wait some time
		Then the measure of asset "<device_id>" with measures "<generated_measures>" is received or NOT by context broker
		
		Examples:
			| phenomenon	|service		|protocol	| type		| device_id	| alias	| value	| field		|generated_measures	|
			| temperature	|servicemqtt	|IoTMqtt	| Quantity	| mqtt15	| t		| 5		| apikey	|					|
			| temperature	|servicemqtt	|IoTMqtt	| Quantity	| 			| t		| 35	| device_id	|					|
			| presence		|servicemqtt	|IoTMqtt	| Boolean	| mqtt16	| 		| false	| alias		|					|
			| location		|servicemqtt	|IoTMqtt	| Location	| mqtt17	| l		| 		| payload	|<alias>:void		|
#			| temperature	|servicemqtt	|IoTMqtt	| Quantity	| mqtt19	| t		| 90	| topic		|					|

	
	@iot_mqtt @IDAS-18329	
	Scenario Outline: Send several measures
		Given a Service with name "<service>" and protocol "<protocol>" created
		When I publish a MQTT message with device_id "<device_id>", tag "<tag>" and payload "<alias1>|<value1>#<alias2>|<value2>#<alias3>|<value3>#<alias4>|<value4>"
		And I Wait some time
		Then "<num_measures>" measures of asset "<device_id>" are received by context broker
			|generated_measures |
			|<alias1>:<value1>	|
			|<alias2>:<value2>	|
			|<alias3>:<value3>	|
			|<alias4>:<value4>	|
			
		Examples:
			|device_id		|service		|protocol	| tag	| value1	| alias1	| value2	| alias2		| value3	| alias3	| value4	| alias4	| num_measures	|
			|mqtt20			|servicemqtt	|IoTMqtt	| mul20	| bye		| a			| 53.2		| t				| false		| p			| 5.4,-4.1	| l			| 4				|
			|mqtt21			|servicemqtt	|IoTMqtt	| mul20	| 7.8		| t			| 3.2		| t				| hello		| alarm		| bye		| alarm		| 4				|
#			|AssetCompleto	| multi	| hello		| alarm		| 27.9		| temperature	| true		| presence	| -1.3,2.4	| location	| 4				|


    @iot_mqtt @IDAS-20395
    Scenario Outline: Send a single observation for provisioned device
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_id>", protocol "<protocol>", entity type "<ent_type>" and entity name "<ent_name>" created
		When I publish a MQTT message with device_id "<device_id>", alias "<alias>" and payload "<value>"
		And I Wait some time
		Then the measure of asset "<device_id>" with entity_type "<ent_type>", entity_name "<ent_name>" and measures "<generated_measures>" is received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value		|ent_type	|ent_name	|generated_measures	|
            |device_mqtt23	|servicemqtt	|IoTMqtt	|t		|70			|thing13	|room13		|<alias>:<value>	|
            |device_mqtt24	|servicemqtt	|IoTMqtt	|t		|80			|			|room14		|<alias>:<value>	|
            |device_mqtt25	|servicemqtt	|IoTMqtt	|t		|90			|thing9		|			|<alias>:<value>	|
            |device_mqtt26	|servicemqtt	|IoTMqtt	|t		|100		|			|			|<alias>:<value>	|


    @iot_mqtt @IDAS-20396
    Scenario Outline: Send a single observation for provisioned device with attributes
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_id>", protocol "<protocol>", atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value1>" and "<value2>" created
		When I publish a MQTT message with device_id "<device_id>", alias "<alias>" and payload "<value>"
		And I Wait some time
		Then the measure of asset "<device_id>" with measures "<generated_measures>" and attributes are received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value	|typ		|name	|type	|value1	|typ2		|name2	|type2	|value2		|generated_measures	|
            |device_mqtt27	|servicemqtt	|IoTMqtt	|t		|10		|dev_attr	|temp1	|int	|t1		|dev_attr	|hum	|int2	|h			|<alias>:<value>	|
            |device_mqtt27	|servicemqtt	|IoTMqtt	|t1		|20		|dev_attr	|temp1	|int	|t1		|dev_attr	|hum	|int2	|h			|<alias>:<value>	|
            |device_mqtt27	|servicemqtt	|IoTMqtt	|h		|90		|dev_attr	|temp1	|int	|t1		|dev_attr	|hum	|int2	|h			|<alias>:<value>	|
            |device_mqtt28	|servicemqtt	|IoTMqtt	|t		|11		|dev_attr	|temp1	|int	|t1		|dev_st_att	|h		|int2	|100		|<alias>:<value>	|
            |device_mqtt28	|servicemqtt	|IoTMqtt	|t1		|21		|dev_attr	|temp1	|int	|t1		|dev_st_att	|h		|int2	|100		|<alias>:<value>	|
            |device_mqtt29	|servicemqtt	|IoTMqtt	|t		|12		|dev_st_att	|t1		|int	|30		|dev_st_att	|h		|int2	|100		|<alias>:<value>	|
			