Feature: Delete Data Device
    	 As an application
    	 I want to delete devices data
    	 In order to manage Devices into IotAgent
    	
    	@iot_api @IDAS-18463 
    	Scenario Outline: Delete device data
		Given a Service with name "<srv_name>" and path "<srv_path>" created
		And a Device with name "<dev_id>", entity_name "<ent_name>" and entity_type "<ent_type>" created
		When I delete the device "<dev_id_del>"
		Then the Device with name "<dev_id_del>" is deleted
		
		Examples:
			|srv_name		|srv_path			|dev_id		|ent_name	|ent_type	|dev_id_del	|
			|srv_dev_del	|/path_srv_dev_del	|dev_del	|room_del	|thing_del	|dev_del	|	
			|srv_dev_del	|void				|dev_del	|room_del	|thing_del	|dev_del	|	
			|srv_dev_del	|/path_srv_dev_del	|dev_del1	|room_del	|thing_del	|dev_del2	|	


    	@iot_api @IDAS-18467 
    	Scenario Outline: Delete device data with invalid fields
		When I try to delete the device "<dev_id>" with service name "<srv_name>" and path "<srv_path>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path			|dev_id		|HTTP_status	|exceptionText 												|
			|void			|/path_srv_dev_del	|dev_del1	|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|srv_dev_del	|path_srv_dev_del	|dev_del1	|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|srv_dev_del	|/path_srv_dev_del	|			|404			|															|
			