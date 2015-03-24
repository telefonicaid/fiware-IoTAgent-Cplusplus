Feature: Create Service
    	 As an application
    	 I want to create services
    	 In order to include Services into IotAgent
    	
    	@iot_api @IDAS-18460 
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

		
		@iot_api @IDAS-18461 @IDAS-18462 
    	Scenario Outline: Create a service with invalid fields
		Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>" and apikey "<apikey>" not created
		When I try to create a Service with name "<srv_name>", path "<srv_path>", resource "<resource>", apikey "<apikey>" and cbroker "<cbroker>"
        Then user receives the "<HTTP_code>" and the "<exceptionText>"
		
		Examples:
			|srv_name	|srv_path			|resource	|apikey			|cbroker			| HTTP_code	|exceptionText 												|
			|srv_post6	|void				|/iot/d		|null			|http://myurl:80	| 400		|Missing required property: apikey							|
			|srv_post6	|void				|null		|srv_postkey6	|http://myurl:80	| 400		|Missing required property: resource						|
			|srv_post6	|void				|/iot/d		|srv_postkey6	|null				| 400		|Missing required property: cbroker							|
			|srv_post6	|void				|			|srv_postkey6	|http://myurl:80	| 400		|String does not match pattern "^/"							|
			|srv_post6	|void				|/iot/d		|srv_postkey6	|					| 400		|String is too short (0 chars), minimum 1					|
			|srv_post6	|void				|iot/d		|srv_postkey6	|http://myurl:80	| 400		|String does not match pattern "^/"		 					|
			|srv_post6	|void				|/iot/d		|srv_postkey6	|http:/wrongurl:80	| 400		|A parameter of the request is invalid/not allowed[{}]		|
			|void		|void				|/iot/d		|srv_postkey6	|http://myurl:80	| 400		|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|srv_post6	|path_srv_post6		|/iot/d		|srv_postkey6	|http://myurl:80	| 400		|Fiware-Service/Fiware-ServicePath not present or invalid	|
#			|srv_post7	|/path_srv_post7	|/iot/d		|srv_postkey1	|http://myurl:80	| 409		|Invalid parameter value: config.cbroker					|
			|srv_post5	|/path_srv_post5	|/iot/d		|				|http://myurl:80	| 409		|duplicate key error										|
			|srv_post1	|/path_srv_post1	|/iot/d		|srv_postkey1	|http://myurl:80	| 409		|duplicate key error										|
			|srv_post2	|/path_srv_post2	|/iot/d		|srv_postkey2_3	|http://myurl:80	| 409		|duplicate key error										|
			