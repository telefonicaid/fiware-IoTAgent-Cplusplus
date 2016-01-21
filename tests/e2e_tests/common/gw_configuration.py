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
#GW_HOSTNAME='iotagent'
#IOT_HOSTNAME='10.95.213.36'
IOT_HOSTNAME='192.168.21.77'
#MODBUS_HOSTNAME='10.95.213.36'
#MODBUS_HOSTNAME='localhost'
MODBUS_HOSTNAME='192.168.21.77'
GW_PORT='8002'
IOT_PORT='8087'
#IOT_PORT='8080'
MANAGER_PORT='8081'
#TLG_PORT='9081'
TLG_PORT='8091'
IOT_SERVER_ROOT = 'http://{}:{}'.format(IOT_HOSTNAME, IOT_PORT)
MANAGER_SERVER_ROOT = 'http://{}:{}'.format(IOT_HOSTNAME, MANAGER_PORT)
TLG_SERVER_ROOT = '{}:{}'.format(MODBUS_HOSTNAME, TLG_PORT)
#CBROKER_URL='http://10.95.213.159:6500'
CBROKER_URL='http://192.168.22.218:6500'
#CBROKER_URL_TLG='http://10.95.213.159:6502'
CBROKER_URL_TLG='http://192.168.22.218:6500'
CBROKER_HEADER='Fiware-Service'
CBROKER_PATH_HEADER='Fiware-ServicePath'
SMPP_URL='http://sbc04:5371'
SMPP_FROM='682996050'
DEF_ENTITY_TYPE='thing'
DEF_TYPE='string'
PATH_UL20_COMMAND='/iot/ngsi/d/updateContext'
PATH_MQTT_COMMAND='/iot/ngsi/mqtt/updateContext'
PATH_TLG_COMMAND='/iot/ngsi/tgrepsol/updateContext'
PATH_UL20_SIMULATOR='/simulaClient/ul20Command'
TIMEOUT_COMMAND=10
MQTT_APIKEY='1234'
UL20_APIKEY='apikey3'
TIMESTAMP=2
KS_DOMAIN='repsolglp'
KS_USER='repsol_admin'
KS_PASSWD='password'
KS_IP='192.168.21.64'
KS_PORT='5001'