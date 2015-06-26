Feature: API Service Create
    	 As an application
    	 I want to create services
    	 In order to include Services into IotAgent
    	
    	@iot_api @IDAS-20315 
    	Scenario Outline: Create a service
		Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>" and apikey "<apikey>" not created
		When I create a Service with name "<srv_name>", path "<srv_path>", resource "<resource>", apikey "<apikey>", cbroker "<cbroker>", entity_type "<entity_type>" and token "<token>"
        Then the Service with name "<srv_name>" and path "<srv_path>" is created
		
		Examples:
			|srv_name	|srv_path			|resource	|apikey			|cbroker			|entity_type	|token		|
			|srv_post	|void				|/iot/d		|srv_postkey	|http://myurl:80	|				|			|
			|srv_post1	|/path_srv_post1	|/iot/d		|srv_postkey1	|http://myurl:80	|				|			|
			|srv_post2	|/path_srv_post2	|/iot/d		|srv_postkey2	|http://myurl:80	|my_thing		|my_token	|
			|srv_post3	|void				|/iot/d		|				|http://myurl:80	|				|			|
			|srv_post1	|/path_srv_post1	|/iot/tt	|srv_postkey1	|http://myurl:80	|				|			|
			|srv_post1	|/path_srv_post1_2	|/iot/tt	|srv_postkey1	|http://myurl:80	|				|			|
			|srv_post2	|/path_srv_post2	|/iot/tt	|srv_postkey2_2	|http://myurl:80	|				|			|
			|srv_post1	|/path_srv_post1_3	|/iot/tt	|srv_postkey1_3	|http://myurl:80	|				|			|
			|srv_post4	|/path_srv_post1_3	|/iot/tt	|srv_postkey4	|http://myurl:80	|				|			|
			|srv_post5	|void				|/iot/d		|				|http://myurl:80	|				|			|
			|srv_post5	|/path_srv_post5	|/iot/d		|				|http://myurl:80	|				|			|
			|srv_post5	|/path_srv_post5	|/iot/tt	|				|http://myurl:80	|				|			|

		
		@iot_api @IDAS-20316 
    	Scenario Outline: Create a service with invalid fields
#		Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>" and apikey "<apikey>" not created
		When I try to create a Service with name "<srv_name>", path "<srv_path>", resource "<resource>", apikey "<apikey>" and cbroker "<cbroker>"
        Then user receives the "<HTTP_code>" and the "<exceptionText>"
		
		Examples:
			|srv_name	|srv_path			|resource	|apikey			|cbroker			| HTTP_code	|exceptionText 											|
			|srv_post6	|void				|/iot/d		|null			|http://myurl:80	| 400		|Missing required property: apikey						|
			|srv_post6	|void				|null		|srv_postkey6	|http://myurl:80	| 400		|Missing required property: resource					|
#			|srv_post6	|void				|/iot/d		|srv_postkey6	|null				| 400		|Missing required property: cbroker						|
			|srv_post6	|void				|void		|srv_postkey6	|http://myurl:80	| 400		|String does not match pattern "^/"						|
			|srv_post6	|void				|/iot/d		|srv_postkey6	|					| 400		|String is too short (0 chars), minimum 1				|
			|srv_post6	|void				|iot/d		|srv_postkey6	|http://myurl:80	| 400		|String does not match pattern "^/"		 				|
			|srv_post6	|void				|/iot/d		|srv_postkey6	|http:/wrongurl:80	| 400		|A parameter of the request is invalid/not allowed[{}]	|
			|void		|void				|/iot/d		|srv_postkey6	|http://myurl:80	| 400		|Fiware-Service not accepted							|
			|srv_post6	|path_srv_post6		|/iot/d		|srv_postkey6	|http://myurl:80	| 400		|Fiware-ServicePath not accepted						|
#			|srv_post7	|/path_srv_post7	|/iot/d		|srv_postkey1	|http://myurl:80	| 409		|Invalid parameter value: config.cbroker				|
			|srv_post5	|/path_srv_post5	|/iot/d		|				|http://myurl:80	| 409		|duplicate key: iot.SERVICE 							|
			|srv_post1	|/path_srv_post1	|/iot/d		|srv_postkey1	|http://myurl:80	| 409		|duplicate key: iot.SERVICE 							|
			|srv_post2	|/path_srv_post2	|/iot/d		|srv_postkey2_3	|http://myurl:80	| 409		|duplicate key: iot.SERVICE 							|


    	@iot_api @IDAS-20404 
    	Scenario Outline: Create a service with attributes
		Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>" and apikey "<apikey>" not created
		When I create a Service with name "<srv_name>", path "<srv_path>", resource "<resource>", apikey "<apikey>", cbroker "<cbroker>" and atributes "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value>" and "<value2>"
        Then the Service with name "<srv_name>" and path "<srv_path>" is created
		
		Examples:
			|srv_name	|srv_path			|resource	|apikey			|cbroker			|typ		|name		|type	|value	|typ2		|name2		|type2	|value2		|
			|srv_post8	|/path_srv_post8	|/iot/d		|srv_postkey8	|http://myurl:80	|srv_attr	|t			|int	|temp	|			|			|		|			|
			|srv_post8	|/path_srv_post8_0	|/iot/d		|srv_postkey8	|http://myurl:80	|srv_attr	|t1			|int	|temp1	|srv_attr	|h			|int	|hum		|
			|srv_post8	|/path_srv_post8_1	|/iot/d		|srv_postkey8	|http://myurl:80	|srv_st_att	|t2			|int	|22		|			|			|		|			|
			|srv_post8	|/path_srv_post8_2	|/iot/d		|srv_postkey8	|http://myurl:80	|srv_st_att	|t3			|int	|33		|srv_st_att	|h3			|int	|333		|
			|srv_post8	|/path_srv_post8_3	|/iot/d		|srv_postkey8	|http://myurl:80	|srv_attr	|t6			|int	|temp6	|srv_st_att	|h6			|int	|666		|
			