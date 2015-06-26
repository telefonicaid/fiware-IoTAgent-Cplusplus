from lettuce import step, world
from iotqautils.iota_utils import Rest_Utils_IoTA
from common.functions import Functions, URLTypes
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,CBROKER_PATH_HEADER,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,MANAGER_SERVER_ROOT,SMPP_URL,SMPP_FROM
import time, requests

iotagent = Rest_Utils_IoTA(server_root=IOT_SERVER_ROOT+'/iot')
functions = Functions()
iota_manager = Rest_Utils_IoTA(server_root=MANAGER_SERVER_ROOT+'/iot')

# IoTA STEPS

@step('a service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
#        world.service_name = service_name
        world.protocol = protocol
        functions.service_precond(service_name, protocol)

@step('a service with name "([^"]*)", protocol "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)" created')
def service_with_attributes_created_precond(step, service_name, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.service_name = service_name
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ1, typ2, name1, name2, type1, type2, value1, value2)
    functions.service_precond(service_name, protocol, world.attributes, world.st_attributes)

@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)" and apikey "([^"]*)" not created')
def service_not_created_precond(step, service_name, service_path, resource, apikey):
    functions.not_service_precond(service_name, service_path, resource, apikey)

@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)" and apikey "([^"]*)" not created')
def service_not_created_manager_precond(step, service_name, service_path, protocol, apikey):
    resource = URLTypes.get(protocol)
    if not resource:
        print "No hay que buscar servicio"
        return
    functions.not_service_precond(service_name, service_path, resource, apikey)

@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)" and apikey "([^"]*)" created')
def service_with_params_precond(step, service_name, service_path, resource, apikey):
    world.service_name = service_name
    world.srv_path = service_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker= 'http://myurl:80'
    world.entity_type = {}
    world.token = {}
    world.typ1 = {}
    world.typ2 = {}
    functions.service_with_params_precond(service_name, service_path, resource, apikey, world.cbroker)

@step('I create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)"')
def create_service(step,srv_name,srv_path,resource,apikey,cbroker,entity_type,token):
    world.typ1 = {}
    world.typ2 = {}
    world.srv_path = srv_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    service=functions.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,entity_type,token)
    assert service.status_code == 201, 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('I create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)"')
def create_service_manager(step,srv_name,srv_path,protocol,apikey,cbroker,entity_type,token):
    world.typ1 = {}
    world.typ2 = {}
    world.srv_path = srv_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    service=functions.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,entity_type,token,{},{},protocol)
    assert service.status_code == 201, 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('I create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_service_with_attrs(step,srv_name,srv_path,resource,apikey,cbroker,typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.srv_path = srv_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ1, typ2, name1, name2, type1, type2, value1, value2)
    service=functions.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,{},{},world.attributes,world.st_attributes)
    assert service.status_code == 201, 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('I create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_service_with_attrs_manager(step, srv_name, srv_path, protocol, apikey, cbroker, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.srv_path = srv_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    world.attributes=[]
    world.st_attributes=[]
    functions.fill_attributes(typ1, typ2, name1, name2, type1, type2, value1, value2)
    service=functions.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,{},{},world.attributes,world.st_attributes,protocol)
    assert service.status_code == 201, 'ERROR: ' + service.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('the Service with name "([^"]*)" and path "([^"]*)" is created')
def service_created(step, service_name, service_path):
    functions.get_service_created(service_name, service_path, world.resource)
    functions.check_service_created(1)

@step('I retrieve the service data of "([^"]*)", path "([^"]*)", resource "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_service_data(step,service_name, service_path, resource, limit, offset):
    world.req = functions.get_service_created(service_name, service_path, resource, limit, offset)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I receive the service data of "([^"]*)" services')
def check_service_data(step, num_services):
    functions.check_service_created(num_services)

@step('a device with device name "([^"]*)" and protocol "([^"]*)" created')    
def device_created_precond(step, device_name, protocol):
    functions.device_precond(device_name, {}, protocol)

@step('a device with device id "([^"]*)", protocol "([^"]*)", entity type "([^"]*)" and entity name "([^"]*)" created')
def device_with_entity_values_created_precond(step, device_id, protocol, ent_type, ent_name):
    functions.device_precond(device_id, {}, protocol, {}, ent_name, ent_type)
#   world.device_id=device_id

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
    functions.device_precond(device_id, {}, protocol, {}, {}, {}, attributes, st_attributes)
    world.device_id=device_id

@step('a device with device id "([^"]*)", device name "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_commands_created_precond(step, device_id, device_name, protocol, cmd_name, cmd_value):
    functions.device_with_commands_precond(device_id, device_name, protocol, cmd_name, cmd_value, {}, {})

@step('a device with device id "([^"]*)", device name "([^"]*)", endpoint "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_endpoint_created_precond(step, device_id, device_name, endpoint, protocol, cmd_name, cmd_value):
    functions.device_with_commands_precond(device_id, device_name, protocol, cmd_name, cmd_value, endpoint, {})

@step('a device with device id "([^"]*)", entity type "([^"]*)", entity name "([^"]*)", protocol "([^"]*)", command name "([^"]*)" and command value "([^"]*)" created')
def device_with_cmds_entity_values_created_precond(step, device_id, ent_type, ent_name, protocol, cmd_name, cmd_value):
    functions.device_with_commands_precond(device_id, ent_name, protocol, cmd_name, cmd_value, {}, ent_type)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" is received by context broker')
def check_measure_cbroker(step, asset_name, measures):
    functions.check_measure(asset_name, measures)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" and timestamp "([^"]*)" is received by context broker')
def check_measure_cbroker_timestamp(step, asset_name, measures, timestamp):
    functions.check_measure(asset_name, measures, timestamp)

@step('the measure of asset "([^"]*)" with entity_type "([^"]*)", entity_name "([^"]*)" and measures "([^"]*)" is received by context broker')
def check_measure_cbroker_entity(step, asset_name, entity_type, entity_name, measures):
    functions.check_measure(asset_name, measures, {}, entity_type, entity_name)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" and attributes are received by context broker')
def check_measure_cbroker_with_attributes(step, asset_name, measures):
    functions.check_measure(asset_name, measures, {}, {}, {}, True)

@step('"([^"]*)" measures of asset "([^"]*)" are received by context broker')
def check_measures_cbroker(step, num_measures, asset_name):
    check_measures(step, num_measures, asset_name)

@step('"([^"]*)" measures of asset "([^"]*)" with timestamp "([^"]*)" are received by context broker')
def check_measures_cbroker_timestamp(step, num_measures, asset_name, timestamp):
    check_measures(step, num_measures, asset_name, timestamp)

def check_measures(step, num_measures, asset_name, timestamp={}):
    time.sleep(1)
    measures_count =  requests.get(CBROKER_URL+"/countMeasure")
    assert measures_count.text == str(num_measures), 'ERROR: ' + str(num_measures) + ' measures expected, ' + measures_count.text + ' received'
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    for measures_dict in step.hashes:
        measures = measures_dict['generated_measures']
        print measures
        for i in measures.split('#'):
            if i:
                d = dict([i.split(':')])
            else:
                break 
            measure_name=str(d.items()[0][0])
            measure_value=str(d.items()[0][1])
            metadata_value=""
            if  "/" in measure_value:
                if not measure_name=='l':
                    d2 = dict([measure_value.split('/')])
                    measure_value=str(d2.items()[0][0])
                    metadata_value=str(d2.items()[0][1])
            attr_matches=False
            for attr in contextElement['attributes']:
                if str(measure_name) in attr['name']:
                    if str(attr['value']) == str(measure_value):
                        print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                        attr_matches=True
                        if metadata_value:
                            assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                            assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                        assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                        if timestamp:
                            assert str(timestamp) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(timestamp) + " not found in: " + str(attr['metadatas'][0])
                        else:
                            assert functions.check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                        break
    assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
    is_timestamp=False
    for attr in contextElement['attributes']:
        if attr ['name'] == "TimeInstant":
            print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
            if timestamp:
                assert str(timestamp) == attr['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
            else:
                assert functions.check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
            is_timestamp=True
            break
    assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
    if world.def_entity:
        asset_name = DEF_ENTITY_TYPE + ':' + asset_name
        world.thing = DEF_ENTITY_TYPE
    assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
    assert typeElement == "{}".format(world.thing), 'ERROR: type: ' + str(world.thing) + " not found in: " + str(contextElement)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" is received or NOT by context broker')
def check_NOT_measure_cbroker(step, asset_name, measures):
    time.sleep(1)
    print measures
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    typeElement = contextElement['type']
    if (world.field == "timestamp") | (world.field == "sens_type") | (world.field == "payload"):
        if world.def_entity:
            asset_name = DEF_ENTITY_TYPE + ':' + asset_name
            world.thing = DEF_ENTITY_TYPE
        assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
        assert typeElement == "{}".format(world.thing), 'ERROR: type: ' + str(world.thing) + " not found in: " + str(contextElement)
        if measures:
            for i in measures.split('#'):
                d = dict([i.split(':')]) 
                measure_name=str(d.items()[0][0])
                measure_value=str(d.items()[0][1])
                metadata_value=""
                if  "/" in measure_value:
                    d2 = dict([measure_value.split('/')])
                    measure_value=str(d2.items()[0][0])
                    metadata_value=str(d2.items()[0][1])
                attr_matches=False
                for attr in contextElement['attributes']:
                    if str(measure_name) in attr['name']:
                        print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                        attr_matches=True
                        assert attr['value'] == str(measure_value), 'ERROR: value: ' + str(measure_value) + " not found in: " + str(attr)
                        if metadata_value:
                            assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                            assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                        assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                        if (world.field == "timestamp"):
                            assert functions.check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])   
                        else:
                            assert str(world.st) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                        break
                assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
        if world.field == "payload":
            assert len(contextElement['attributes']) == 1, 'ERROR: Atribute ' + str(contextElement['attributes'][1]) + " found in: " + str(contextElement['attributes'])
        is_timestamp=False
        for attr in contextElement['attributes']:
            if attr ['name'] == "TimeInstant":
                print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
                if (world.field == "timestamp") | (world.field == "payload"):
                    assert functions.check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                else:
                    assert str(world.st) == attr['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
                is_timestamp=True
                break
        assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measure is NOT received"

@step('"([^"]*)" measures of asset "([^"]*)" are received or NOT by context broker')
def check_NOT_measures_cbroker(step, num_measures, asset_name):
    check_NOT_measures(step, num_measures, asset_name)
    
@step('([^"]*)" measures of asset "([^"]*)" with timestamp "([^"]*)" are received or NOT by context broker')
def check_NOT_measures_cbroker_timestamp(step, num_measures, asset_name, timestamp):
    check_NOT_measures(step, num_measures, asset_name, timestamp)

def check_NOT_measures(step, num_measures, asset_name, timestamp={}):
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
    typeElement = contextElement['type']
    if int(num_measures)>0:
        measures_dict=step.hashes[0]
        measures = measures_dict['generated_measures']
        print measures
        for i in measures.split('#'):
            #print i
            d = dict([i.split(':')]) 
            measure_name=str(d.items()[0][0])
            measure_value=str(d.items()[0][1])
            metadata_value=""
            if  "/" in measure_value:
                d2 = dict([measure_value.split('/')])
                measure_value=str(d2.items()[0][0])
                metadata_value=str(d2.items()[0][1])
            attr_matches=False
            for attr in contextElement['attributes']:
                if str(measure_name) in attr['name']:
                    print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                    if str(attr['value']) == str(measure_value):
                        attr_matches=True
                        if metadata_value:
                            assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                            assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                        assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                        if not timestamp:
                            timestamp=world.st
                        assert str(timestamp) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                        break
            assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
        if len(step.hashes)==2:
            measures_dict=step.hashes[1]
            measures = measures_dict['generated_measures']
            print measures
            for i in measures.split('#'):
                d = dict([i.split(':')]) 
                measure_name=str(d.items()[0][0])
                measure_value=str(d.items()[0][1])
                metadata_value=""
                if  "/" in measure_value:
                    d2 = dict([measure_value.split('/')])
                    measure_value=str(d2.items()[0][0])
                    metadata_value=str(d2.items()[0][1])
                attr_matches=True
                attr_exists=False
                for attr in contextElement['attributes']:
                    if str(measure_name) in attr['name']:
                        attr_exists=True
                        print 'Compruebo no existencia del atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                        if str(attr['value']) != str(measure_value):
                            attr_matches=False
                            break
                if not attr_exists:
                    attr_matches=False
                assert not attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " found in: " + str(contextElement['attributes'])
        print 'Compruebo atributo TimeInstant y {} en {}'.format(contextElement['attributes'][0]['value'],str(contextElement['attributes'][0]))
        assert contextElement['attributes'][0]['name'] == "TimeInstant", 'ERROR: ' + str(contextElement['attributes'][0])
        if not timestamp:
            timestamp=world.st
        assert str(timestamp) == contextElement['attributes'][0]['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(contextElement['attributes'][0])
        assert assetElement == "{}".format(asset_name), 'ERROR: id: ' + str(asset_name) + " not found in: " + str(contextElement)
        assert typeElement == "{}".format(world.thing), 'ERROR: type: ' + str(world.thing) + " not found in: " + str(contextElement)
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measures are NOT received"
                
@step('the command of device "([^"]*)" with response "([^"]*)" and status "([^"]*)" is received by context broker')
def check_status_info(step, asset_name, response, status):
    world.response={}
    if status=='OK':
        functions.check_command_cbroker(asset_name, status, response)
    else:
        if response:
            world.response=status
            functions.check_command_cbroker(asset_name, status)
            functions.check_NOT_command_cbroker(asset_name, response, "Info")

@step('the command of device "([^"]*)" with response "([^"]*)" and status "([^"]*)" is received or NOT by context broker')
def check_wrong_status_info(step, asset_name, response, status):
    world.response={}
    if status != "fail":
        functions.check_command_cbroker(asset_name, status, response)
    else:
        functions.check_NOT_command_cbroker(asset_name, response, "Status")
        functions.check_NOT_command_cbroker(asset_name, response, "Info")
    
@step('the command of device "([^"]*)" with response "([^"]*)", entity_type "([^"]*)" and status "([^"]*)" is received by context broker')
def check_status_entity_info(step, asset, response, entity_type, status):
    if asset:
        asset_name = asset
    else:
        if entity_type:
            asset_name = entity_type + ":" + world.device_id
        else:
            asset_name = DEF_ENTITY_TYPE + ":" + world.device_id
    if entity_type:
        functions.check_command_cbroker(asset_name, status, response, entity_type)
    else:
        functions.check_command_cbroker(asset_name, status, response)
    
@step('a SMS to telephone "([^"]*)" is received by SMPP Host')
def check_sms(step, tel_number):
    time.sleep(1)
    req =  requests.get(SMPP_URL+"/simulaClient/smpp1")
    response = req.json()
    assert response['from'] == "tel:"+SMPP_FROM, 'ERROR: telephone number: ' + SMPP_FROM + " not found in: " + str(response['from'])
    assert response['to'][0] == "tel:"+tel_number, 'ERROR: telephone number: ' + tel_number + " not found in: " + str(response['to'][0])
    assert response['message'] == world.code+"AK"+world.msg_id, 'ERROR: message ' + world.code+"AK"+world.msg_id + " not found in: " + str(response['message'])

@step('a SMS to telephone "([^"]*)" is received or NOT by SMPP Host')
def check_NOT_sms(step, tel_number):
    time.sleep(1)
    req =  requests.get(SMPP_URL+"/simulaClient/smpp1")
    response = req.json()
    if ((world.field == "timestamp" ) | world.are_measures) & (not world.multidata):
        assert response['from'] == "tel:"+SMPP_FROM, 'ERROR: telephone number: ' + SMPP_FROM + " not found in: " + str(response['from'])
        assert response['to'][0] == "tel:"+tel_number, 'ERROR: telephone number: ' + tel_number + " not found in: " + str(response['to'][0])
        assert response['message'] == world.code+"AK"+world.msg_id, 'ERROR: message ' + world.code+"AK"+world.msg_id + " not found in: " + str(response['message'])
        print "SMS is received"
    else:
        assert response['to'] != "tel:"+tel_number, 'ERROR: telephone number: ' + tel_number + " found in: " + str(response['to'])
        print "SMS is NOT received"    