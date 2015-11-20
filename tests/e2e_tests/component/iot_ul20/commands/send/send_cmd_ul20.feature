Feature: UL20 Command Send
    	 As a device
    	 I want to send commands to IoTAgent throught context broker for UL20
    	 In order to assure IotAgent_UL20-Cbroker integration
  
    	
    	@iot_ul20 @iot_cmd_ul20 @IDAS-20170
    	Scenario Outline: Send a single command
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", name "<device_name>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", entity_type "<ent_type>" and value "<value>" 
		Then the command of device "<device_name>" with response "<response>" and status "<status>" is received by context broker
		
		Examples:
            |device_id	|device_name |service		|protocol	|command	|value			|cmd_value	|response						|status		|ent_type	|
            |dev1		|room1		 |serviceul20	|IoTUL2		|ping1		|				|			|dev1@ping1						|OK			|			|
            |dev1_1		|room1_1	 |serviceul20	|IoTUL2		|ping1_1	|param1=value1	|			|dev1_1@ping1_1#param1=value1	|OK			|			|
            |dev1_2		|room1_2	 |serviceul20	|IoTUL2		|ping1_2	|p1=v1#p2=v2	|			|dev1_2@ping1_2#p1=v1#p2=v2		|OK			|			|
#            |dev1		|room1		 |serviceul20	|IoTUL2		|attr_1_dev_1	|ping1					|dev1@%s			|dev1@ping1						|OK			|			|
#            |dev1_1		|room1_1	 |serviceul20	|IoTUL2		|attr_2_dev_1_1	|ping1_1#param1=value1	|dev1_1@%s#%s		|dev1_1@ping1_1#param1=value1	|OK			|			|
#            |dev1_2		|room1_2	 |serviceul20	|IoTUL2		|attr_3_dev_1_2	|ping1_2#p1=v1#p2=v2	|dev1_2@%s#%s#%s	|dev1_2@ping1_2#p1=v1#p2=v2		|OK			|			|


    	@iot_ul20 @iot_cmd_ul20 @IDAS-20236
    	Scenario Outline: Send a pooling command
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", name "<device_name>", endpoint "<endpoint>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", entity_type "<ent_type>" and value "<value>"
		And I get the command and send the response "<response>" for device "<device_id>"
		Then the command of device "<device_name>" with response "<response>" and status "<status>" is received by context broker
		
		Examples:
            |device_id	|device_name |service		|protocol	|command	|value	|cmd_value	|endpoint	|response			|status						|ent_type	|
            |dev6		|room6		 |serviceul20	|IoTUL2		|ping6		|		|			|void		|ping6_OK			|OK							|			|
            |dev6_0		|room6_0	 |serviceul20	|IoTUL2		|ping6_0	|		|			|void		|ping6_0_measure	|OK							|			|
            |dev6_1		|room6_1	 |serviceul20	|IoTUL2		|ping6_1	|p1=v1	|			|void		|ping6_1_OK			|OK							|			|
            |dev6_2		|room6_2	 |serviceul20	|IoTUL2		|ping6_2	|		|			|void		|					|delivered but no respond	|			|
            |dev6_3		|room6_3	 |serviceul20	|IoTUL2		|ping6_3	|		|			|void		|ping6_3_expired	|delivered but no respond	|			|
            |dev6_4		|room6_4	 |serviceul20	|IoTUL2		|ping6_4	|		|			|void		|not_read			|expired read				|			|
#            |dev6		|room6		 |serviceul20	|IoTUL2		|attr_1_dev_6	|ping6			|dev6@%s		|void		|ping6_OK			|OK							|			|
#            |dev6_0	|room6_0	 |serviceul20	|IoTUL2		|attr_1_dev_6_0	|ping6_0		|dev6_0@%s		|void		|ping6_0_measure	|OK							|			|
#            |dev6_1	|room6_1	 |serviceul20	|IoTUL2		|attr_1_dev_6_1	|ping6_1#p1=v1	|dev6_1@%s#%s	|void		|ping6_1_OK			|OK							|			|
#            |dev6_2	|room6_2	 |serviceul20	|IoTUL2		|attr_1_dev_6_2	|ping6_2		|dev6_2@%s		|void		|					|delivered but no respond	|			|
#            |dev6_3	|room6_3	 |serviceul20	|IoTUL2		|attr_1_dev_6_3	|ping6_3		|dev6_3@%s		|void		|ping6_3_expired	|delivered but no respond	|			|
#            |dev6_4	|room6_4	 |serviceul20	|IoTUL2		|attr_1_dev_6_4	|ping6_4		|dev6_4@%s		|void		|not_read			|expired read				|			|

    	@iot_ul20 @iot_cmd_ul20 @IDAS-20169
    	Scenario Outline: Send a wrong command
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", name "<device_name>", endpoint "<endpoint>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a wrong command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", value "<value>" and wrong "<wrong_field>" 
		Then the command of device "<device_name>" with response "<response>" and status "<status>" is received or NOT by context broker
		
		Examples:
            |device_id	|device_name |service		|protocol	|command	|value		|cmd_value	|endpoint														|status		|wrong_field			|response																			|
            |dev2		|room2		 |serviceul20	|IoTUL2		|ping2		|ping2		|			|http://10.95.213.159:5371/simulaClient/lightCommand			|OK			|simulator_type			|no ul20 response command:															|
            |dev3		|room3		 |serviceul20	|IoTUL2		|ping3		|ping3		|			|http://192.0.3.1:9001											|error		|nonexist_destination	|http://192.0.3.1:9001/  [No route to host]											|
            |dev2		|room2	 	 |serviceul20	|IoTUL2		|ping2_1	|ping2_1	|			|																|fail		|nonexistent_command	|the device does not have implemented this command									|         
            |dev11		|room11	 	 |serviceul20	|IoTUL2		|ping11		|ping11		|fail		|																|fail		|nonexistent_device		|The device does not exist															| 
            |dev5		|room5		 |serviceMqtt	|			|ping5		|ping5		|fail		|																|fail		|nonexistent_service	|Fiware-Service not accepted														|        
