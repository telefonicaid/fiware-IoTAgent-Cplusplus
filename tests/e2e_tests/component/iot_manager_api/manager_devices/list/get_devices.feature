Feature: Manager API Device Get List
    	 As an application
    	 I want to list Devices
    	 In order to include Devices into IotAgent Manager
    	
    	@iot_manager_api @IDAS-20441 
    	Scenario Outline: List Devices data
		Given a Service with name "<srv_name>", path "<srv_path>" and protocol "<protocol>" created
		And a Device with name "<dev_id>", protocol "<protocol>", entity_name "<ent_name>" and entity_type "<ent_type>" created
		When I list in manager the devices of "<find_srv>", path "<find_path>", entity "<find_ent>", protocol "<find_prot>", detailed "<detail>", limit "<limit>" and offset "<offset>"
        Then I receive the device data of "<n_devs>" devices with data "<devices>"
		
		Examples:
			|srv_name		|srv_path			  |protocol	|dev_id		|ent_name	|ent_type		|find_srv		|find_path			  |find_ent	  |find_prot |n_devs |limit	|offset	|detail	|devices															|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list	|room_list	|thing_list		|srv_devm_list	|/path_srv_devm_list  |			  |			 |1		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list  |room_list1 |			 |1		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list1 |IoTTT	|devm_list	|room_list	|thing_list		|srv_devm_list	|/path_srv_devm_list1 |room_list1 |			 |0		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list1 |IoTTT	|devm_list	|room_list	|thing_list		|srv_devm_list	|/path_srv_devm_list1 |			  |IoTTT	 |1		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list	|room_list	|thing_list		|srv_devm_list	|/path_srv_devm_list  |			  |IoTUL2	 |2		 |		|		|		|devm_list/devm_list1												|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list	|room_list	|thing_list		|srv_devm_list	|/path_srv_devm_list  |			  |IoTUL2	 |2		 |		|		|on		|devm_list#room_list:thing_list/devm_list1#room_list1:thing_list1	|
			|srv_devm_list	|/path_srv_devm_list1 |IoTTT	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list1 |room_list1 |			 |1		 |		|		|on		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list  |			  |IoTTT	 |0		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list  |			  |IoTUnknown|0		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list  |room_list1 |IoTUL2	 |1		 |		|		|on		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list  |room_list1 |IoTTT	 |0		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list  |room_list2 |IoTUL2	 |0		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list2	|room_list2	|thing_list2	|srv_devm_list	|/path_srv_devm_list  |			  |			 |3		 |		|		|off	|devm_list/devm_list1/devm_list2									|
			|srv_devm_list	|/path_srv_devm_list1 |IoTUL2	|devm_list1	|room_list1	|thing_list1	|srv_devm_list	|/path_srv_devm_list1 |			  |			 |2		 |		|		|on		|devm_list#room_list:thing_list/devm_list1#room_list1:thing_list1	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list3	|room_list3	|thing_list3	|srv_devm_list	|/path_srv_devm_list  |			  |			 |2		 |2		|		|on		|devm_list#room_list:thing_list/devm_list1#room_list1:thing_list1	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list4	|room_list4	|thing_list4	|srv_devm_list	|/path_srv_devm_list  |			  |			 |2		 |2		|2		|on		|devm_list2#room_list2:thing_list2/devm_list3#room_list3:thing_list3|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list5	|room_list5	|thing_list5	|srv_devm_list	|/path_srv_devm_list  |			  |			 |4		 |		|2		|off	|devm_list2/devm_list3/devm_list4/devm_list5						|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list6	|room_list6	|thing_list6	|srv_devm_list	|/path_srv_devm_list  |			  |			 |4		 |4		|2		|		|devm_list2/devm_list3/devm_list4/devm_list5						|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list	|room_list	|thing_list		|srv_devm_list	|/path_srv_devm_list2 |			  |			 |0		 |		|		|		|																	|
			|srv_devm_list	|/path_srv_devm_list  |IoTUL2	|devm_list	|room_list	|thing_list		|srv_devm_list1	|/path_srv_devm_list  |			  |			 |0		 |		|		|		|																	|
			|srv_devm_list	|void				  |IoTUL2	|devm_list	|room_list	|thing_list		|srv_devm_list	|void				  |			  |			 |1		 |		|		|on		|																	|


    	@iot_manager_api @IDAS-20442 
    	Scenario Outline: Retrieve devices data with invalid fields
		When I try to retrieve the devices data of "<srv_name>", path "<srv_path>", entity "<find_ent>", protocol "<find_prot>", detailed "<detailed>", limit "<limit>" and offset "<offset>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name		|srv_path			  |find_ent	  |find_prot |detailed		|limit			|offset			|HTTP_status	|exceptionText 									|
			|void			|void				  |			  |			 |				|				|				|400			|Fiware-Service not accepted					|
			|void			|/path_srv_devm_list  |			  |			 |				|				|				|400			|Fiware-Service not accepted					|
			|srv_devm_list	|path_srv_devm_list	  |			  |			 |				|				|				|400			|Fiware-ServicePath not accepted				|
			|srv_devm_list	|/path_srv_devm_list  |			  |			 |wrong_detail	|				|				|400			|parameter detailed must be on or off			|
			|srv_devm_list	|/path_srv_devm_list  |			  |			 |				|wrong_limit	|				|400			|limit must be a number but it is wrong_limit	|
			|srv_devm_list	|/path_srv_devm_list  |			  |			 |				|				|wrong_offset	|400			|offset must be a number but it is wrong_offset	|
#			|srv_devm_list	|/path_srv_devm_list  |			  |IoTTT	 |				|				|				|404			|there aren't iotagents for this operation		|
#			|srv_devm_list	|/path_srv_devm_list  |room_list1 |IoTTT	 |				|				|				|404			|there aren't iotagents for this operation		|
#			|srv_devm_list	|/path_srv_devm_list2 |			  |			 |				|				|				|404			|there aren't iotagents for this operation		|
#			|srv_devm_list1	|/path_srv_devm_list  |			  |			 |				|				|				|404			|there aren't iotagents for this operation		|
			