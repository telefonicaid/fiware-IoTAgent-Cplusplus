Feature: API Service Get Data
    	 As an application
    	 I want to retrieve services
    	 In order to include Services into IotAgent
    	
    	@iot_api @IDAS-20317 
    	Scenario Outline: Retrieve service data
    	Given a Service with name "<srv_name>", path "<srv_path>", resource "<resource>" and apikey "<apikey>" created
		When I retrieve the service data of "<find_srv>", path "<find_path>", resource "<find_res>", limit "<limit>" and offset "<offset>"
        Then I receive the service data of "<n_srvs>" services
		
		Examples:
			|srv_name	|srv_path		|resource	|apikey			|find_srv	|find_path		|find_res	|n_srvs	|limit	|offset	|
			|srv_get	|/path_srv_get	|/iot/d		|srv_getkey		|srv_get	|/path_srv_get	|			|1		|		|		|
			|srv_get	|/path_srv_get1	|/iot/d		|				|srv_get	|/path_srv_get1	|			|1		|		|		|
			|srv_get	|/path_srv_get	|/iot/tt	|srv_getkey		|srv_get	|/path_srv_get	|/iot/tt	|1		|		|		|
			|srv_get	|/path_srv_get1	|/iot/tt	|				|srv_get	|/path_srv_get1	|/iot/tt	|1		|		|		|
			|srv_get	|/path_srv_get	|/iot/mqtt	|srv_getkey		|srv_get	|/path_srv_get	|			|3		|		|		|
			|srv_get	|/path_srv_get1	|/iot/mqtt	|				|srv_get	|/path_srv_get1	|			|3		|		|		|
			|srv_get	|/path_srv_get	|/iot/d2	|srv_getkey		|srv_get	|/path_srv_get	|			|2		|2		|		|
			|srv_get	|/path_srv_get	|/iot/d3	|srv_getkey		|srv_get	|/path_srv_get	|			|3		|3		|0		|
			|srv_get	|/path_srv_get	|/iot/d4	|srv_getkey		|srv_get	|/path_srv_get	|			|0		|1		|6		|
			|srv_get	|/path_srv_get	|/iot/d5	|srv_getkey		|srv_get	|/path_srv_get	|			|2		|3		|5		|
			|srv_get	|/path_srv_get	|/iot/d6	|srv_getkey		|srv_get	|/*				|			|11		|		|		|
			|srv_get	|/path_srv_get2	|/iot/mqtt	|srv_getkey2	|srv_get	|/*				|/iot/mqtt	|3		|		|		|
			|srv_get	|/path_srv_get	|/iot/d7	|srv_getkey		|srv_get	|/*				|			|3		|5		|10		|
			|srv_get	|/path_srv_get3	|/iot/mqtt	|srv_getkey3	|srv_get	|/*				|/iot/mqtt	|2		|2		|1		|
			|srv_get	|/path_srv_get4	|/iot/mqtt	|srv_getkey4	|srv_get2	|/*				|			|0		|		|		|
			|srv_get	|/path_srv_get5	|/iot/mqtt	|srv_getkey5	|srv_get	|/path_srv_get6	|			|0		|		|		|
			|srv_get	|/path_srv_get6	|/iot/mqtt	|srv_getkey6	|srv_get	|/*				|/iot/d8	|0		|		|		|
			|srv_get	|/path_srv_get7	|/iot/mqtt	|srv_getkey7	|srv_get	|				|			|0		|		|		|
			|srv_get	|void			|/iot/mqtt	|srv_getkey8	|srv_get	|				|			|1		|		|		|


    	@iot_api @IDAS-20318 
    	Scenario Outline: Retrieve service data with invalid fields
		When I try to retrieve the service data of "<srv_name>", path "<srv_path>", resource "<resource>", limit "<limit>" and offset "<offset>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name	|srv_path		|resource	|limit			|offset			|HTTP_status	|exceptionText 									|
			|void		|void			|			|				|				|400			|Fiware-Service not accepted					|
			|void		|/path_srv_get	|			|				|				|400			|Fiware-Service not accepted					|
			|srv_get	|path_srv_get	|			|				|				|400			|Fiware-ServicePath not accepted				|
			|srv_get	|/path_srv_get	|			|wrong_limit	|				|400			|limit must be a number but it is wrong_limit	|
			|srv_get	|/path_srv_get	|			|				|wrong_offset	|400			|offset must be a number but it is wrong_offset	|
			