Feature: TT Observation	Send
    	 As a device
    	 I want to send TT measures to context broker throught IoTAgent for TT
    	 In order to assure IotAgent_TT-Cbroker integration
    	
    	@iot_tt @IDAS-20233
    	Scenario Outline: Send a single observation
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_name>" and protocol "<protocol>" created
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
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_name>" and protocol "<protocol>" created
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
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_name>" and protocol "<protocol>" created
		When I send a measure to the GW with name "<device_name>", protocol "<protocol>", type "<obs_type>", value "<obs_value>" and with wrong field "<field>"
		Then "<num_meas>" measures of asset "<device_name>" with measures "<gen_meas>" and error "<error_text>" are received or NOT by context broker
		
		Examples:
            |service	|device_name |obs_type	|obs_value					|protocol  	|field 		|error_text									|num_meas	|gen_meas	|
            |servicett	|devicett10	 |GM		|temp#						|IoTTT		|value		|											|0			|			|
            |servicett	|devicett10	 |GM		|temp#/20&wakeUP			|IoTTT		|value		|											|0			|			|
            |servicett	|devicett11	 |GC		|temp#						|IoTTT		|value		|											|0			|			|
            |servicett	|devicett11	 |GC		|temp#/20&wakeUP			|IoTTT		|value		|											|0			|			|
            |servicett	|devicett12	 |P1		|11#22#33#/					|IoTTT		|value		|											|0			|			|
            |servicett	|devicett12	 |P1		|11#22#33#/40&wakeUP		|IoTTT		|value		|											|0			|			|
            |servicett	|devicett13	 |B			|12#23#34#45#56/			|IoTTT		|value		|											|0			|			|
            |servicett	|devicett13	 |B			|12#23#34#45#56/40&wakeUP	|IoTTT		|value		|											|0			|			|
            |servicett	|devicett13	 |B			|12#23#34#45#56#/			|IoTTT		|value		|											|0			|			|
            |servicett	|devicett13	 |B			|12#23#34#45#56#/40&wakeUP	|IoTTT		|value		|											|0			|			|
            |servicett	|devicett14	 |GPS		|14#/						|IoTTT		|value		|											|0			|			|
            |servicett	|devicett14	 |GPS		|141#/40&wakeUP				|IoTTT		|value		|											|0			|			|
            |servicett	|devicett14	 |GPS		|142/						|IoTTT		|value		|											|0			|			|
            |servicett	|devicett14	 |GPS		|143/40&wakeUP				|IoTTT		|value		|											|0			|			|
            |servicett	|devicett15	 |GM		|temp#88/20&wakeUP			|IoTTT		|stack_id	|Error: TT message not properly formatted	|0			|			|
            |servicett	|devicett16	 |GM		|temp#99/20&wakeUP			|IoTTT		|bus_id		|											|0			|			|

    	@iot_tt @IDAS-20320
    	Scenario Outline: Send a wrong multiple observation
		Given a Service with name "<service>" and protocol "<protocol>" created
		And a Device with name "<device_name>" and protocol "<protocol>" created
		When I send several measures to the GW with name "<device_name>", protocol "<protocol>" 
			|obs_type		|obs_value		|
			|<obs_type1>	|<obs_value1>	|
			|<obs_type2>	|<obs_value2>	|
		Then "<num_measures>" measures of asset "<device_name>" with measures "<generated_measures>" and error "<error_text>" are received or NOT by context broker
		
		Examples:
            |service	|device_name |obs_type1	|obs_value1						|obs_type2	|obs_value2					|protocol  |generated_measures																	|num_measures	|error_text							|
            |servicett	|devicett17	 |GM		|temp#15/10&wakeUP				|GM			|hum#/40&wakeUP4			|IoTTT	   |temp:15/sleeptime#sleepcondition													|2				|									|
            |servicett	|devicett17	 |GC		|temp#25:15/20:10&wakeUP1:wakeUP|GM			|temp#/50&wakeUP5			|IoTTT	   |_TTcurrent_temp:25/sleeptime#sleepcondition											|1				|									|
            |servicett	|devicett17	 |P1		|11#22#33#44#55/40&wakeUP		|GM			|temp#/50&wakeUP5			|IoTTT	   |mcc#mnc#cell-id#lac#dbm/sleeptime#sleepcondition									|1				|									|
            |servicett	|devicett17	 |B			|12#23#34#45#56#67/				|GM			|temp#/50&wakeUP5			|IoTTT	   |voltaje#estado_activacion#hay_cargador#estado_carga#modo_carga#tiempo_desco_stack/	|1				|									|
            |servicett	|devicett17	 |GPS		|11#22#33#44#55/40&wakeUP		|GM			|temp#/50&wakeUP5			|IoTTT	   |position:11,22/location#sleeptime#sleepcondition									|1				|									|
            |servicett	|devicett17	 |GM		|temp#35/30&wakeUP				|GC			|temp#/60&wakeUP6			|IoTTT	   |temp:35/sleeptime#sleepcondition													|1				|									|
            |servicett	|devicett17	 |GC		|hum#35/33&wakeUP3				|GC			|temp#/50&wakeUP5			|IoTTT	   |_TTcurrent_hum:35/sleeptime#sleepcondition											|1				|									|
            |servicett	|devicett18	 |GM		|temp#45/40&wakeUP2				|P1			|11#22#33#/70&wakeUP7		|IoTTT	   |temp:45/sleeptime#sleepcondition													|2				|									|
            |servicett	|devicett18	 |GM		|temp#55/50:40&wakeUP3:wakeUP2	|B			|12#23#34#45#56#/80&wakeUP8	|IoTTT	   |temp:55/sleeptime#sleepcondition													|1				|            						|
            |servicett	|devicett18	 |GM		|temp#65/60:50&wakeUP4:wakeUP3	|GPS		|11#/90&wakeUP9				|IoTTT	   |temp:65/sleeptime#sleepcondition													|1				|									|
            |servicett	|devicett18	 |GM		|hum#/40&wakeUP4				|GC			|temp#/50&wakeUP5			|IoTTT	   |																					|0				|									|
            