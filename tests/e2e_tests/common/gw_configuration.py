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

REST_HOSTNAME='dev-idas-fe-01'
#REST_HOSTNAME='sbc04'
REST_PORT='5371'
REST_SERVER_ROOT = 'http://{}:{}/m2m/v2'.format(REST_HOSTNAME, REST_PORT)
REST_SERVER_ROOT_SECURE = 'http://{}:{}/secure/m2m/v2'.format(REST_HOSTNAME, REST_PORT)
HEADERS={'content-type': 'application/json'}
#MQTT_BROKER_HOSTNAME='dev-idas-gw-01'
MQTT_BROKER_HOSTNAME='iotagent'
MQTT_BROKER_PORT='1883'
#GW_HOSTNAME='dev-idas-gw-01'
GW_HOSTNAME='iotagent'
GW_PORT='8002'
IOT_PORT='80'
GW_SERVER_ROOT = 'http://{}:{}'.format(GW_HOSTNAME, GW_PORT)
IOT_SERVER_ROOT = 'http://{}:{}'.format(GW_HOSTNAME, IOT_PORT)
MeasuresType = {
    "UL": "/idas/2.0?apikey={}&ID={}",
    "UL2": "/d?k={}&i={}",
    "IoTUL2": "/iot/d?k={}&i={}",
    "IoTRepsol": "/iot/repsol",
    "RegLight": "/idas/2.0?apikey={}"
}
#CBROKER_URL='http://dev-idas-cep-02:6500'
#CBROKER_URL='http://iotagent:6500'
CBROKER_URL='http://10.95.213.159:6500'
CBROKER_HEADER='Fiware-Service'
CBROKER_PATH_HEADER='Fiware-ServicePath'
IDM_URL = 'https://10.95.82.63/oauth2/token'
IDM_SECRET = 'Basic MTc6MzI3ZTYyZGUwNWM4ZWQ1ZGM5N2MxOTUxOTliOTEyNjg1NjNmYWU0NWFlMTdmYjgwODFkOGE0MTllZDg1NTdmODI1NGI4YWQ5YWRlNjgyNmJlOTgzNTBhMWIyY2EyNjJlNzQ1YmU2NGJjYzU2YWZjOTVkMDMwYmNjOWZkMGI2NjM='
SMPP_URL='http://sbc04:5371'
SMPP_FROM='682996050'
REPSOL_SERVICE='servicerepsol'
UL20_SERVICE='serviceul20'
UL20_SUBSERVICE='path_serviceul20'
UL20_APIKEY='apikey_serviceul20'
MQTT_SERVICE='serviceMqtt'
MQTT_APIKEY='1234'
EVADTS_SERVICE='666'
EVADTS_APIKEY='apikey3'
#TT_SERVICE='Myservice'
TT_SERVICE='serviceTT'
DEF_ENTITY_TYPE='thing'
PATH_IOT_COMMAND='/iot/ngsi/d/updateContext'
PATH_UL20_COMMAND='/simulaClient/ul20Command'
TIMEOUT_COMMAND=10
#Mapeo de fenomenos ultralight
acceleration='8:19'
alarm='8:73'
altitude='8:30'
amount='8:78'
amountOfSubstance='8:62'
angle='8:52'
angularVelocity='8:57'
atmosphericPressure='8:9'
averagePower='8:35'
batteryCharge='8:49'
BloodPressure='8:46'
CH4Concentration='8:69'
CO2Concentration='8:15'
COConcentration='8:16'
concentration='8:11'
connectivity='8:31'
cost='8:40'
counter='8:74'
currency='8:39'
direction='8:4'
electricCurrent='8:25'
electricPotential='8:24'
energy='8:38'
energyCO2='8:42'
energyCost='8:41'
event='8:0'
fillLevel='8:80'
flow='8:68'
frecuency='8:43'
gasConcentration='8:50'
GlucoseConcentration='8:45'
HeartRate='8:47'
illuminance='8:72'
latitude='8:29'
length='8:26'
location='8:27'
longitude='8:28'
luminousFlux='8:51'
luminousIntensity='8:61'
magneticFluxIntensity='8:53'
mass='8:2'