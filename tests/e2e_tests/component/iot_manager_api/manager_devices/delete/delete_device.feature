Feature: Manager API Device Delete 
    	 As an application
    	 I want to delete devices data
    	 In order to manage Devices into IotAgent Manager
    	
    	@iot_manager_api @IDAS-20468 
    	Scenario Outline: Delete device data
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>", protocol "<protocol>", entity_name "<ent_name>" and entity_type "<ent_type>" created
		When I delete in manager the device "<dev_id_del>"
		Then the Device with name "<dev_id_del>" is deleted
		
		Examples:
			|srv_name		|srv_path			 |dev_id	|ent_name	|ent_type	|dev_id_del	|protocol	|
			|srv_devm_del	|/path_srv_devm_del	 |devm_del	|room_del	|thing_del	|devm_del	|IoTUL2		|	
			|srv_devm_del	|void				 |devm_del	|room_del	|thing_del	|devm_del	|IoTTT		|	
			|srv_devm_del	|/path_srv_devm_del1 |devm_del1	|room_del	|thing_del	|devm_del2	|IoTMqtt	|	


    	@iot_manager_api @IDAS-20469 
    	Scenario Outline: Delete device data with invalid fields
		When I try to delete the device "<dev_id>" with service name "<srv_name>", protocol "<protocol>" and path "<srv_path>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path				|dev_id		|protocol	|HTTP_status	|exceptionText 								|
			|void			|/path_srv_devm_del		|devm_del1	|IoTUL2		|400			|Fiware-Service not accepted				|
			|srv_devm_del	|path_srv_devm_del		|devm_del1	|IoTUL2		|400			|Fiware-ServicePath not accepted			|
			|srv_devm_del	|/path_srv_devm_del		|			|IoTUL2		|404			|											|
			|srv_devm_del	|/path_srv_devm_del1	|devm_del1	|IoTTT		|404			|there aren't iotagents for this operation	|
			|srv_devm_del	|/path_srv_devm_del1	|devm_del1	|IoTUnknow	|404			|there aren't iotagents for this operation	|
			|srv_devm_del	|/path_srv_devm_del1	|devm_del1	|			|400			|protocol parameter is mandatory			|
			