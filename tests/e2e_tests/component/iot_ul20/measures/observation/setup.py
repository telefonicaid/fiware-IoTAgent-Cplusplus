from lettuce import step, world
from common.steps import service_created_precond,check_measure_cbroker,check_measures_cbroker,check_NOT_measure_cbroker
from iotqautils.gtwMeasures import Gw_Measures_Utils
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,IOT_SERVER_ROOT,UL20_APIKEY,DEF_ENTITY_TYPE,DEF_TYPE
from common.user_steps import UserSteps
import requests
import time,datetime

gw = Gw_Measures_Utils(server_root=IOT_SERVER_ROOT)
user_steps = UserSteps()

@step('a service with name "([^"]*)", protocol "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)" created')
def service_with_attritubes_created_precond(step, service_name, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.service_name = service_name
    attributes=[]
    st_attributes=[]
    world.typ1 = typ1
    world.typ2 = typ2
    world.name1 = name1
    world.name2 = name2
    world.type1 = type1
    world.type2 = type2
    world.value1 = value1
    world.value2 = value2
    if typ1=='srv_attr':
        attributes=[
             {
              "name": name1,
              "type": type1,
              "object_id": value1
              }
             ]
    if typ2=='srv_attr':
        attribute={
              "name": name2,
              "type": type2,
              "object_id": value2
              }
        attributes.append(attribute)
    if typ1=='srv_st_att':
        st_attributes=[
             {
              "name": name1,
              "type": type1,
              "value": value1
              }
             ]
    if typ2=='srv_st_att':
        st_attribute={
              "name": name2,
              "type": type2,
              "value": value2
              }
        st_attributes.append(st_attribute)
    user_steps.service_precond(service_name, protocol, attributes, st_attributes)

@step('a device with device id "([^"]*)", protocol "([^"]*)", entity type "([^"]*)" and entity name "([^"]*)" created')
def device_with_entity_values_created_precond(step, device_id, protocol, ent_type, ent_name):
    user_steps.device_precond(device_id, {}, protocol, {}, ent_name, ent_type)
    world.device_id=device_id

@step('a device with device id "([^"]*)", protocol "([^"]*)", atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)" created')
def device_with_attributes_created_precond(step, device_id, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    attributes=[]
    st_attributes=[]
    world.typ1 = typ1
    world.typ2 = typ2
    world.name1 = name1
    world.name2 = name2
    world.type1 = type1
    world.type2 = type2
    world.value1 = value1
    world.value2 = value2
    if typ1=='dev_attr':
        attributes=[
             {
              "name": name1,
              "type": type1,
              "object_id": value1
              }
             ]
    if typ2=='dev_attr':
        attribute={
              "name": name2,
              "type": type2,
              "object_id": value2
              }
        attributes.append(attribute)
    if typ1=='dev_st_att':
        st_attributes=[
             {
              "name": name1,
              "type": type1,
              "value": value1
              }
             ]
    if typ2=='dev_st_att':
        st_attribute={
              "name": name2,
              "type": type2,
              "value": value2
              }
        st_attributes.append(st_attribute)
    user_steps.device_precond(device_id, {}, protocol, {}, {}, {}, attributes, st_attributes)
    world.device_id=device_id

@step('I send a measure to the GW with apikey, id "([^"]*)", protocol "([^"]*)", alias "([^"]*)", timestamp "([^"]*)" and value "([^"]*)"')
def send_measure(step, device_id, protocol, alias, timestamp, value):
    measures= []
    measure = {}
    measure['alias'] = alias
    measure['value'] = value
    world.timestamp=False
    world.def_entity=True
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
    world.def_entity=True
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
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = UL20_APIKEY
    req = gw.sendMeasure(protocol,apikey,device_id,measures)
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
    world.def_entity=True    
    if world.service_name:
        apikey = 'apikey_'+world.service_name
    else:
        apikey = UL20_APIKEY
    if field=="not_apikey":
        req = gw.sendMeasure("/iot/d?i={}","",device_id,measures)
    elif field=="not_device":
        req = gw.sendMeasure("/iot/d?k={}",apikey,"",measures)        
    elif field=="wrong_apikey":
        req = gw.sendMeasure(protocol,field,device_id,measures)
    else:
        req = gw.sendMeasure(protocol,apikey,device_id,measures)
    #print req.status_code + req.ok
    if field:
        if field=="timestamp":
            assert req.ok, 'ERROR: ' + str(req)
        else:
            assert not req.ok, 'ERROR: ' + str(req)
    else:
        assert req.ok, 'ERROR: ' + str(req)

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

@step('the measure of asset "([^"]*)" with phenom "([^"]*)" and value "([^"]*)" and attributes are received by context broker')
def check_measure_with_attributes_cbroker(step, asset, phenom_name, value):
    time.sleep(1)
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Dispositivo {}'.format(assetElement)
    attrs=0
    asset_name = DEF_ENTITY_TYPE + ':' + asset
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    typeElement = contextElement['type']
    assert typeElement == DEF_ENTITY_TYPE, 'ERROR: ' + DEF_ENTITY_TYPE + ' type expected, ' + typeElement + ' received'
    if ('attr' in world.typ1) & (phenom_name==world.value1):
        check_attribute(contextElement, world.name1, world.type1, value)
    elif ('attr' in world.typ2) & (phenom_name==world.value2):
        check_attribute(contextElement, world.name2, world.type2, value)
    else:
        check_attribute(contextElement, phenom_name, DEF_TYPE, value)
    attrs+=1
    if 'st_att' in world.typ1:
        check_attribute(contextElement, world.name1, world.type1, world.value1)
        attrs+=1
    if 'st_att' in world.typ2:
        check_attribute(contextElement, world.name2, world.type2, world.value2)
        attrs+=1
    nameTime = contextElement['attributes'][attrs]['name']
    assert nameTime == "TimeInstant", 'ERROR: ' + "TimeInstant" + ' name expected, ' + nameTime + ' received'
    assert check_timestamp(contextElement['attributes'][attrs]['value']), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][attrs])

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
def check_attribute (contextElement, name, typ, value):
    attr_matches=False
    for attr in contextElement['attributes']:
        if str(name) == attr['name']:
            print 'Compruebo atributo {} en {}'.format(name,attr)
            assert attr['type'] == str(typ), 'ERROR: type: ' + str(typ) + " not found in: " + str(attr)
            assert attr['value'] == str(value), 'ERROR: value: ' + str(value) + " not found in: " + str(attr)
            assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: TimeInstant metadata not found in: ' + str(attr)
            assert check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
            attr_matches=True
            break
    assert attr_matches, 'ERROR: attribute: ' + str(name) + " not found in: " + str(contextElement['attributes'])
