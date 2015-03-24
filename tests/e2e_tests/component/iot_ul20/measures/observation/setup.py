from lettuce import step, world
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,IOT_SERVER_ROOT,UL20_SERVICE,UL20_APIKEY,DEF_ENTITY_TYPE
from common.user_steps import UserSteps
import requests
import time,datetime

gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
user_steps = UserSteps()

@step('a service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
        world.service_name = service_name
        user_steps.service_precond(service_name, protocol)

@step('a device with device id "([^"]*)", entity type "([^"]*)" and entity name "([^"]*)" created')
def device_with_entity_values_created_precond(step, device_id, ent_type, ent_name):
    user_steps.device_precond(device_id, {}, {}, ent_name, ent_type)
    world.device_id=device_id

@step('I send a measure to the GW with apikey, id "([^"]*)", protocol "([^"]*)", alias "([^"]*)", timestamp "([^"]*)" and value "([^"]*)"')
def send_measure(step, device_id, protocol, alias, timestamp, value):
    measures= []
    measure = {}
    measure['alias'] = alias
    measure['value'] = value
    world.timestamp=False
    if timestamp:
        measure['timestamp'] = timestamp
        world.st=timestamp
        world.timestamp=True
    else:
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
    measures.append(measure)
    #print 'protocol: ' + protocol
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = UL20_APIKEY
    req = gw.sendMeasure(protocol,apikey,device_id,measures)
    #print req.status_code + req.ok
    assert req.ok, 'ERROR: ' + req.text


@step('I send several measures to the GW with apikey, id "([^"]*)", protocol "([^"]*)"')
def send_measures(step, device_id, protocol):
    measures = []
    world.measures_count=0
    for measures_dict in step.hashes:
        measure = {}
        measure['alias'] = measures_dict['alias']
        measure['value'] = measures_dict['value']
        measures.append(measure)
        world.measures_count+=1
    print 'measures: ' + str(measures)
    ts = time.time()
    st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
    world.st=st
    world.ts=ts
    requests.post(CBROKER_URL+"/reset")
    #apikey=api.get_apikey(world.service_name)
    req = gw.sendMeasure(protocol,UL20_APIKEY,device_id,measures)
    #print req.status_code + req.ok
    assert req.ok, 'ERROR: ' + req.text
        

@step('I send a wrong measure to the GW with apikey, id "([^"]*)", protocol "([^"]*)", alias "([^"]*)", value "([^"]*)", timestamp "([^"]*)" and wrong field "([^"]*)"')
def send_wrong_measure(step, device_id, protocol, alias, value, timestamp, field):
    measures= []
    measure = {}
    measure['alias'] = alias
    measure['value'] = value
    if field=="timestamp":
        measure['timestamp'] = timestamp
        ts = time.time()
        st = datetime.datetime.utcfromtimestamp(ts).strftime('%Y-%m-%dT%H:%M:%S')
        world.st=st
        world.ts=ts
    measures.append(measure)
    #print 'protocol: ' + protocol
    #apikey=api.get_apikey(world.service_name)
    world.field=field
    if field=="not_apikey":
        req = gw.sendMeasure("/iot/d?i={}","",device_id,measures)
    elif field=="not_device":
        req = gw.sendMeasure("/iot/d?k={}",UL20_APIKEY,"",measures)        
    elif field=="wrong_apikey":
        req = gw.sendMeasure(protocol,field,device_id,measures)
    else:
        req = gw.sendMeasure(protocol,UL20_APIKEY,device_id,measures)
    #print req.status_code + req.ok
    if field:
        if field=="timestamp":
            assert req.ok, 'ERROR: ' + str(req)
        else:
            assert not req.ok, 'ERROR: ' + str(req)
    else:
        assert req.ok, 'ERROR: ' + str(req)

@step('the measure of asset "([^"]*)" with phenom "([^"]*)" and value "([^"]*)" is received by context broker')
def check_measure_cbroker(step, asset, phenom_name, value):
    time.sleep(1)
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == UL20_SERVICE, 'ERROR de Cabecera: ' + str(req.headers[CBROKER_HEADER])
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Dispositivo {}'.format(assetElement)
    valueElement = contextElement['attributes'][0]['value']
    #print '{}.{}'.format(model_name,asset_name)
    nameElement = contextElement['attributes'][0]['name']
    nameTime = contextElement['attributes'][1]['name']
    #print 'Metadatas {}'.format(metasElement)
    asset_name = DEF_ENTITY_TYPE + ':' + asset
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    typeElement = contextElement['type']
    assert typeElement == DEF_ENTITY_TYPE, 'ERROR: ' + DEF_ENTITY_TYPE + ' type expected, ' + typeElement + ' received'
    assert valueElement == value, 'ERROR: ' + value + ' value expected, ' + valueElement + ' received'
    assert nameElement == phenom_name, 'ERROR: ' + phenom_name + ' name expected, ' + nameElement + ' received'
    assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
    assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
    if world.timestamp:
        assert str(world.st) == contextElement['attributes'][0]['metadatas'][0]['value'], 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
        assert str(world.st) == contextElement['attributes'][1]['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])
    else:
        assert check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
        assert check_timestamp(contextElement['attributes'][1]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])


@step('"([^"]*)" measures of asset "([^"]*)" are received by context broker')
def check_measures_cbroker(step, num_measures, asset):
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
    #attrsElement = contextElement['attributes']
    asset_name = DEF_ENTITY_TYPE + ':' + asset
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    typeElement = contextElement['type']
    assert typeElement == DEF_ENTITY_TYPE, 'ERROR: ' + DEF_ENTITY_TYPE + ' type expected, ' + typeElement + ' received'
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
def check_NOT_measure_cbroker(step, asset, phenom_name, value):
    time.sleep(1)
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    assert typeElement == DEF_ENTITY_TYPE, 'ERROR: ' + DEF_ENTITY_TYPE + ' type expected, ' + typeElement + ' received'
    asset_name = DEF_ENTITY_TYPE + ':' + asset
    if world.field == "timestamp":
        contextElement = response['contextElements'][0]
        assetElement = contextElement['id']
        valueElement = contextElement['attributes'][0]['value']
        nameElement = contextElement['attributes'][0]['name']
        assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
        print 'Compruebo atributo {} y {} en {}'.format(str(phenom_name),str(value),str(contextElement['attributes'][0]))
        assert valueElement == value, 'ERROR: ' + value + ' value expected, ' + valueElement + ' received'
        assert nameElement == phenom_name, 'ERROR: ' + phenom_name + ' name expected, ' + nameElement + ' received'
        assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: metadata: TimeInstant not found in: ' + str(contextElement['attributes'][0]['metadatas'][0])
        assert check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
        print 'Compruebo atributo TimeInstant y {} en {}'.format(str(world.st),str(contextElement['attributes'][1]))
        assert contextElement['attributes'][1]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][1])
        assert check_timestamp(contextElement['attributes'][1]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measure is NOT received"

@step('the measure of asset "([^"]*)" with phenom "([^"]*)", entity_type "([^"]*)", entity_name "([^"]*)" and value "([^"]*)" is received by context broker')
def check_measure_entity_cbroker(step, device, phenom_name, entity_type, entity_name, value):
    time.sleep(1)
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
    #print 'Metadatas {}'.format(metasElement)
    if entity_name:
        device_name = entity_name
    else:
        if entity_type:
            device_name = entity_type + ":" + device
        else:
            device_name = DEF_ENTITY_TYPE + ':' + device
    assert assetElement == "{}".format(device_name), 'ERROR: id: ' + str(device_name) + " not found in: " + str(contextElement)
    print 'ID: ' + str(assetElement)
    typeElement = contextElement['type']
    if entity_type:
        ent_type = entity_type
    else:
        ent_type = DEF_ENTITY_TYPE
    assert typeElement == ent_type, 'ERROR: ' + ent_type + ' type expected, ' + typeElement + ' received'
    print 'TYPE: ' + str(typeElement)
    assert valueElement == value, 'ERROR: ' + value + ' value expected, ' + valueElement + ' received'
    assert nameElement == phenom_name, 'ERROR: ' + phenom_name + ' name expected, ' + nameElement + ' received'
    assert contextElement['attributes'][0]['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0]['metadatas'][0])
    assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
    if world.timestamp:
        assert str(world.st) == contextElement['attributes'][0]['metadatas'][0]['value'], 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
        assert str(world.st) == contextElement['attributes'][1]['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])
    else:
        assert check_timestamp(contextElement['attributes'][0]['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0]['metadatas'][0])
        assert check_timestamp(contextElement['attributes'][1]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][1])

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