Feature: API Device Get Data
    	 As an application
    	 I want to retrieve a device
    	 In order to include Devices into IotAgent
    	
    	@iot_api @IDAS-20329 
    	Scenario Outline: Retrieve device data
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>", entity_name "<ent_name>", entity_type "<ent_type>", endpoint "<endpoint>", protocol "<protocol>" and atribute or command "<typ>", with name "<name>", type "<type>" and value "<value>" created
		When I retrieve the device data of "<dev_id>"
        Then I receive the device data of "<dev_id>"
		
		Examples:
			|srv_name		|srv_path			|dev_id		|ent_name	|ent_type	|endpoint			|protocol	|typ		|name		|type	|value	|
			|srv_dev_get	|/path_srv_dev_get	|dev_get	|room_get	|thing_get	|http://myurl:10	|IoTUL2		|dev_attr	|t			|int	|temp	|	
			|srv_dev_get	|void				|dev_get	|room_get	|thing_get	|http://myurl:10	|IoTTT		|dev_attr	|t			|int	|temp	|	
			|srv_dev_get	|/path_srv_dev_get1	|dev_get1	|room_get1	|thing_get1	|http://myurl:11	|IoTMqtt	|dev_st_att	|t			|int	|11		|	
			|srv_dev_get	|/path_srv_dev_get	|dev_get2	|room_get2	|thing_get2	|http://myurl:12	|IoTUL2		|cmd		|cmd_dev	|		|ping	|	

    	@iot_api @IDAS-20330 
    	Scenario Outline: Retrieve device data with invalid fields
		When I try to retrieve the device data of device "<dev_id>" with service "<srv_name>" and path "<srv_path>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path			|dev_id		|HTTP_status	|exceptionText 						|
			|void			|void				|dev_get	|400			|Fiware-Service not accepted		|
			|srv_dev_get	|path_srv_dev_get	|dev_get	|400			|Fiware-ServicePath not accepted	|
			|srv_dev_get	|/path_srv_dev_get2	|dev_get	|404			|The device does not exist			|
			|srv_dev_get	|/path_srv_dev_get	|dev_get3	|404			|The device does not exist			|
			|srv_dev_get	|/path_srv_dev_get	|			|404			|									|
			