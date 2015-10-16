Feature: API Device Update
    	 As an application
    	 I want to update devices data
    	 In order to manage Devices into IotAgent
    	
    	@iot_api @IDAS-20333 
    	Scenario Outline: Update data into device
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>", entity_name "<ent_name>", entity_type "<ent_type>", endpoint "<endpoint>", protocol "<protocol>" and atribute or command "<typ>", with name "<name>", type "<type>" and value "<value>" created
		When I update the attribute "<attribute>" of device "<dev_id>" with value "<attr_value>"
        Then the device data contains attribute "<attribute>" with value "<attr_value>"
		
		Examples:
			|srv_name		|srv_path			|dev_id		|ent_name	|ent_type	|endpoint			|protocol	|typ		|name		|type	|value	|attribute		|attr_value			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put	|room_put	|thing_put	|http://myurl:10	|IoTUL2		|dev_attr	|t			|int	|temp	|entity_name	|room_put1			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put	|room_put1	|thing_put	|http://myurl:10	|IoTUL2		|dev_attr	|t			|int	|temp	|entity_type	|thing_put1			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put	|room_put1	|thing_put1	|http://myurl:10	|IoTUL2		|dev_attr	|t			|int	|temp	|endpoint		|http://myurl:100	|
			|srv_dev_put	|/path_srv_dev_put	|dev_put	|room_put1	|thing_put1	|http://myurl:100	|IoTUL2		|dev_attr	|t			|int	|temp	|device_id		|dev_put1			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|room_put1	|thing_put1	|http://myurl:100	|IoTUL2		|dev_attr	|t			|int	|temp	|dev_attr		|h#str:hum			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|room_put1	|thing_put1	|http://myurl:100	|IoTUL2		|dev_attr	|h			|str	|hum	|dev_st_att		|t1#int:20			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|room_put1	|thing_put1	|http://myurl:100	|IoTUL2		|dev_attr	|h			|str	|hum	|dev_st_att		|t2#int:200			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|room_put1	|thing_put1	|http://myurl:100	|IoTUL2		|dev_st_att	|t2			|int	|200	|cmd			|cmd_dev#cmd:ping	|
			|srv_dev_put	|/path_srv_dev_put	|dev_put2	|room_put2	|thing_put2	|http://myurl:200	|IoTUL2		|dev_attr	|t2			|int	|temp2	|cmd			|cmd_dev2#cmd:ping	|
			|srv_dev_put	|/path_srv_dev_put	|dev_put2	|room_put2	|thing_put2	|http://myurl:200	|IoTUL2		|dev_attr	|t2			|int	|temp2	|cmd			|cmd_dev2#cmd:ping2	|
			|srv_dev_put	|/path_srv_dev_put	|dev_put2	|room_put2	|thing_put2	|http://myurl:200	|IoTUL2		|cmd		|cmd_dev2	|		|ping2	|dev_st_att		|t3#int:300			|
			|srv_dev_put	|void				|dev_put	|room_put	|thing_put	|http://myurl:10	|IoTUL2		|dev_attr	|t			|int	|temp	|dev_st_att		|t1#int:20			|

    	@iot_api @IDAS-20334 
    	Scenario Outline: Update device data with invalid fields
		When I try to update the device data of device "<dev_id>" with service "<srv_name>" and path "<srv_path>" with the attribute "<attribute>" and value "<attr_value>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path			|dev_id		|attribute		|attr_value	|HTTP_status	|exceptionText 									|
			|void			|void				|dev_put1	|entity_name	|room_put1	|400			|Fiware-Service not accepted					|
			|srv_dev_put	|path_srv_dev_put	|dev_put1	|entity_name	|room_put1	|400			|Fiware-ServicePath not accepted				|
			|srv_dev_put	|/path_srv_dev_put1	|dev_put1	|entity_name	|room_put1	|404			|The device does not exist						|
			|srv_dev_put	|/path_srv_dev_put	|dev_put3	|entity_name	|room_put3	|404			|The device does not exist						|
#			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|entity_name	|room_put2	|409			|entity already exists							|
			|srv_dev_put	|/path_srv_dev_put	|			|entity_name	|room_put1	|404			|												|
			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|				|			|400			|Additional properties not allowed. [/]			|
			|srv_dev_put	|/path_srv_dev_put	|dev_put1	|empty_json		|			|400			|empty body										|
			
