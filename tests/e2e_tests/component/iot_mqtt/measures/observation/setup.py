from iotqautils.gtwMqtt import Gw_MQTT_Utils
from lettuce import step, world
import time, requests, datetime
from common.gw_configuration import MQTT_BROKER_HOSTNAME,MQTT_BROKER_PORT,CBROKER_URL,CBROKER_HEADER,MQTT_APIKEY,DEF_ENTITY_TYPE
from common.user_steps import UserSteps

mqtt = Gw_MQTT_Utils(mqtt_host=MQTT_BROKER_HOSTNAME,mqtt_port=MQTT_BROKER_PORT)
user_steps = UserSteps()

# Steps
@step('a service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
        world.service_name = service_name
        user_steps.service_precond(service_name, protocol)

@step('I publish a MQTT message with device_id "([^"]*)", alias "([^"]*)" and payload "([^"]*)"')   
def publish_MQTT_message(step, device_id, alias, payload):
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = MQTT_APIKEY
    topic = apikey+'/'+device_id+'/'+alias
    replaces = {
        "True" : "1",
        "False": "0",
        "true": "1",
        "false": "0",
        ",": "/"
    }
    #for kreplace in replaces:
    #    payload = payload.replace(kreplace,replaces[kreplace])
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
    replaces = {
        "True" : "1",
        "False": "0",
        "true": "1",
        "false": "0",
        ",": "/"
    }
    #for kreplace in replaces:
    #    payload = payload.replace(kreplace,replaces[kreplace])
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
    replaces = {
        "True" : "1",
        "False": "0",
        "true": "1",
        "false": "0",
        ",": "/"
    }
    #for kreplace in replaces:
    #    payload = payload.replace(kreplace,replaces[kreplace])
    requests.post(CBROKER_URL+"/reset")
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    mqtt.publish_message(str(topic), str(payload), apikey)


@step('I Wait some time')
def wait(step):
    time.sleep(1)

   
@step('the measure of asset "([^"]*)" with phenom "([^"]*)" and value "([^"]*)" is received by context broker')
def check_measure_cbroker(step, asset_name, phenom_name, value):
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Dispositivo {}'.format(assetElement)
    valueElement = contextElement['attributes'][0]['value']
    #print '{}.{}'.format(model_name,asset_name)
    nameElement = contextElement['attributes'][0]['name']
    nameTime = contextElement['attributes'][1]['name']
    asset_name = DEF_ENTITY_TYPE + ':' + asset_name
    #print 'Metadatas {}'.format(metasElement)
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    typeElement = contextElement['type']
    assert typeElement == "thing", 'ERROR: ' + str(contextElement)
    assert valueElement == value, 'ERROR: ' + value + ' value expected, ' + valueElement + ' received'
    assert nameElement == phenom_name, 'ERROR: ' + phenom_name + ' name expected, ' + nameElement + ' received'
    assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
    assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
    assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
    assert check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
    assert check_timestamp(contextElement['attributes'][1]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])

@step('"([^"]*)" measures of asset "([^"]*)" are received by context broker')
def check_measures_cbroker(step, num_measures, asset_name):
    time.sleep(1)
    measures_count =  requests.get(CBROKER_URL+"/countMeasure")
    assert measures_count.text == str(num_measures), 'ERROR: ' + str(num_measures) + ' measures expected, ' + measures_count.text + ' received'
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    valueElement = contextElement['attributes'][0]['value']
    #print '{}.{}'.format(model_name,asset_name)
    nameElement = contextElement['attributes'][0]['name']
    nameTime = contextElement['attributes'][1]['name']
    asset_name = DEF_ENTITY_TYPE + ':' + asset_name
    #attrsElement = contextElement['attributes']
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    typeElement = contextElement['type']
    assert typeElement == "thing", 'ERROR: type thing not found in: ' + str(contextElement)
    #num_attrs=0
    #while num_attrs<world.measures_count:
    isValue = False
    for measures_dict in step.hashes:
        valueDict = measures_dict['value']
        if (valueDict == valueElement) & (measures_dict['alias']==nameElement):
            print 'Encontrado valor {}'.format(valueDict)
            isValue = True
            break
    assert isValue, 'ERROR: ' + valueDict + ' value expected, ' + valueElement + ' received'
    #    num_attrs=num_attrs+1   
    assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
    assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
    assert check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
    assert check_timestamp(contextElement['attributes'][1]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])

@step('the measure of asset "([^"]*)" with phenom "([^"]*)" and value "([^"]*)" is received or NOT by context broker')
def check_NOT_measure_cbroker(step, asset_name, phenom_name, value):
    time.sleep(1)
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    assert typeElement == "thing", 'ERROR: type thing not found in: ' + str(contextElement)
    asset_name = DEF_ENTITY_TYPE + ':' + asset_name
    if world.field == "payload":
        contextElement = response['contextElements'][0]
        assetElement = contextElement['id']
        assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
        print 'Compruebo atributo TimeInstant y {} en {}'.format(str(world.st),str(contextElement['attributes'][0]))
        assert contextElement['attributes'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0])
        assert check_timestamp(contextElement['attributes'][0]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0])
#        assert str(world.st) in contextElement['attributes'][0]['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0])
        assert len(contextElement['attributes']) == 1, 'ERROR: Atribute ' + str(contextElement['attributes'][1]) + " found in: " + str(contextElement['attributes'])
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measure is NOT received"

def check_timestamp (timestamp):
    st = datetime.datetime.utcfromtimestamp(world.ts).strftime('%Y-%m-%dT%H:%M:%S')
    if st in timestamp:
        return True
    else:
        st = datetime.datetime.utcfromtimestamp(world.ts+1).strftime('%Y-%m-%dT%H:%M:%S')
        if st in timestamp:
            return True
        else:
            st = datetime.datetime.utcfromtimestamp(world.ts-1).strftime('%Y-%m-%dT%H:%M:%S')
            if st in timestamp:
                return True
            else:
                return False