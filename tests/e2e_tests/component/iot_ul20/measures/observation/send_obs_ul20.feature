Feature: Send UL20 Observation	
    	 As a device
    	 I want to send measures to context broker throught IoTAgent for UL20
    	 In order to assure IotAgent_UL20-Cbroker integration
    	

    	@iot_ul20 @IDAS-18478
    	Scenario Outline: Send a single observation
		Given a service with name "<service>" and protocol "<protocol>" created
		When I send a measure to the GW with apikey, id "<device_name>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_name>" with phenom "<alias>" and value "<value>" is received by context broker
		
		Examples:
            |device_name |service		|protocol	|alias	|value		|timestamp				|
            |device_ul1	 |serviceul20	|IoTUL2		|t		|23			|						|
            |device_ul2	 |serviceul20	|IoTUL2		|a		|stop		|						|
            |device_ul3  |serviceul20	|IoTUL2		|p		|false		|						|
            |device_ul4	 |serviceul20	|IoTUL2		|l		|23.1/2.3	|						|
            |device_ul5  |serviceul20	|IoTUL2		|t		|35			|2014-11-07T10:22:03Z	|
            

        @iot_ul20 @IDAS-18480
    	Scenario Outline: Send a multiple observation
		Given a service with name "<service>" and protocol "<protocol>" created
		When I send several measures to the GW with apikey, id "<device_name>", protocol "<protocol>" 
			|value 		|alias		|
			|<valuea>	|<aliasa>	|
			|<valuet>	|<aliast>	|
			|<valuep>	|<aliasp>	|
			|<valuel>	|<aliasl>	|
		Then "<num_measures>" measures of asset "<device_name>" are received by context broker
			|value 		|alias		|
			|<valuea>	|<aliasa>	|
			|<valuet>	|<aliast>	|
			|<valuep>	|<aliasp>	|
			|<valuel>	|<aliasl>	|
		
		Examples:
			|device_name	|service		|protocol	|aliast	|valuet	|aliasa	|valuea	|aliasp	|valuep	|aliasl	|valuel		|num_measures |
            |device_ul6		|serviceul20	|IoTUL2		|t		|25		|a		|stop	|p		|false	|l		|25.1/2.5	|4			  |
            |device_ul7		|serviceul20	|IoTUL2		|t		|25		|a		|stop	|p		|false	|t		|99			|4			  |


    	@iot_ul20 @IDAS-20047
    	Scenario Outline: Send a wrong observation
		Given a service with name "<service>" and protocol "<protocol>" created
		When I send a wrong measure to the GW with apikey, id "<device_name>", protocol "<protocol>", alias "<alias>", value "<value>", timestamp "<timestamp>" and wrong field "<field>" 
		Then the measure of asset "<device_name>" with phenom "<alias>" and value "<value>" is received or NOT by context broker
		
		Examples:
            |device_name |service		|protocol	|alias	|value		|field			|timestamp				|
            |device_ul8	 |serviceul20	|IoTUL2		|t		|23			|not_apikey		|						|
            |device_ul9	 |serviceul20	|IoTUL2		|a		|stop		|not_device		|						|
            |device_ul10 |serviceul20	|IoTUL2		|p		|false		|wrong_apikey	|						|
            |device_ul11 |serviceul20	|IoTUL2		|l		|			|not_value		|						|
            |device_ul12 |serviceul20	|IoTUL2		|		|23			|not_alias		|						|
            |device_ul13 |serviceul20	|IoTUL2		|t		|55			|timestamp		|2014-11-31T10:22:03Z	|


    	@iot_ul20 @IDAS-20172
    	Scenario Outline: Send a single observation for provisioned device
		Given a service with name "<service>" and protocol "<protocol>" created
		And a device with device id "<device_id>", entity type "<ent_type>" and entity name "<ent_name>" created
		When I send a measure to the GW with apikey, id "<device_id>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_id>" with phenom "<alias>", entity_type "<ent_type>", entity_name "<ent_name>" and value "<value>" is received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value		|ent_type	|ent_name	|timestamp	|
            |device_ul13	|serviceul20	|IoTUL2		|t		|70			|thing13	|room13		|			|
            |device_ul14	|serviceul20	|IoTUL2		|t		|80			|			|room14		|			|
            |device_ul15	|serviceul20	|IoTUL2		|t		|90			|thing9		|			|			|
            |device_ul16	|serviceul20	|IoTUL2		|t		|100		|			|			|			|


    	@iot_ul20 @IDAS-20172
    	Scenario Outline: Send a single observation for provisioned device with attributes
		Given a service with name "<service>" and protocol "<protocol>" created
		And a device with device id "<device_id>", atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value1>" and "<value2>" created
		When I send a measure to the GW with apikey, id "<device_id>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_id>" with phenom "<alias>" and value "<value>" and attributes are received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value	|typ	|name	|type	|value1	|typ2	|name2		|type2	|value2		|timestamp	|
            |device_ul17	|serviceul20	|IoTUL2		|t		|10		|attr	|temp1	|int	|t1		|attr	|hum		|int2	|h			|			|
            |device_ul17	|serviceul20	|IoTUL2		|t1		|20		|attr	|temp1	|int	|t1		|attr	|hum		|int2	|h			|			|
            |device_ul17	|serviceul20	|IoTUL2		|h		|90		|attr	|temp1	|int	|t1		|attr	|hum		|int2	|h			|			|
            |device_ul18	|serviceul20	|IoTUL2		|t		|11		|attr	|temp1	|int	|t1		|st_att	|h			|int2	|100		|			|
            |device_ul18	|serviceul20	|IoTUL2		|t1		|21		|attr	|temp1	|int	|t1		|st_att	|h			|int2	|100		|			|
            |device_ul19	|serviceul20	|IoTUL2		|t		|12		|st_att	|t1		|int	|30		|st_att	|h			|int2	|100		|			|
                                    