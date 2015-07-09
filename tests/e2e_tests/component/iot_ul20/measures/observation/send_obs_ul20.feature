Feature: UL20 Observation Send
    	 As a device
    	 I want to send measures to context broker throught IoTAgent for UL20
    	 In order to assure IotAgent_UL20-Cbroker integration
    	

    	@iot_ul20 @IDAS-18478
    	Scenario Outline: Send a single observation
		Given a Service with name "<service>" and protocol "<protocol>" created
		When I send a measure to the GW with apikey, id "<device_id>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_id>" with measures "<generated_measures>" and timestamp "<timestamp>" is received by context broker
		
		Examples:
            |device_id	|service		|protocol	|alias	|value		|timestamp				|generated_measures	|
            |device_ul1	|serviceul20	|IoTUL2		|t		|23			|						|<alias>:<value>	|
            |device_ul2	|serviceul20	|IoTUL2		|a		|stop		|						|<alias>:<value>	|
            |device_ul3 |serviceul20	|IoTUL2		|p		|false		|						|<alias>:<value>	|
            |device_ul4	|serviceul20	|IoTUL2		|l		|23.1/2.3	|						|<alias>:<value>	|
            |device_ul5 |serviceul20	|IoTUL2		|t		|35			|2014-11-07T10:22:03Z	|<alias>:<value>	|
            

        @iot_ul20 @IDAS-18480
    	Scenario Outline: Send a multiple observation
		Given a Service with name "<service>" and protocol "<protocol>" created
		When I send several measures to the GW with apikey, id "<device_name>", protocol "<protocol>" 
			|value 		|alias		|
			|<valuea>	|<aliasa>	|
			|<valuet>	|<aliast>	|
			|<valuep>	|<aliasp>	|
			|<valuel>	|<aliasl>	|
		Then "<num_measures>" measures of asset "<device_name>" are received by context broker
			|generated_measures |
			|<aliasa>:<valuea>	|
			|<aliast>:<valuet>	|
			|<aliasp>:<valuep>	|
			|<aliasl>:<valuel>	|
		
		Examples:
			|device_name	|service		|protocol	|aliast	|valuet	|aliasa	|valuea	|aliasp	|valuep	|aliasl	|valuel		|num_measures |
            |device_ul6		|serviceul20	|IoTUL2		|t		|25		|a		|stop	|p		|false	|l		|25.1/2.5	|4			  |
            |device_ul7		|serviceul20	|IoTUL2		|t		|25		|a		|stop	|p		|false	|t		|99			|4			  |


    	@iot_ul20 @IDAS-20047
    	Scenario Outline: Send a wrong observation
		Given a Service with name "<service>" and protocol "<protocol>" created
		When I send a wrong measure to the GW with apikey, id "<device_name>", protocol "<protocol>", alias "<alias2>", value "<value>", timestamp "<timestamp>" and wrong field "<field>" 
		Then the measure of asset "<device_name>" with measures "<generated_measures>" is received or NOT by context broker
		
		Examples:
            |device_name |service		|protocol	|alias2	|value		|field			|timestamp				|generated_measures	|
            |device_ul8	 |serviceul20	|IoTUL2		|t		|23			|not_apikey		|						|					|
            |device_ul9	 |serviceul20	|IoTUL2		|a		|stop		|not_device		|						|					|
            |device_ul10 |serviceul20	|IoTUL2		|p		|false		|wrong_apikey	|						|					|
            |device_ul11 |serviceul20	|IoTUL2		|l		|			|not_value		|						|					|
            |device_ul12 |serviceul20	|IoTUL2		|		|23			|not_alias		|						|					|
            |device_ul13 |serviceul20	|IoTUL2		|t		|55			|timestamp		|2014-11-31T10:22:03Z	|<alias2>:<value>	|


    	@iot_ul20 @IDAS-20172
    	Scenario Outline: Send a single observation for provisioned device
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_id>", protocol "<protocol>", entity type "<ent_type>" and entity name "<ent_name>" created
		When I send a measure to the GW with apikey, id "<device_id>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_id>" with entity_type "<ent_type>", entity_name "<ent_name>" and measures "<generated_measures>" is received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value		|ent_type	|ent_name	|timestamp	|generated_measures	|
            |device_ul14	|serviceul20	|IoTUL2		|t		|70			|thing14	|room14		|			|<alias>:<value>	|
            |device_ul15	|serviceul20	|IoTUL2		|t		|80			|			|room15		|			|<alias>:<value>	|
            |device_ul16	|serviceul20	|IoTUL2		|t		|90			|thing16	|			|			|<alias>:<value>	|
            |device_ul17	|serviceul20	|IoTUL2		|t		|100		|			|			|			|<alias>:<value>	|


    	@iot_ul20 @IDAS-20358
    	Scenario Outline: Send a single observation for provisioned device with attributes
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_id>", protocol "<protocol>", atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value1>" and "<value2>" created
		When I send a measure to the GW with apikey, id "<device_id>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_id>" with measures "<generated_measures>" and attributes are received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value	|typ		|name	|type	|value1	|typ2		|name2		|type2	|value2		|timestamp	|generated_measures	|
            |device_ul18	|serviceul20	|IoTUL2		|t		|10		|dev_attr	|temp1	|int	|t1		|dev_attr	|hum		|int2	|h			|			|<alias>:<value>	|
            |device_ul18	|serviceul20	|IoTUL2		|t1		|20		|dev_attr	|temp1	|int	|t1		|dev_attr	|hum		|int2	|h			|			|<alias>:<value>	|
            |device_ul18	|serviceul20	|IoTUL2		|h		|90		|dev_attr	|temp1	|int	|t1		|dev_attr	|hum		|int2	|h			|			|<alias>:<value>	|
            |device_ul19	|serviceul20	|IoTUL2		|t		|11		|dev_attr	|temp1	|int	|t1		|dev_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul19	|serviceul20	|IoTUL2		|t1		|21		|dev_attr	|temp1	|int	|t1		|dev_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul20	|serviceul20	|IoTUL2		|t		|12		|dev_st_att	|t1		|int	|30		|dev_st_att	|h			|int2	|100		|			|<alias>:<value>	|


    	@iot_ul20 @IDAS-20399
    	Scenario Outline: Send a single observation for provisioned service and device with attributes
		Given a service with name "<service>", protocol "<protocol>" and atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value1>" and "<value2>" created
		And a device with name "<device_id>", protocol "<protocol>", atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value1>" and "<value2>" created
		When I send a measure to the GW with apikey, id "<device_id>", protocol "<protocol>", alias "<alias>", timestamp "<timestamp>" and value "<value>" 
		Then the measure of asset "<device_id>" with measures "<generated_measures>" and attributes are received by context broker
		
		Examples:
            |device_id 		|service		|protocol	|alias	|value	|typ		|name	|type	|value1	|typ2		|name2		|type2	|value2		|timestamp	|generated_measures	|
            |device_ul21	|serviceul20_0	|IoTUL2		|t		|10		|srv_attr	|temp1	|int	|t1		|srv_attr	|hum		|int2	|h			|			|<alias>:<value>	|
            |device_ul21	|serviceul20_0	|IoTUL2		|t1		|20		|srv_attr	|temp1	|int	|t1		|srv_attr	|hum		|int2	|h			|			|<alias>:<value>	|
            |device_ul21	|serviceul20_0	|IoTUL2		|h		|90		|srv_attr	|temp1	|int	|t1		|srv_attr	|hum		|int2	|h			|			|<alias>:<value>	|
            |device_ul22	|serviceul20_1	|IoTUL2		|t		|11		|srv_attr	|temp1	|int	|t1		|srv_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul22	|serviceul20_1	|IoTUL2		|t1		|21		|srv_attr	|temp1	|int	|t1		|srv_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul23	|serviceul20_2	|IoTUL2		|t		|12		|srv_st_att	|t1		|int	|30		|srv_st_att	|h			|int2	|200		|			|<alias>:<value>	|
            |device_ul24	|serviceul20_0	|IoTUL2		|t		|31		|dev_attr	|temp2	|int	|t2		|dev_attr	|hum2		|int2	|h			|			|<alias>:<value>	|
            |device_ul24	|serviceul20_0	|IoTUL2		|t1		|32		|srv_attr	|temp1	|int	|t1		|dev_attr	|hum2		|int2	|h			|			|<alias>:<value>	|
            |device_ul24	|serviceul20_0	|IoTUL2		|t2		|33		|dev_attr	|temp2	|int	|t2		|dev_attr	|hum2		|int2	|h			|			|<alias>:<value>	|
            |device_ul24	|serviceul20_0	|IoTUL2		|h		|91		|dev_attr	|temp2	|int	|t2		|dev_attr	|hum2		|int2	|h			|			|<alias>:<value>	|
            |device_ul25	|serviceul20_1	|IoTUL2		|t		|41		|dev_attr	|temp2	|int	|t2		|srv_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul25	|serviceul20_1	|IoTUL2		|t1		|42		|srv_attr	|temp1	|int	|t1		|srv_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul25	|serviceul20_1	|IoTUL2		|t2		|43		|dev_attr	|temp2	|int	|t2		|srv_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul26	|serviceul20_1	|IoTUL2		|t		|51		|dev_st_att	|t2		|int	|50		|srv_st_att	|h			|int2	|100		|			|<alias>:<value>	|
            |device_ul27	|serviceul20_1	|IoTUL2		|t		|61		|dev_attr	|temp2	|int	|t1		|dev_st_att	|h			|int2	|300		|			|<alias>:<value>	|
            |device_ul27	|serviceul20_1	|IoTUL2		|t1		|62		|dev_attr	|temp2	|int	|t1		|dev_st_att	|h			|int2	|300		|			|<alias>:<value>	|
            |device_ul28	|serviceul20_2	|IoTUL2		|t		|72		|dev_st_att	|t1		|int	|70		|srv_st_att	|h			|int2	|200		|			|<alias>:<value>	|
            |device_ul29	|serviceul20_2	|IoTUL2		|t		|82		|dev_st_att	|t1		|int	|80		|dev_st_att	|h			|int2	|800		|			|<alias>:<value>	|
                                    