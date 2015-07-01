Feature: API Service Update
    	 As an application
    	 I want to update services data
    	 In order to manage Services into IotAgent
    	
    	@iot_api @IDAS-20321 
    	Scenario Outline: Update data into service
    	Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>", apikey "<apikey>", cbroker "<cbroker>", entity_type "<entity_type>" and token "<token>" created
		When I update the attribute "<attribute>" of service "<srv_name>" with value "<value>"
        Then the service data contains attribute "<attribute>" with value "<value>"
		
		Examples:
			|srv_name	|srv_path			|resource	|apikey			|cbroker			|entity_type	|token		|attribute		|value				|
			|srv_put	|/path_srv_put		|/iot/d		|srv_putkey		|http://myurl:80	|my_thing		|my_token	|cbroker		|http://myurl2:200	|
			|srv_put	|/path_srv_put2		|/iot/d		|srv_putkey2	|http://myurl:80	|my_thing		|my_token	|entity_type	|my_thing22			|
			|srv_put	|/path_srv_put2		|/iot/tt	|srv_putkey2	|http://myurl:90	|my_thing		|my_token	|token			|my_token33			|
			|srv_put	|/path_srv_put3		|/iot/d		|srv_putkey3	|http://myurl:80	|my_thing		|my_token	|apikey			|srv_putkey3_1		|
			|srv_put	|/path_srv_put4		|/iot/d		|srv_putkey4	|http://myurl:80	|my_thing		|my_token	|apikey			|					|
			|srv_put	|/path_srv_put5		|/iot/d		|srv_putkey5	|http://myurl:80	|my_thing		|my_token	|resource		|/iot/tt			|
			|srv_put	|void				|/iot/d		|srv_putkey		|http://myurl:80	|my_thing		|my_token	|apikey			|					|
			|srv_put	|void				|/iot/tt	|				|http://myurl:80	|my_thing		|my_token	|cbroker		|http://myurl4:400	|


    	@iot_api @IDAS-20322 
    	Scenario Outline: Update data into service with invalid fields
		When I try to update the attribute "<attribute>" with value "<value>" of service "<srv_name>" with path "<srv_path>", resource "<resource>", apikey "<apikey>" and cbroker "<cbroker>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
        And the service data NOT contains attribute "<attribute>" with value "<value>"
		
		Examples:
			|srv_name	|srv_path		|resource	|apikey			|cbroker			|attribute	|value				|HTTP_status	|exceptionText 										|
			|void		|void			|/iot/d		|srv_putkey		|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Fiware-Service not accepted						|
			|void		|/path_srv_put	|/iot/d		|srv_putkey		|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Fiware-Service not accepted						|
			|srv_put	|path_srv_put	|/iot/d		|srv_putkey		|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|Fiware-ServicePath not accepted					|
			|srv_put	|/path_srv_put	|/iot/tt	|srv_putkey		|http://myurl2:200	|cbroker	|http://myurl2:2000	|404			|The service does not exist							|
			|srv_put	|/path_srv_put	|/iot/d		|srv_putkey2	|http://myurl2:200	|cbroker	|http://myurl2:2000	|404			|The service does not exist							|
			|srv_put	|/path_srv_put	|			|srv_putkey		|http://myurl2:200	|cbroker	|http://myurl2:2000	|400			|resource parameter is mandatory in PUT operation	|
			|srv_put	|/path_srv_put	|/iot/tt	|				|http://myurl2:200	|cbroker	|http://myurl2:2000	|404			|The service does not exist							|
			|srv_put	|/path_srv_put2	|/iot/d		|srv_putkey2	|http://myurl:80	|resource	|/iot/tt			|409			|duplicate key: iot.SERVICE							|
			|srv_put	|void			|/iot/d		|				|http://myurl:80	|resource	|/iot/tt			|409			|duplicate key: iot.SERVICE							|
			|srv_put	|/path_srv_put2	|/iot/d		|srv_putkey2	|http://myurl:80	|			|					|400			|Additional properties not allowed. [/]				|
			|srv_put	|/path_srv_put2	|/iot/d		|srv_putkey2	|http://myurl:80	|empty_json	|					|400			|empty body											|


    	@iot_api @IDAS-20406 
    	Scenario Outline: Update data into service with attributes
    	Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>", apikey "<apikey>", cbroker "<cbroker>" and atribute "<typ>", with name "<name>", type "<type>" and value "<value>" created
		When I update the attribute "<attribute>" of service "<srv_name>" with value "<attr_value>"
        Then the service data contains attribute "<attribute>" with value "<attr_value>"
		
		Examples:
			|srv_name	|srv_path		|resource	|apikey			|cbroker			|typ		|name	|type	|value	|attribute		|attr_value		|
			|srv_put1	|/path_srv_put	|/iot/d		|srv_putkey6	|http://myurl:80	|srv_attr	|t		|int	|temp	|srv_attr		|h#str:hum		|
			|srv_put1	|/path_srv_put	|/iot/d		|srv_putkey6	|http://myurl:80	|srv_attr	|h		|str	|hum	|srv_st_att		|t1#int:20		|
			|srv_put1	|/path_srv_put	|/iot/d		|srv_putkey6	|http://myurl:80	|srv_attr	|h		|str	|hum	|srv_st_att		|t2#int:200		|
			|srv_put1	|/path_srv_put	|/iot/d		|srv_putkey6	|http://myurl:80	|srv_st_att	|t2		|int	|200	|srv_st_att		|t3#int2:300	|
			|srv_put1	|void		 	|/iot/d		|srv_putkey6	|http://myurl:80	|srv_attr	|t4		|int	|temp	|srv_st_att		|t1#int:30		|
			|srv_put1	|void		 	|/iot/tt	|				|http://myurl:80	|srv_attr	|t5		|int	|temp	|srv_st_att		|t1#int:40		|
			
			
			