Feature: Create Device
    	 As an application
    	 I want to create devices
    	 In order to include Devices into IotAgent
    	
    	@iot_api @IDAS-18460 
    	Scenario Outline: Create a device
		Given a Service with name "<srv_name>" and path "<srv_path>" created
		And a Device with name "<dev_id>" and path "<srv_path>" not created
		When I create a Device with name "<dev_id>", entity_name "<ent_name>", entity_type "<ent_type>" and endpoint "<endpoint>"
        Then the Device with name "<dev_id>" is created
		
		Examples:
			|srv_name		|srv_path				|dev_id			|ent_name	|ent_type		|endpoint			|
			|srv_dev_post	|void					|dev_post		|			|				|					|
			|srv_dev_post	|/path_srv_dev_post		|dev_post		|			|				|					|
			|srv_dev_post	|/path_srv_dev_post		|dev_post1		|room_post1	|				|					|
			|srv_dev_post1	|/path_srv_dev_post		|dev_post		|			|thing_post		|					|
			|srv_dev_post1	|/path_srv_dev_post		|dev_post1		|			|				|http://myurl:99	|
			|srv_dev_post1	|/path_srv_dev_post1	|dev_post1		|room_post1 |thing_post1	|http://myurl2:99	|

		
    	@iot_api @IDAS-18460 
    	Scenario Outline: Create a device with attributes and/or commands
		Given a service with name "<srv_name>" and path "<srv_path>" created
		And a Device with name "<dev_id>" and path "<srv_path>" not created
		When I create a Device with name "<dev_id>", atributes and/or commands "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value>" and "<value2>"
        Then the Device with name "<dev_id>" is created
		
		Examples:
			|srv_name		|srv_path				|dev_id			|typ	|name		|type	|value	|typ2	|name2		|type2	|value2		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post		|attr	|t			|int	|temp	|		|			|		|			|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post1		|attr	|t1			|int	|temp1	|attr	|h			|int	|hum		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post2		|st_att	|t2			|int	|22		|		|			|		|			|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post3		|st_att	|t3			|int	|33		|st_att	|h3			|int	|333		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post4		|cmd	|cmd_dev4	|		|ping4	|		|			|		|			|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post5		|cmd	|cmd_dev5	|		|ping5	|cmd	|cmd2_dev5	|		|ping5_2	|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post6		|attr	|t6			|int	|temp6	|st_att	|h6			|int	|666		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post7		|attr	|t7			|int	|temp7	|cmd	|cmd_dev7	|		|ping7		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post8		|st_att	|t8			|int	|88		|cmd	|cmd_dev8	|		|ping8		|


		@iot_api @IDAS-18461 @IDAS-18462 
    	Scenario Outline: Create a device with invalid fields
		Given a service with name "<srv_name>" and path "<srv_path>" created
		And a Device with name "<dev_id>" and path "<srv_path_dev>" not created
		When I try to create a Device with name "<dev_id>" and atribute or command "<typ>", with name "<name>", type "<type>" and value "<value>"
        Then user receives the "<HTTP_code>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path				|srv_path_dev			|dev_id			|typ	|name		|type		|value	| HTTP_code	|exceptionText 																		|
			|void			|void					|void					|dev_post		|attr	|t			|int		|temp	| 400		|Fiware-Service/Fiware-ServicePath not present or invalid							|
			|srv_dev_post3	|/path_srv_dev_post3	|path_srv_dev_post3		|dev_post		|attr	|t			|int		|temp	| 400		|Fiware-Service/Fiware-ServicePath not present or invalid							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|void			|attr	|t			|int		|temp	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/device_id]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|				|attr	|t			|int		|temp	| 400		|Missing required property: device_id												|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|attr	|			|int		|temp	| 400		|Missing required property: name [/devices[0]/attributes[0]]						|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|attr	|void		|int		|temp	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/attributes[0]/name]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|attr	|t			|			|temp	| 400		|Missing required property: type [/devices[0]/attributes[0]]						|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|attr	|t			|void		|temp	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/attributes[0]/type]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|attr	|t			|int		|		| 400		|Missing required property: object_id [/devices[0]/attributes[0]]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|attr	|t			|int		|void	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/attributes[0]/object_id]		|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|st_att	|			|int		|11		| 400		|Missing required property: name [/devices[0]/static_attributes[0]]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|st_att	|void		|int		|11		| 400		|String is too short (0 chars), minimum 1 [/devices[0]/static_attributes[0]/name]	|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|st_att	|t			|			|11		| 400		|Missing required property: type [/devices[0]/static_attributes[0]]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|st_att	|t			|void		|11		| 400		|String is too short (0 chars), minimum 1 [/devices[0]/static_attributes[0]/type]	|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|st_att	|t			|int		|		| 400		|Missing required property: value [/devices[0]/static_attributes[0]]				|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|st_att	|t			|int		|void	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/static_attributes[0]/value]	|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|			|command	|ping	| 400		|Missing required property: name [/devices[0]/commands[0]]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|void		|command	|ping	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/commands[0]/name]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|cmd_dev	|			|ping	| 400		|Missing required property: type [/devices[0]/commands[0]]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|cmd_dev	|void		|ping	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/commands[0]/type]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|cmd_dev	|command	|		| 400		|Missing required property: value [/devices[0]/commands[0]]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|cmd_dev	|command	|void	| 400		|String is too short (0 chars), minimum 1 [/devices[0]/commands[0]/value]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|cmd	|cmd_dev	|comand		|ping	| 409		|Missing required property: apikey													|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post4	|dev_post		|		|			|			|		| 404		|The service does not exist															|
			|srv_dev_post2	|/path_srv_dev_post2	|/path_srv_dev_post2	|dev_post		|		|			|			|		| 409		|duplicate key error																|
			|srv_dev_post	|void					|void					|dev_post		|		|			|			|		| 409		|duplicate key error																|
			