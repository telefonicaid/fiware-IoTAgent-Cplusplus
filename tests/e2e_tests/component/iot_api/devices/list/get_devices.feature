Feature: Get List Devices
    	 As an application
    	 I want to list Devices
    	 In order to include Devices into IotAgent
    	
    	@iot_api @IDAS-18463 
    	Scenario Outline: List Devices data
		Given a Service with name "<srv_name>" and path "<srv_path>" created
		And a Device with name "<dev_id>", entity_name "<ent_name>" and entity_type "<ent_type>" created
		When I list the devices of "<find_srv>", path "<find_path>", entity "<find_ent>", detailed "<detail>", limit "<limit>" and offset "<offset>"
        Then I receive the device data of "<n_devs>" devices with data "<devices>"
		
		Examples:
			|srv_name	|srv_path			|dev_id		|ent_name	|ent_type		|find_srv	|find_path			|find_ent	|n_devs	|limit	|offset	|detail	|devices															|
			|srv_list	|/path_srv_list		|dev_list	|room_list	|thing_list		|srv_list	|/path_srv_list		|			|1		|		|		|		|																	|
			|srv_list	|/path_srv_list		|dev_list1	|room_list1	|thing_list1	|srv_list	|/path_srv_list		|room_list1	|1		|		|		|		|																	|
			|srv_list	|/path_srv_list1	|dev_list	|room_list	|thing_list		|srv_list	|/path_srv_list1	|room_list1	|0		|		|		|		|																	|
			|srv_list	|/path_srv_list1	|dev_list1	|room_list1	|thing_list1	|srv_list	|/path_srv_list1	|room_list1	|1		|		|		|on		|																	|
			|srv_list	|/path_srv_list		|dev_list2	|room_list2	|thing_list2	|srv_list	|/path_srv_list		|			|3		|		|		|off	|dev_list/dev_list1/dev_list2										|
			|srv_list	|/path_srv_list1	|dev_list1	|room_list1	|thing_list1	|srv_list	|/path_srv_list1	|			|2		|		|		|on		|dev_list#room_list:thing_list/dev_list1#room_list1:thing_list1		|
			|srv_list	|/path_srv_list		|dev_list3	|room_list3	|thing_list3	|srv_list	|/path_srv_list		|			|2		|2		|		|on		|dev_list#room_list:thing_list/dev_list1#room_list1:thing_list1		|
			|srv_list	|/path_srv_list		|dev_list4	|room_list4	|thing_list4	|srv_list	|/path_srv_list		|			|2		|2		|2		|on		|dev_list2#room_list2:thing_list2/dev_list3#room_list3:thing_list3	|
			|srv_list	|/path_srv_list		|dev_list5	|room_list5	|thing_list5	|srv_list	|/path_srv_list		|			|4		|		|2		|off	|dev_list2/dev_list3/dev_list4/dev_list5							|
			|srv_list	|/path_srv_list		|dev_list6	|room_list6	|thing_list6	|srv_list	|/path_srv_list		|			|4		|4		|2		|		|dev_list2/dev_list3/dev_list4/dev_list5							|
			|srv_list	|/path_srv_list		|dev_list	|room_list	|thing_list		|srv_list	|/path_srv_list2	|			|0		|		|		|		|																	|
			|srv_list	|/path_srv_list		|dev_list	|room_list	|thing_list		|srv_list1	|/path_srv_list		|			|0		|		|		|		|																	|
			|srv_list	|void				|dev_list	|room_list	|thing_list		|srv_list	|void				|			|1		|		|		|on		|																	|


    	@iot_api @IDAS-18463 
    	Scenario Outline: Retrieve devices data with invalid fields
		When I try to retrieve the devices data of "<srv_name>", path "<srv_path>", detailed "<detailed>", limit "<limit>" and offset "<offset>"
        Then user receives the "<HTTP_status>" and the "<exceptionText>"
		
		Examples:
			|srv_name	|srv_path		|detailed		|limit			|offset			|HTTP_status	|exceptionText 												|
			|void		|void			|				|				|				|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|void		|/path_srv_get	|				|				|				|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|srv_get	|path_srv_get	|				|				|				|400			|Fiware-Service/Fiware-ServicePath not present or invalid	|
			|srv_get	|/path_srv_get	|wrong_detail	|				|				|400			|parameter detailed must be on or off						|
			|srv_get	|/path_srv_get	|				|wrong_limit	|				|400			|limit must be a number but it is wrong_limit				|
			|srv_get	|/path_srv_get	|				|				|wrong_offset	|400			|offset must be a number but it is wrong_offset				|
			