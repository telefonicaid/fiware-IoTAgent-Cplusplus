Feature: Manager API Service Get Data
    	 As an application
    	 I want to retrieve services
    	 In order to include Services into IotAgent Manager
    	
    	@iot_manager_api @IDAS-20419 
    	Scenario Outline: Retrieve service data
    	Given a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>" and apikey "<apikey>" created
		When I retrieve the service data of "<find_srv>", path "<find_path>", protocol "<find_prot>", limit "<limit>" and offset "<offset>"
        Then I receive the service data of "<n_srvs>" services
		
		Examples:
			|srv_name	|srv_path		 |protocol	|apikey			|find_srv	|find_path		 |find_prot	|n_srvs	|limit	|offset	|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/path_srvm_get	 |			|1		|		|		|
			|srvm_get	|/path_srvm_get1 |IoTUL2	|				|srvm_get	|/path_srvm_get1 |			|1		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTTT		|srvm_getkey	|srvm_get	|/path_srvm_get	 |IoTTT		|1		|		|		|
			|srvm_get	|/path_srvm_get1 |IoTTT		|				|srvm_get	|/path_srvm_get1 |IoTTT		|1		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTMqtt	|srvm_getkey	|srvm_get	|/path_srvm_get	 |			|3		|		|		|
			|srvm_get	|/path_srvm_get1 |IoTMqtt	|				|srvm_get	|/path_srvm_get1 |			|3		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/path_srvm_get	 |			|2		|2		|		|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/path_srvm_get	 |			|2		|2		|0		|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/path_srvm_get	 |			|0		|1		|3		|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/path_srvm_get	 |			|2		|3		|1		|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/*				 |			|6		|		|		|
			|srvm_get	|/path_srvm_get2 |IoTMqtt	|srvm_getkey2	|srvm_get	|/*				 |IoTMqtt	|3		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTUL2	|srvm_getkey	|srvm_get	|/*				 |			|3		|4		|4		|
			|srvm_get	|/path_srvm_get3 |IoTMqtt	|srvm_getkey3	|srvm_get	|/*				 |IoTMqtt	|2		|2		|1		|
			|srvm_get	|/path_srvm_get	 |IoTMqtt	|srvm_getkey	|srvm_get2	|/*				 |			|0		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTMqtt	|srvm_getkey	|srvm_get	|/path_srvm_get6 |			|0		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTMqtt	|srvm_getkey	|srvm_get	|/*				 |IoTUnknwn	|0		|		|		|
			|srvm_get	|/path_srvm_get	 |IoTMqtt	|srvm_getkey	|srvm_get	|				 |			|0		|		|		|
			|srvm_get	|void			 |IoTMqtt	|srvm_getkey8	|srvm_get	|				 |			|1		|		|		|


    	@iot_manager_api @IDAS-20420 
    	Scenario Outline: Retrieve service data with invalid fields
		When I try to retrieve the service data of "<srv_name>", path "<srv_path>", protocol "<protocol>", limit "<limit>" and offset "<offset>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name	|srv_path		|protocol	|limit			|offset			|HTTP_status	|exceptionText 									|
			|void		|void			|			|				|				|400			|Fiware-Service not accepted					|
			|void		|/path_srv_get	|			|				|				|400			|Fiware-Service not accepted					|
			|srv_get	|path_srv_get	|			|				|				|400			|Fiware-ServicePath not accepted				|
			|srv_get	|/path_srv_get	|			|wrong_limit	|				|400			|limit must be a number but it is wrong_limit	|
			|srv_get	|/path_srv_get	|			|				|wrong_offset	|400			|offset must be a number but it is wrong_offset	|
			