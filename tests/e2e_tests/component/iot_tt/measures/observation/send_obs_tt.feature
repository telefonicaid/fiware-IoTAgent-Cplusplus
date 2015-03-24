Feature: Send TT Observation	
    	 As a device
    	 I want to send TT measures to context broker throught IoTAgent for TT
    	 In order to assure IotAgent_TT-Cbroker integration
    	
    	@iot_tt @IDAS-20233
    	Scenario Outline: Send a single observation
		Given a service with name "<service>" and protocol "<protocol>" created
		And a device with device name "<device_name>" created
		When I send a measure to the GW with name "<device_name>", protocol "<protocol>", type "<obs_type>" and value "<obs_value>" 
		Then "<num_measures>" measures of asset "<device_name>" with measures "<generated_measures>" are received by context broker
		
		Examples:
            |service	|device_name |obs_type	|obs_value						|protocol  |generated_measures																							|num_measures	|
            |servicett	|devicett1	 |GM		|temp#25/20&wakeUP				|IoTTT	   |temp:25/sleeptime#sleepcondition																			|2				|
            |servicett	|devicett1	 |GM		|temp#30/30:20&wakeUP2:wakeUP	|IoTTT	   |temp:30/sleeptime#sleepcondition																			|1				|
            |servicett	|devicett1	 |GM		|hum#35/40&wakeUP3				|IoTTT	   |hum:35/sleeptime#sleepcondition																				|1				|
            |servicett	|devicett1_1 |GM		|temp#44/20&					|IoTTT	   |temp:44/sleeptime#						 																	|2				|
            |servicett	|devicett1_2 |GM		|temp#55/&wakeUP				|IoTTT	   |temp:55/sleepcondition#																						|2				|
            |servicett	|devicett1_3 |GM		|temp#44/&						|IoTTT	   |temp:44/																									|2				|
            |servicett	|devicett2	 |GC		|temp#45						|IoTTT	   |_TTcurrent_temp:45/																							|2				|
            |servicett	|devicett2	 |GC		|temp#55:45						|IoTTT	   |_TTcurrent_temp:55/																							|1				|
            |servicett	|devicett2	 |GC		|hum#70							|IoTTT	   |_TTcurrent_hum:70/																							|1				|
            |servicett	|devicett2_1 |GC		|temp#45/22&wakeUP4				|IoTTT	   |_TTcurrent_temp:45/sleeptime#sleepcondition																	|2				|
            |servicett	|devicett3	 |P1		|11#22#33#44#55/40&wakeUP		|IoTTT	   |mcc#mnc#cell-id#lac#dbm/sleeptime#sleepcondition															|2				|
            |servicett	|devicett3_1 |P1		|11#22#33#44/40&wakeUP			|IoTTT	   |mcc#mnc#cell-id#lac/sleeptime#sleepcondition																|2				|
            |servicett	|devicett3_2 |P1		|11#22#33#44/					|IoTTT	   |mcc#mnc#cell-id#lac/																						|2				|
            |servicett	|devicett4	 |B			|12#23#34#45#56#67/50&wakeUP	|IoTTT	   |voltaje#estado_activacion#hay_cargador#estado_carga#modo_carga#tiempo_desco_stack/sleeptime#sleepcondition	|2				|
            |servicett	|devicett4_1 |B			|12#23#34#45#56#67/				|IoTTT	   |voltaje#estado_activacion#hay_cargador#estado_carga#modo_carga#tiempo_desco_stack/							|2				|
            |servicett	|devicett5	 |GPS		|11#22#33#44#55/40&wakeUP		|IoTTT	   |position:11,22/location#sleeptime#sleepcondition															|2				|
            |servicett	|devicett5_1 |GPS		|11#22#33#44/40&wakeUP			|IoTTT	   |position:11,22/location#sleeptime#sleepcondition															|2				|
            |servicett	|devicett5_2 |GPS		|11#22#33/40&wakeUP				|IoTTT	   |position:11,22/location#sleeptime#sleepcondition															|2				|
            |servicett	|devicett5_3 |GPS		|11#22/40&wakeUP				|IoTTT	   |position:11,22/location#sleeptime#sleepcondition															|2				|
            |servicett	|devicett5_4 |GPS		|11#22/							|IoTTT	   |position:11,22/location																						|2				|
