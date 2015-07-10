Feature: API Service Delete
    	 As an application
    	 I want to delete services data
    	 In order to manage Services into IotAgent
    	
    	@iot_api @IDAS-20323 
    	Scenario Outline: Delete service data
    	Given two Services with name "<srv_name>", paths "<srv_path>" and "<srv_path2>", protocols "<protocol>" and "<protocol2>" and apikey "<apikey>" created
		When I delete the service "<srv_name>" with path "<del_path>"
		Then the Services with name "<srv_name>" and paths "<del_path1>" and "<del_path2>" are deleted or not
		
		Examples:
			|srv_name	|srv_path		|srv_path2			|protocol	|protocol2	|apikey			|del_path		|del_path1	|del_path2	|		
			|srv_del	|/path_srv_del	|					|IoTUL2		|			|srv_delkey		|/path_srv_del	|true		|			|
			|srv_del	|/path_srv_del1	|					|IoTUL2		|			|				|/path_srv_del1	|true		|			|
			|srv_del	|void			|					|IoTUL2		|			|srv_delkey		|void			|true		|			|
			|srv_del	|void			|					|IoTUL2		|			|				|void			|true		|			|			
			|srv_del	|/path_srv_del3	|/path_srv_del3_1	|IoTUL2		|IoTUL2		|srv_delkey		|/*				|true		|true		|
			|srv_del	|/path_srv_del4	|void				|IoTUL2		|IoTUL2		|srv_delkey		|/#				|true		|true		|
			|srv_del	|/path_srv_del5	|void				|IoTUL2		|IoTTT		|srv_delkey		|/*				|true		|false		|
			|srv_del	|/path_srv_del6	|/path_srv_del6		|IoTUL2		|IoTTT		|srv_delkey		|/#				|true		|false		|


    	@iot_api @IDAS-20324 
    	Scenario Outline: Delete service with devices data
    	Given two Services with name "<srv_name>", paths "<srv_path>" and "<srv_path2>", protocols "<protocol>" and "<protocol2>" and apikey "<apikey>" created
    	And devices for services with name "<srv_name>", paths "<srv_path>" and "<srv_path2>" and protocols "<protocol>" and "<protocol2>" created
		When I delete the service "<srv_name>" with param "<device>" and path "<del_path>"
		Then the Services with name "<srv_name>" and paths "<del_path1>" and "<del_path2>" are deleted or not
		And devices "<del_devs>" of services with name "<srv_name>" and paths "<del_path1>" and "<del_path2>" are deleted or not
		
		Examples:
			|srv_name	|srv_path		|srv_path2		|protocol	|protocol2	|apikey			|del_path		|del_path1	|del_path2	|device	|del_devs	|		
			|srv_del2	|/path_srv_del	|				|IoTUL2		|			|srv_delkey2	|/path_srv_del	|true		|			|		|false		|
			|srv_del2	|/path_srv_del1	|				|IoTUL2		|			|srv_delkey2	|/path_srv_del1	|true		|			|true	|true		|
			|srv_del2	|/path_srv_del2	|				|IoTUL2		|			|srv_delkey2	|/path_srv_del2	|true		|			|false	|false		|
			|srv_del3	|/path_srv_del3	|/path_srv_del4	|IoTUL2		|IoTUL2		|srv_delkey2	|/path_srv_del3	|true		|false		|true	|true		|
			|srv_del3	|/path_srv_del5	|/path_srv_del5	|IoTUL2		|IoTTT		|srv_delkey2	|/path_srv_del5	|true		|false		|true	|false		|


    	@iot_api @IDAS-20325 
    	Scenario Outline: Delete service data with invalid fields
		When I try to delete the service "<srv_name>" with path "<srv_path>", resource "<resource>", apikey "<apikey>" and param device "<device>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
        And the Service with name "<srv_name>" and path "<srv_path>" is not deleted
		
		Examples:
			|srv_name	|srv_path		|resource	|apikey			|device	|HTTP_status	|exceptionText 																|
			|void		|/path_srv_del4	|/iot/d		|srv_delkey2	|		|400			|Fiware-Service not accepted												|
			|srv_del3	|path_srv_del4	|/iot/d		|srv_delkey2	|		|400			|Fiware-ServicePath not accepted											|
			|srv_del3	|/path_srv_del4	|			|srv_delkey2	|		|400			|resource parameter is mandatory											|
			|srv_del3	|/*				|/iot/d		|srv_delkey2	|true	|400			|Fiware-ServicePath is invalid in DELETE operation [/*] removing devices	|
			|srv_del3	|/#				|/iot/d		|srv_delkey2	|true	|400			|Fiware-ServicePath is invalid in DELETE operation [/#] removing devices	|
			