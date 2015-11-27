Feature: MQTT Command Send 
	In order to include MQTT protocol into SBC
	As a MQTT subscriber
	I want to receive messages from SBC
	
	@iot_mqtt @iot_cmd_mqtt @IDAS-18382
	Scenario: Send a command with parameters
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", name "<device_name>", endpoint "<cmdURL>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", cmd_type "<cmd_type>" and value "<cmd_name>" 
		And I wait the pooling period
		Then the command of device "<device_name>" with response "<response>" and status "<status>" is received by context broker
		
		Examples:
			|device_id	|device_name |service		|protocol	|command	|cmd_name				|cmd_value	|cmd_type |cmdURL			|status |response								|
#			|dev1		|room1		 |servicemqtt	|IoTMqtt	|ping1		|param1=value1			|			|push	  |void				|OK		|dev1@ping1#param1=value1				|
			|dev1_1		|room1_1	 |servicemqtt	|IoTMqtt	|ping1_1	|p1=v1#p2=v2			|			|push	  |void				|OK		|dev1_1@ping1_1#p1=v1#p2=v2				|
			|dev1_2		|room1_2	 |servicemqtt	|IoTMqtt	|ping1_2	|p1=v1#p2=v2#p3=v3#p4=v4|			|push	  |void				|OK		|dev1_2@ping1_2#p1=v1#p2=v2#p3=v3#p4=v4	|
			|dev1_3		|room1_3	 |servicemqtt	|IoTMqtt	|ping1_3	|p1=v1#p2=v2			|			|push	  |http://myurl.com	|OK		|dev1_3@ping1_3#p1=v1#p2=v2				|
#			|dev1		|room1		 |servicemqtt	|IoTMqtt	|attr_1_dev_1	|ping1#param1=value1			|dev1@%s#%s				|push	  |void				|OK		|dev1@ping1#param1=value1				|
#			|dev1_1		|room1_1	 |servicemqtt	|IoTMqtt	|attr_1_dev_1_1	|ping1_1#p1=v1#p2=v2			|dev1_1@%s#%s#%s		|push	  |void				|OK		|dev1_1@ping1_1#p1=v1#p2=v2				|
#			|dev1_2		|room1_2	 |servicemqtt	|IoTMqtt	|attr_1_dev_1_2	|ping1_2#p1=v1#p2=v2#p3=v3#p4=v4|dev1_2@%s#%s#%s#%s#%s	|push	  |void				|OK		|dev1_2@ping1_2#p1=v1#p2=v2#p3=v3#p4=v4	|
#			|dev1_3		|room1_3	 |servicemqtt	|IoTMqtt	|attr_1_dev_1_3	|ping1_3#p1=v1#p2=v2			|dev1_3@%s#%s#%s		|push	  |http://myurl.com	|OK		|dev1_3@ping1_3#p1=v1#p2=v2				|
		
	@iot_mqtt @iot_cmd_mqtt @IDAS-18381
	Scenario: Send a command without parameters
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", name "<device_name>", endpoint "<cmdURL>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", cmd_type "<cmd_type>" and value "<cmd_name>" 
		And I wait the pooling period
		Then the command of device "<device_name>" with response "<response>" and status "<status>" is received by context broker
		
		Examples:
			|device_id	|device_name |service		|protocol	|command	|cmd_name	|cmd_value	|cmd_type |cmdURL			|response							|status		|
			|dev2		|room2		 |servicemqtt	|IoTMqtt	|ping2		|			|			|push	  |void				|dev2@ping2Result#Command_OK		|OK			|	
			|dev2_1		|room2_1	 |servicemqtt	|IoTMqtt	|ping2_1	|			|			|push	  |http://myurl.com	|dev2_1@ping2_1Result#Command_OK	|OK			|	
#			|dev2		|room2		 |servicemqtt	|IoTMqtt	|attr_1_dev_2	|ping2		|dev2@%s	|push	  |void				|dev2@ping2Result#Command_OK		|OK			|	
#			|dev2_1		|room2_1	 |servicemqtt	|IoTMqtt	|attr_1_dev_2_1	|ping2_1	|dev2_1@%s	|push	  |http://myurl.com	|dev2_1@ping2_1Result#Command_OK	|OK			|	
		
		
	@iot_mqtt @iot_cmd_mqtt @IDAS-18380
	Scenario: Send a command without response
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", name "<device_name>", endpoint "<cmdURL>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", cmd_type "<cmd_type>" and value "<cmd_name>" 
		And I wait the command timeout "<timeout>" for "<cmd_type>"
		Then the command of device "<device_name>" with response "" and status "<status>" is received by context broker
		
		Examples:
			|device_id	|device_name |service		|protocol	|command	|cmd_name	|cmd_value	|cmd_type 		|cmdURL				|status							|timeout |
			|dev3		|room3		 |servicemqtt	|IoTMqtt	|ping3		|			|dev3@%s	|push	  		|void				|delivered but no respond 					|10		 |
			|dev3_1		|room3_1	 |servicemqtt	|IoTMqtt	|ping3_1	|			|dev3_1@%s	|push	 	 	|http://myurl.com	|delivered but no respond		|10		 |
			|dev3_2		|room3_2	 |servicemqtt	|IoTMqtt	|ping3_2	|			|dev3_2@%s	|push_not_resp	|void				|delivered but no respond		|10		 |
			|dev3_3		|room3_3	 |servicemqtt	|IoTMqtt	|ping3_3	|			|dev3_3@%s	|push_not_resp 	|http://myurl.com	|delivered but no respond		|10		 |
#			|dev3		|room3		 |servicemqtt	|IoTMqtt	|attr_1_dev_3	|ping3		|dev3@%s	|push	  		|void				|delivered but no respond read					|10		 |
#			|dev3_1		|room3_1	 |servicemqtt	|IoTMqtt	|attr_1_dev_3_1	|ping3_1	|dev3_1@%s	|push	 	 	|http://myurl.com	|delivered but no respond		|10		 |
#			|dev3_2		|room3_2	 |servicemqtt	|IoTMqtt	|attr_1_dev_3_2	|ping3_2	|dev3_2@%s	|push_not_resp	|void				|delivered but no respond		|10		 |
#			|dev3_3		|room3_3	 |servicemqtt	|IoTMqtt	|attr_1_dev_3_3	|ping3_3	|dev3_3@%s	|push_not_resp 	|http://myurl.com	|delivered but no respond		|10		 |
		    
