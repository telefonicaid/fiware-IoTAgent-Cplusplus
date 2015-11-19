from iotqautils.gtwMqtt import Gw_MQTT_Utils
from common.steps import service_created_precond,device_with_entity_values_created_precond,device_with_attributes_created_precond,check_measure_cbroker,check_measures_cbroker,check_NOT_measure_cbroker,check_measure_cbroker_entity,check_measure_cbroker_with_attributes
from lettuce import step, world
import time, requests, datetime
from common.gw_configuration import MQTT_BROKER_HOSTNAME,MQTT_BROKER_PORT,CBROKER_URL,MQTT_APIKEY

mqtt = Gw_MQTT_Utils(mqtt_host=MQTT_BROKER_HOSTNAME,mqtt_port=MQTT_BROKER_PORT)
world.def_entity=True
world.device={}

# Steps

@step('I publish a MQTT message with device_id "([^"]*)", alias "([^"]*)" and payload "([^"]*)"')   
def publish_MQTT_message(step, device_id, alias, payload):
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = MQTT_APIKEY
    topic = apikey+'/'+device_id+'/'+alias
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    mqtt.publish_message(str(topic), str(payload), apikey)

@step('I publish a MQTT message with device_id "([^"]*)", alias "([^"]*)", payload "([^"]*)" and wrong field "([^"]*)"')   
def publish_invalid_MQTT_message(step, device_id, alias, payload, field):
    if field == 'apikey':
        apikey=""
    else:
        if world.service_name:
            apikey = 'apikey_'+world.service_name
        else:
            apikey = MQTT_APIKEY
    topic = apikey+'/'+device_id+'/'+alias
    print topic
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    world.field=field
    if field == 'topic':
        try:
            mqtt.publish_message("", str(payload), apikey)
        except ValueError as e:
            assert str(e) == "Invalid topic.", 'ERROR: Invalid topic. expected, ' + str(e) + ' received'
            print "Exception:" + str(e)
    elif field == 'payload':
        mqtt.publish_message(str(topic), "", apikey)
    else:
        mqtt.publish_message(str(topic), str(payload), apikey)

@step('I publish a MQTT message with device_id "([^"]*)", tag "([^"]*)" and payload "([^"]*)"')
def publish_several_MQTT_messages(step, device_id, tag,  payload):
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = MQTT_APIKEY
    topic = apikey+'/'+device_id+'/'+tag
    requests.post(CBROKER_URL+"/reset")
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    mqtt.publish_message(str(topic), str(payload), apikey)


@step('I Wait some time')
def wait(step):
    time.sleep(1)