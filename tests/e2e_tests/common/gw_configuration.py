# -*- coding: utf-8 -*-
'''
(c) Copyright 2013 Telefonica, I+D. Printed in Spain (Europe). All Rights
Reserved.

The copyright to the software program(s) is property of Telefonica I+D.
The program(s) may be used and or copied only with the express written
consent of Telefonica I+D or in accordance with the terms and conditions
stipulated in the agreement/contract under which the program(s) have
been supplied.
'''

HEADERS={'content-type': 'application/json'}
MQTT_BROKER_HOSTNAME='iotagent'
MQTT_BROKER_PORT='1883'
GW_HOSTNAME='iotagent'
GW_PORT='8002'
IOT_PORT='8080'
MANAGER_PORT='8081'
GW_SERVER_ROOT = 'http://{}:{}'.format(GW_HOSTNAME, GW_PORT)
IOT_SERVER_ROOT = 'http://{}:{}'.format(GW_HOSTNAME, IOT_PORT)
MANAGER_SERVER_ROOT = 'http://{}:{}'.format(GW_HOSTNAME, MANAGER_PORT)
CBROKER_URL='http://10.95.213.159:6500'
CBROKER_HEADER='Fiware-Service'
CBROKER_PATH_HEADER='Fiware-ServicePath'
SMPP_URL='http://sbc04:5371'
SMPP_FROM='682996050'
DEF_ENTITY_TYPE='thing'
DEF_TYPE='string'
PATH_UL20_COMMAND='/iot/ngsi/d/updateContext'
PATH_MQTT_COMMAND='/iot/ngsi/mqtt/updateContext'
PATH_UL20_SIMULATOR='/simulaClient/ul20Command'
TIMEOUT_COMMAND=10
MQTT_APIKEY='1234'
UL20_APIKEY='apikey3'