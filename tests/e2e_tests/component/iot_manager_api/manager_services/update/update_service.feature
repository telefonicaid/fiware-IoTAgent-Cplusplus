Feature: Manager API Service Update
    	 As an application
    	 I want to update services data
    	 In order to manage Services into IotAgent
    	
    	@iot_manager_api @IDAS-20494 
    	Scenario Outline: Update data into service
    	Given a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>", apikey "<apikey>", cbroker "<cbroker>", entity_type "<entity_type>" and token "<token>" created
		When I update in manager the attribute "<attribute>" of service "<srv_name>" with value "<value>"
        Then the service data of manager contains attribute "<attribute>" with value "<value>"
		
		Examples:
			|srv_name	|srv_path			|protocol	|apikey			|cbroker			|entity_type	|token		|attribute		|value				|
			|srvm_put	|/path_srvm_put		|IoTUL2		|srvm_putkey	|http://myurl:80	|my_thing		|my_token	|cbroker		|http://myurl2:200	|
			|srvm_put	|/path_srvm_put2	|IoTUL2		|srvm_putkey2	|http://myurl:80	|my_thing		|my_token	|entity_type	|my_thing22			|
			|srvm_put	|/path_srvm_put2	|IoTTT		|srvm_putkey2	|http://myurl:90	|my_thing		|my_token	|token			|my_token33			|
			|srvm_put	|/path_srvm_put3	|IoTUL2		|srvm_putkey3	|http://myurl:80	|my_thing		|my_token	|apikey			|srvm_putkey3_1		|
			|srvm_put	|/path_srvm_put4	|IoTUL2		|srvm_putkey4	|http://myurl:80	|my_thing		|my_token	|apikey			|					|
#			|srvm_put	|/path_srvm_put5	|IoTUL2		|srvm_putkey5	|http://myurl:80	|my_thing		|my_token	|protocol		|IoTTT				|
			|srvm_put	|void				|IoTUL2		|srvm_putkey	|http://myurl:80	|my_thing		|my_token	|apikey			|					|
			|srvm_put	|void				|IoTTT		|				|http://myurl:80	|my_thing		|my_token	|cbroker		|http://myurl4:400	|


    	@iot_manager_api @IDAS-20495 
    	Scenario Outline: Update data into service with invalid fields
		When I try to update the attribute "<attribute>" with value "<value>" of service "<srv_name>" with path "<srv_path>", protocol "<protocol>", apikey "<apikey>" and cbroker "<cbroker>"
        Then user receives the "<HTTP_status>", the "<HTTP_code>" and the "<exceptionText>"
        And the service data NOT contains attribute "<attribute>" with value "<value>"
		
		Examples:
			|srv_name	|srv_path		 |protocol	 |apikey		|cbroker			|attribute	|value				|HTTP_status	|exceptionText 									|HTTP_code	|
			|void		|void			 |IoTUL2	 |srvm_putkey	|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Fiware-Service not accepted					|			|
			|void		|/path_srvm_put	 |IoTUL2	 |srvm_putkey	|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Fiware-Service not accepted					|			|
			|srvm_put	|path_srvm_put	 |IoTUL2	 |srvm_putkey	|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Fiware-ServicePath not accepted				|			|
			|srvm_put	|/path_srvm_put	 |IoTTT		 |srvm_putkey	|http://myurl2:200	|cbroker	|http://myurl2:2000	|404			|The service does not exist						|			|
			|srvm_put	|/path_srvm_put	 |IoTUL2	 |srvm_putkey2	|http://myurl2:200	|cbroker	|http://myurl2:2000	|404			|The service does not exist						|			|
			|srvm_put	|/path_srvm_put	 |			 |srvm_putkey	|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Missing required property: protocol			|			|
			|srvm_put	|/path_srvm_put	 |IoTTT		 |				|http://myurl2:200	|cbroker	|http://myurl2:2000	|404			|The service does not exist						|			|
			|srvm_put	|/path_srvm_put3 |IoTTT		 |srvm_putkey2	|http://myurl:80	|cbroker	|http://myurl2:2000	|404			|The service does not exist						|			|
			|srvm_put	|void			 |IoTUnknown |				|http://myurl:80	|cbroker	|http://myurl2:2000	|400			|No exists protocol IoTUnknown					|			|
			|srvm_put	|/path_srvm_put2 |void		 |srvm_putkey2	|http://myurl:80	|cbroker	|http://myurl2:2000	|400			|Array is to short (0), minimum 1 				|			|
			|srvm_put	|/path_srvm_put2 |			 |srvm_putkey2	|http://myurl:80	|protocol	|					|400			|invalid data type: StringType expected: array	|			|
			|srvm_put	|/path_srvm_put2 |IoTUL2	 |srvm_putkey2	|http://myurl:80	|empty_json	|					|404			|empty body										|400		|
			|srvm_put	|/path_srvm_put2 |			 |srvm_putkey2	|http://myurl:80	|empty_json	|					|400			|Missing required property: protocol			|			|


    	@iot_manager_api @IDAS-20496 
    	Scenario Outline: Update data into service with attributes
    	Given a Service with name "<srv_name>", path "<srv_path>", protocol "<protocol>", apikey "<apikey>", cbroker "<cbroker>" and atribute "<typ>", with name "<name>", type "<type>" and value "<value>" created
		When I update in manager the attribute "<attribute>" of service "<srv_name>" with value "<attr_value>"
        Then the service data of manager contains attribute "<attribute>" with value "<attr_value>"
		
		Examples:
			|srv_name	|srv_path		|protocol	|apikey			|cbroker			|typ		|name	|type	|value	|attribute		|attr_value		|
			|srvm_put1	|/path_srvm_put	|IoTUL2		|srvm_putkey6	|http://myurl:80	|srv_attr	|t		|int	|temp	|srv_attr		|h#str:hum		|
			|srvm_put1	|/path_srvm_put	|IoTUL2		|srvm_putkey6	|http://myurl:80	|srv_attr	|h		|str	|hum	|srv_st_att		|t1#int:20		|
			|srvm_put1	|/path_srvm_put	|IoTUL2		|srvm_putkey6	|http://myurl:80	|srv_attr	|h		|str	|hum	|srv_st_att		|t2#int:200		|
			|srvm_put1	|/path_srvm_put	|IoTUL2		|srvm_putkey6	|http://myurl:80	|srv_st_att	|t2		|int	|200	|srv_st_att		|t3#int2:300	|
			|srvm_put1	|void		 	|IoTUL2		|srvm_putkey6	|http://myurl:80	|srv_attr	|t4		|int	|temp	|srv_st_att		|t1#int:30		|
			|srvm_put1	|void		 	|IoTTT		|				|http://myurl:80	|srv_attr	|t5		|int	|temp	|srv_st_att		|t1#int:40		|
			
			
			