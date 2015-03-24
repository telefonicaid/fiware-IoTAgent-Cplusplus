Feature: Update Service
    	 As an application
    	 I want to update services data
    	 In order to manage Services into IotAgent
    	
    	@iot_api @IDAS-18464 @IDAS-18465 @IDAS-18466 
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

    	@iot_api @IDAS-18467 
    	Scenario Outline: Update data into service with invalid fields
		When I try to update the attribute "<attribute>" with value "<value>" of service "<srv_name>" with path "<srv_path>", resource "<resource>", apikey "<apikey>" and cbroker "<cbroker>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
        And the service data NOT contains attribute "<attribute>" with value "<value>"
		
		Examples:
			|srv_name	|srv_path		|resource	|apikey			|cbroker			|attribute		|value				|HTTP_status	|exceptionText 												|
			|void		|void			|/iot/d		|srv_putkey		|http://myurl2:200	|cbroker		|http://myurl2:2000	|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|void		|/path_srv_put	|/iot/d		|srv_putkey		|http://myurl2:200	|cbroker		|http://myurl2:2000	|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|void		|path_srv_put	|/iot/d		|srv_putkey		|http://myurl2:200	|cbroker		|http://myurl2:2000	|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|srv_put	|/path_srv_put	|/iot/tt	|srv_putkey		|http://myurl2:200	|cbroker		|http://myurl2:2000	|204			|															|
			|srv_put	|/path_srv_put	|/iot/d		|srv_putkey2	|http://myurl2:200	|cbroker		|http://myurl2:2000	|204			|															|
			|srv_put	|/path_srv_put	|			|srv_putkey		|http://myurl2:200	|cbroker		|http://myurl2:2000	|400			|resource parameter is mandatory in PUT operation			|
			|srv_put	|/path_srv_put	|/iot/tt	|				|http://myurl2:200	|cbroker		|http://myurl2:2000	|204			|															|
			|srv_put	|/path_srv_put2	|/iot/d		|srv_putkey2	|http://myurl:80	|resource		|/iot/tt			|409			|duplicate key error										|
			|srv_put	|void			|/iot/d		|				|http://myurl:80	|resource		|/iot/tt			|409			|duplicate key error										|
			