#            |dev2		|room2		 |serviceul20	|IoTUL2		|attr_1_dev_2	|ping2		|dev2@%s	|http://10.95.213.159:5371/simulaClient/lightCommand			|OK			|simulator_type			|no ul20 response command:															|
#            |dev3		|room3		 |serviceul20	|IoTUL2		|attr_1_dev_3	|ping3		|dev3@%s	|http://192.168.1.1:9001										|error		|nonexist_destination	|http://192.168.1.1:9001/  [No route to host]										|
#            |dev4		|room4		 |serviceul20	|IoTUL2		|attr_1_dev_4	|ping4		|dev4@%s	|http://10.95.213.159:5371/simulaClient/lightCommand?delay=10	|error		|unreachable_dest		|http://10.95.213.159:5371/simulaClient/lightCommand delay=10 [Connection timed out]|         
#            |dev2_1	|room2_1	 |serviceul20	|IoTUL2		|attr_2_dev_2_1	|ping2_1	|%s			|																|fail		|wrong_command_format	|this is not a valid command														|
#            |dev2		|room2	 	 |serviceul20	|IoTUL2		|attr_2_dev_2	|ping2_1	|			|																|fail		|nonexistent_command	|the device does not have implemented this command									|         
#            |dev11		|room11	 	 |serviceul20	|IoTUL2		|attr_1_dev_11	|ping11		|			|																|fail		|nonexistent_device		|The device does not exist															| 
#            |dev5		|room5		 |serviceMqtt	|			|attr_1_dev_5	|ping5		|			|																|fail		|nonexistent_service	|Fiware-Service not accepted														|        


    	@iot_ul20 @iot_cmd_ul20 @IDAS-20171
    	Scenario Outline: Send a command for a device with entity name 
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with id "<device_id>", entity type "<ent_type>", entity name "<ent_name>", protocol "<protocol>", command name "<command>" and command value "<cmd_value>" created
		When I send a command to the IoTAgent with service "<service>", device "<device_name>", command "<command>", entity_type "<ent_type>" and value "<value>" 
		Then the command of device "<device_name>" with response "<response>", entity_type "<ent_type>" and status "<status>" is received by context broker
		
		Examples:
            |device_id	|device_name |service		|protocol	|command	|value	|cmd_value	|response		|status	|ent_type	|ent_name	|
            |dev7		|room7		 |serviceul20	|IoTUL2		|ping7		|ping7	|			|dev7@ping7		|OK		|thing7		|room7		|
            |dev8		|room8		 |serviceul20	|IoTUL2		|ping8		|ping8	|			|dev8@ping8		|OK		|			|room8		|
            |dev9		|			 |serviceul20	|IoTUL2		|ping9		|ping9	|			|dev9@ping9		|OK		|thing9		|			|
            |dev10		|			 |serviceul20	|IoTUL2		|ping10		|ping10	|			|dev10@ping10	|OK		|			|			|
#            |dev7		|room7		 |serviceul20	|IoTUL2		|attr_1_dev_7	|ping7	|dev7@%s	|dev7@ping7		|OK		|thing7		|room7		|
#            |dev8		|room8		 |serviceul20	|IoTUL2		|attr_1_dev_8	|ping8	|dev8@%s	|dev8@ping8		|OK		|			|room8		|
#            |dev9		|			 |serviceul20	|IoTUL2		|attr_1_dev_9	|ping9	|dev9@%s	|dev9@ping9		|OK		|thing9		|			|
#            |dev10		|			 |serviceul20	|IoTUL2		|attr_1_dev_10	|ping10	|dev10@%s	|dev10@ping10	|OK		|			|			|
                                    