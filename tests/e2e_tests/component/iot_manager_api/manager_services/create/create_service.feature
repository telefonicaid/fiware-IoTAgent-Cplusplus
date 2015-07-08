Feature: Manager API Service Create
    	 As an application
    	 I want to create services
    	 In order to include Services into IotAgent Manager
    	
    	@iot_manager_api @IDAS-20485 
    	Scenario Outline: Create a service
		Given a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>" and apikey "<apikey>" not created
		When I create a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>", apikey "<apikey>", cbroker "<cbroker>", entity_type "<entity_type>" and token "<token>"
        Then the Service with name "<srv_name>" and path "<srv_path>" is created
		
		Examples:
			|srv_name	|srv_path			|protocol	|apikey			 |cbroker			|entity_type	|token		|
			|srvm_post	|void				|IoTUL2		|srvm_postkey	 |http://myurl:80	|				|			|
			|srvm_post1	|/path_srvm_post1	|IoTUL2		|srvm_postkey1	 |http://myurl:80	|				|			|
			|srvm_post2	|/path_srvm_post2	|IoTUL2		|srvm_postkey2	 |http://myurl:80	|my_thing		|my_token	|
			|srvm_post3	|void				|IoTUL2		|				 |http://myurl:80	|				|			|
			|srvm_post1	|/path_srvm_post1	|IoTTT		|srvm_postkey1	 |http://myurl:80	|				|			|
			|srvm_post1	|/path_srvm_post1_2	|IoTTT		|srvm_postkey1	 |http://myurl:80	|				|			|
			|srvm_post2	|/path_srvm_post2	|IoTTT		|srvm_postkey2_2 |http://myurl:80	|				|			|
			|srvm_post1	|/path_srvm_post1_3	|IoTTT		|srvm_postkey1_3 |http://myurl:80	|				|			|
			|srvm_post4	|/path_srvm_post1_3	|IoTTT		|srvm_postkey4	 |http://myurl:80	|				|			|
			|srvm_post5	|void				|IoTUL2		|				 |http://myurl:80	|				|			|
			|srvm_post5	|/path_srvm_post5	|IoTUL2		|				 |http://myurl:80	|				|			|
			|srvm_post5	|/path_srvm_post5	|IoTTT	 	|				 |http://myurl:80	|				|			|

		
		@iot_manager_api @IDAS-20486 
    	Scenario Outline: Create a service with invalid fields
		When I try to create a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>", apikey "<apikey>" and cbroker "<cbroker>"
        Then user receives the "<HTTP_code>" and the "<exceptionText>"
		
		Examples:
			|srv_name	|srv_path			|protocol	|apikey			 |cbroker			| HTTP_code	|exceptionText 												|
			|srvm_post6	|void				|IoTUL2		|null			 |http://myurl:80	| 400		|Missing required property: apikey							|
			|srvm_post6	|void				|null		|srvm_postkey6	 |http://myurl:80	| 400		|Missing required property: protocol						|
			|srvm_post6	|void				|void		|srvm_postkey6	 |http://myurl:80	| 400		|Array is to short (0), minimum 1 [/services[0]/protocol]	|
			|srvm_post6	|void				|IoTUL2		|srvm_postkey6	 |					| 400		|String is too short (0 chars), minimum 1					|
			|srvm_post6	|void				|IoTUnknown	|srvm_postkey6	 |http://myurl:80	| 400		|No exists protocol IoTUnknown			 					|
			|srvm_post6	|void				|IoTUL2		|srvm_postkey6	 |http:/wrongurl:80	| 500		|A parameter of the request is invalid/not allowed[{}]		|
			|void		|void				|IoTUL2		|srvm_postkey6	 |http://myurl:80	| 400		|Fiware-Service not accepted								|
			|srvm_post6	|path_srvm_post6	|IoTUL2		|srvm_postkey6	 |http://myurl:80	| 400		|Fiware-ServicePath not accepted							|
			|srvm_post5	|/path_srvm_post5	|IoTUL2		|				 |http://myurl:80	| 500		|duplicate key: iot.SERVICE 								|
			|srvm_post1	|/path_srvm_post1	|IoTUL2		|srvm_postkey1	 |http://myurl:80	| 500		|duplicate key: iot.SERVICE 								|
			|srvm_post2	|/path_srvm_post2	|IoTUL2		|srvm_postkey2_3 |http://myurl:80	| 500		|duplicate key: iot.SERVICE 								|


    	@iot_manager_api @IDAS-20487 
    	Scenario Outline: Create a service with attributes
		Given a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>" and apikey "<apikey>" not created
		When I create a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>", apikey "<apikey>", cbroker "<cbroker>" and atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value>" and "<value2>"
        Then the Service with name "<srv_name>" and path "<srv_path>" is created
		
		Examples:
			|srv_name	|srv_path			|protocol	|apikey			|cbroker			|typ		|name		|type	|value	|typ2		|name2		|type2	|value2		|
			|srvm_post8	|/path_srvm_post8	|IoTUL2		|srvm_postkey8	|http://myurl:80	|srv_attr	|t			|int	|temp	|			|			|		|			|
			|srvm_post8	|/path_srvm_post8_0	|IoTTT		|srvm_postkey8	|http://myurl:80	|srv_attr	|t1			|int	|temp1	|srv_attr	|h			|int	|hum		|
			|srvm_post8	|/path_srvm_post8_1	|IoTMqtt	|srvm_postkey8	|http://myurl:80	|srv_st_att	|t2			|int	|22		|			|			|		|			|
			|srvm_post8	|/path_srvm_post8_2	|IoTUL2		|srvm_postkey8	|http://myurl:80	|srv_st_att	|t3			|int	|33		|srv_st_att	|h3			|int	|333		|
			|srvm_post8	|/path_srvm_post8_3	|IoTUL2		|srvm_postkey8	|http://myurl:80	|srv_attr	|t6			|int	|temp6	|srv_st_att	|h6			|int	|666		|
			