#            |devicett6	 |K1		|90&wakeUP						|IoTTT	   |																											|0				|
         
    	@iot_tt @IDAS-20234
    	Scenario Outline: Send a multiple observation
		Given a service with name "<service>" and protocol "<protocol>" created
		And a device with device name "<device_name>" created
		When I send several measures to the GW with name "<device_name>", protocol "<protocol>" 
			|obs_type		|obs_value		|
			|<obs_type1>	|<obs_value1>	|
			|<obs_type2>	|<obs_value2>	|
			|<obs_type3>	|<obs_value3>	|
		Then "<num_measures>" measures of asset "<device_name>" with measures "<generated_measures>" are received by context broker
		
		Examples:
            |service	|device_name |obs_type1	|obs_value1						|obs_type2	|obs_value2					|obs_type3	|obs_value3						|protocol  |generated_measures																																				|num_measures	|
            |servicett	|devicett7	 |GM		|temp#25/20&wakeUP				|GM			|hum#35/40&wakeUP3			|			|								|IoTTT	   |temp:25/sleeptime#sleepcondition&hum:35/sleeptime#sleepcondition																								|2				|
            |servicett	|devicett8	 |GM		|temp#35/30&wakeUP				|P1			|11#22#33#44#55/40&wakeUP	|GC			|hum#45							|IoTTT	   |temp:35/sleeptime#sleepcondition&mcc#mnc#cell-id#lac#dbm/sleeptime#sleepcondition&_TTcurrent_hum:45/															|2				|
            |servicett	|devicett8	 |GM		|temp#45/40:30&wakeUP2:wakeUP	|GC			|hum#55						|B			|12#23#34#45#56#67/50&wakeUP	|IoTTT	   |temp:45/sleeptime#sleepcondition&_TTcurrent_hum:55/&voltaje#estado_activacion#hay_cargador#estado_carga#modo_carga#tiempo_desco_stack/sleeptime#sleepcondition	|1				|
            |servicett	|devicett8	 |GM		|temp#55/50:40&wakeUP3:wakeUP2	|GC			|hum#66						|GPS		|11#22#33#44#55/60&wakeUP6		|IoTTT	   |temp:55/sleeptime#sleepcondition&_TTcurrent_hum:66/&position:11,22/location#sleeptime#sleepcondition															|1				|
            |servicett	|devicett9	 |GM		|temp#65/60&wakeUP6				|K1			|90&wakeUP9					|GC			|hum#55							|IoTTT	   |temp:65/sleeptime#sleepcondition&_TTcurrent_hum:55/																												|2				|
            

    	@iot_tt @IDAS-20235
    	Scenario Outline: Send a wrong observation
		Given a service with name "<service>" and protocol "<protocol>" created
		And a device with device name "<device_name>" created
		When I send a measure to the GW with name "<device_name>", protocol "<protocol>", type "<obs_type>", value "<obs_value>" and with wrong field "<field>"
		Then "<num_measures>" measures of asset "<device_name>" with measures "<generated_measures>" and error "<error_text>" are received or NOT by context broker
		
		Examples:
            |service	|device_name |obs_type	|obs_value					|protocol  	|field 		|generated_measures						|error_text							|num_measures	|
            |servicett	|devicett10	 |GM		|temp#						|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett10	 |GM		|temp#/20&wakeUP			|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett11	 |GC		|temp#						|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett11	 |GC		|temp#/20&wakeUP			|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett12	 |P1		|11#22#33#/					|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett12	 |P1		|11#22#33#/40&wakeUP		|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett13	 |B			|12#23#34#45#56/			|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett13	 |B			|12#23#34#45#56/40&wakeUP	|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett13	 |B			|12#23#34#45#56#/			|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett13	 |B			|12#23#34#45#56#/40&wakeUP	|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett14	 |GPS		|14#/						|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett14	 |GPS		|141#/40&wakeUP				|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett14	 |GPS		|142/						|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett14	 |GPS		|143/40&wakeUP				|IoTTT		|value		|										|Error While processing TT message	|0				|
            |servicett	|devicett15	 |GM		|temp#88/20&wakeUP			|IoTTT		|stack_id	|										|Error While processing TT message	|0				|
            |servicett	|devicett16	 |GM		|temp#99/20&wakeUP			|IoTTT		|bus_id		|										|Error While processing TT message	|0				|
            