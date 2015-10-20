Feature: API Device Create
    	 As an application
    	 I want to create devices
    	 In order to include Devices into IotAgent
    	
    	@iot_api @IDAS-20326 
    	Scenario Outline: Create a device
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>" and path "<srv_path>" not created
		When I create a Device with name "<dev_id>", entity_name "<ent_name>", entity_type "<ent_type>", endpoint "<endpoint>" and protocol "<protocol>"
        Then the Device with name "<dev_id>" is created
		
		Examples:
			|srv_name		|srv_path				|dev_id			|ent_name	|ent_type		|endpoint			|protocol	|
			|srv_dev_post	|void					|dev_post		|			|				|					|IoTUL2		|
			|srv_dev_post	|/path_srv_dev_post		|dev_post		|			|				|					|IoTTT		|
			|srv_dev_post	|/path_srv_dev_post_1	|dev_post1		|room_post1	|				|					|IoTMqtt	|
			|srv_dev_post1	|/path_srv_dev_post_2	|dev_post		|			|thing_post		|					|IoTUL2		|
			|srv_dev_post1	|/path_srv_dev_post		|dev_post2		|			|thing_post		|					|IoTTT		|
			|srv_dev_post1	|/path_srv_dev_post_1	|dev_post1		|			|				|http://myurl:99	|IoTMqtt	|
			|srv_dev_post1	|/path_srv_dev_post1	|dev_post1		|room_post1 |thing_post1	|http://myurl2:99	|IoTUL2		|

		
    	@iot_api @IDAS-20327 
    	Scenario Outline: Create a device with attributes and/or commands
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>" and path "<srv_path>" not created
		When I create a Device with name "<dev_id>", protocol "<protocol>", atributes and/or commands "<typ>" and "<typ2>", with names "<name>" and "<name2>", types "<type>" and "<type2>" and values "<value>" and "<value2>"
        Then the Device with name "<dev_id>" is created
		
		Examples:
			|srv_name		|srv_path				|dev_id			|protocol	|typ		|name		|type	|value	|typ2		|name2		|type2	|value2		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post		|IoTUL2		|dev_attr	|t			|int	|temp	|			|			|		|			|
			|srv_dev_post2	|/path_srv_dev_post2_1	|dev_post1		|IoTTT		|dev_attr	|t1			|int	|temp1	|dev_attr	|h			|int	|hum		|
			|srv_dev_post2	|/path_srv_dev_post2_2	|dev_post2		|IoTMqtt	|dev_st_att	|t2			|int	|22		|			|			|		|			|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post3		|IoTUL2		|dev_st_att	|t3			|int	|33		|dev_st_att	|h3			|int	|333		|
			|srv_dev_post2	|/path_srv_dev_post2_1	|dev_post4		|IoTTT		|cmd		|cmd_dev4	|		|ping4	|			|			|		|			|
			|srv_dev_post2	|/path_srv_dev_post2_2	|dev_post5		|IoTMqtt	|cmd		|cmd_dev5	|		|ping5	|cmd		|cmd2_dev5	|		|			|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post6		|IoTUL2		|dev_attr	|t6			|int	|temp6	|dev_st_att	|h6			|int	|666		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post7		|IoTUL2		|dev_attr	|t7			|int	|temp7	|cmd		|cmd_dev7	|		|ping7		|
			|srv_dev_post2	|/path_srv_dev_post2	|dev_post8		|IoTUL2		|dev_st_att	|t8			|int	|88		|cmd		|cmd_dev8	|		|ping8		|


		@iot_api @IDAS-20328 
    	Scenario Outline: Create a device with invalid fields
    	Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		When I try to create a Device with name "<dev_id>", path "<srv_path_dev>", protocol "<protocol>" and atribute or command "<typ>", with name "<name>", type "<type>" and value "<value>"
        Then user receives the "<HTTP_code>" and the "<exceptionText>"        
		
		Examples:
			|srv_name		|srv_path				|srv_path_dev			|dev_id			|protocol	|typ		|name		|type		|value	|HTTP_code	|exceptionText 																		|
			|void			|void					|void					|dev_post		|IoTUL2		|dev_attr	|t			|int		|temp	|400		|Fiware-Service not accepted														|
			|srv_dev_post3	|/path_srv_dev_post3	|path_srv_dev_post3		|dev_post		|IoTUL2		|dev_attr	|t			|int		|temp	|400		|Fiware-ServicePath not accepted													|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|void			|IoTUL2		|dev_attr	|t			|int		|temp	|400		|String is too short (0 chars), minimum 1 [/devices[0]/device_id]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|				|IoTUL2		|dev_attr	|t			|int		|temp	|400		|Missing required property: device_id												|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_attr	|			|int		|temp	|400		|Missing required property: name [/devices[0]/attributes[0]]						|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_attr	|void		|int		|temp	|400		|String is too short (0 chars), minimum 1 [/devices[0]/attributes[0]/name]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_attr	|t			|			|temp	|400		|Missing required property: type [/devices[0]/attributes[0]]						|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_attr	|t			|void		|temp	|400		|String is too short (0 chars), minimum 1 [/devices[0]/attributes[0]/type]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_attr	|t			|int		|		|400		|Missing required property: object_id [/devices[0]/attributes[0]]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_attr	|t			|int		|void	|400		|String is too short (0 chars), minimum 1 [/devices[0]/attributes[0]/object_id]		|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post1		|IoTUL2		|dev_st_att	|			|int		|11		|400		|Missing required property: name [/devices[0]/static_attributes[0]]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_st_att	|void		|int		|11		|400		|String is too short (0 chars), minimum 1 [/devices[0]/static_attributes[0]/name]	|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post2		|IoTUL2		|dev_st_att	|t			|			|11		|400		|Missing required property: type [/devices[0]/static_attributes[0]]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_st_att	|t			|void		|11		|400		|String is too short (0 chars), minimum 1 [/devices[0]/static_attributes[0]/type]	|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post3		|IoTUL2		|dev_st_att	|t			|int		|		|400		|Missing required property: value [/devices[0]/static_attributes[0]]				|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|dev_st_att	|t			|int		|void	|400		|String is too short (0 chars), minimum 1 [/devices[0]/static_attributes[0]/value]	|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|cmd		|			|command	|ping	|400		|Missing required property: name [/devices[0]/commands[0]]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|cmd		|void		|command	|ping	|400		|String is too short (0 chars), minimum 1 [/devices[0]/commands[0]/name]			|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|cmd		|cmd_dev	|null		|ping	|400		|Missing required property: type [/devices[0]/commands[0]]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|cmd		|cmd_dev	|void		|ping	|400		|No enum match for: "" [/devices[0]/commands[0]/type]								|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|cmd		|cmd_dev	|command	|		|400		|Missing required property: value [/devices[0]/commands[0]]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|cmd		|cmd_dev	|comand		|ping	|400		|No enum match for: "comand" [/devices[0]/commands[0]/type]							|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post4	|dev_post		|IoTUL2		|			|			|			|		|404		|The service does not exist															|
			|srv_dev_post2	|/path_srv_dev_post2	|/path_srv_dev_post2	|dev_post		|IoTUL2		|			|			|			|		|409		|duplicate key: iot.DEVICE															|
			|srv_dev_post	|void					|void					|dev_post		|IoTUL2		|			|			|			|		|409		|duplicate key: iot.DEVICE															|
#			|srv_dev_post1	|/path_srv_dev_post1	|/path_srv_dev_post1	|dev_post2		|IoTUL2		|ent_name	|room_post1	|			|		|409		|entity already exists																|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|protocol	|null		|			|		|400		|Missing required property: protocol												|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|protocol	|void		|			|		|400		|String is too short (0 chars), minimum 1 [/devices[0]/protocol]					|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|protocol	|IoTTT		|			|		|400		|There are conflicts, protocol is not correct										|
			|srv_dev_post3	|/path_srv_dev_post3	|/path_srv_dev_post3	|dev_post		|IoTUL2		|protocol	|IoTUnknown	|			|		|400		|There are conflicts, protocol is not correct										|
			
