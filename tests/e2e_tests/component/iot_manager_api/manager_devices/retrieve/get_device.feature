Feature: Manager API Device Get Data
    	 As an application
    	 I want to retrieve a device
    	 In order to include Devices into IotAgent Manager
    	
    	@iot_manager_api @IDAS-20448 
    	Scenario Outline: Retrieve device data
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>", entity_name "<ent_name>", entity_type "<ent_type>", endpoint "<endpoint>", protocol "<protocol>" and atribute or command "<typ>", with name "<name>", type "<type>" and value "<value>" created
		When I retrieve in manager the device data of "<dev_id>" with protocol "<find_prot>"
        Then I receive the device data of "<dev_id>"
		
		Examples:
			|srv_name		|srv_path			 |dev_id	|ent_name	|ent_type	|endpoint			|protocol	|typ		|name		|type	|value	|find_prot	|n_devs	|
			|srv_devm_get	|/path_srv_devm_get	 |devm_get	|room_get	|thing_get	|http://myurl:10	|IoTUL2		|dev_attr	|t			|int	|temp	|IoTUL2		|1		|	
			|srv_devm_get	|void				 |devm_get	|room_get	|thing_get	|http://myurl:10	|IoTTT		|dev_attr	|t			|int	|temp	|			|1		|	
			|srv_devm_get	|/path_srv_devm_get1 |devm_get1	|room_get1	|thing_get1	|http://myurl:11	|IoTMqtt	|dev_st_att	|t			|int	|11		|IoTMqtt	|1		|	
			|srv_devm_get	|/path_srv_devm_get	 |devm_get2	|room_get2	|thing_get2	|http://myurl:12	|IoTUL2		|cmd		|cmd_dev	|		|ping	|			|1		|	

    	@iot_manager_api @IDAS-20449 
    	Scenario Outline: Retrieve device data with invalid fields
		When I try to retrieve the device data of device "<dev_id>" with service "<srv_name>", protocol "<protocol>" and path "<srv_path>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path			 |dev_id	|protocol	|HTTP_status	|exceptionText 						|
			|void			|void				 |devm_get	|			|400			|Fiware-Service not accepted		|
			|srv_devm_get	|path_srv_devm_get	 |devm_get	|			|400			|Fiware-ServicePath not accepted	|
			|srv_devm_get	|/path_srv_devm_get2 |devm_get	|			|200			|									|
			|srv_devm_get	|/path_srv_devm_get	 |devm_get3	|			|200			|The device does not exist			|
			|srv_devm_get	|/path_srv_devm_get	 |			|			|404			|The request is not well formed		|
			|srv_devm_get	|/path_srv_devm_get	 |devm_get	|IoTTT		|200			|									|
			|srv_devm_get	|/path_srv_devm_get	 |devm_get	|IoTUnknown	|200			|									|
			