from lettuce import step, world
from iotqautils.iota_utils import Rest_Utils_IoTA
from common.functions import Functions
from common.gw_configuration import CBROKER_URL,CBROKER_HEADER,CBROKER_PATH_HEADER,IOT_SERVER_ROOT,DEF_ENTITY_TYPE,MANAGER_SERVER_ROOT,SMPP_URL,SMPP_FROM
from lettuce import world
import time, datetime, requests

iotagent = Rest_Utils_IoTA(server_root=IOT_SERVER_ROOT+'/iot')
functions = Functions()
iota_manager = Rest_Utils_IoTA(server_root=MANAGER_SERVER_ROOT+'/iot')

# IoTA STEPS

@step('a service with name "([^"]*)" and protocol "([^"]*)" created')
def service_created_precond(step, service_name, protocol):
    if protocol:
        world.service_name = service_name
        functions.service_precond(service_name, protocol)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" is received by context broker')
def check_measure_cbroker(step, asset_name, measures):
    check_measure(asset_name, measures)

@step('the measure of asset "([^"]*)" with measures "([^"]*)" and timestamp "([^"]*)" is received by context broker')
def check_measure_cbroker_timestamp(step, asset_name, measures, timestamp):
    check_measure(asset_name, measures, timestamp)

def check_measure(asset_name, measures, timestamp={}):
    time.sleep(1)
    print measures
    req =  requests.get(CBROKER_URL+"/last")
    response = req.json()
    assert req.headers[CBROKER_HEADER] == world.service_name, 'ERROR de Cabecera: ' + world.service_name + ' esperada ' + str(req.headers[CBROKER_HEADER]) + ' recibida'
    print 'Compruebo la cabecera {} con valor {}'.format(CBROKER_HEADER,req.headers[CBROKER_HEADER])
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Dispositivo {}'.format(assetElement)
    typeElement = contextElement['type']
    #print 'Dispositivo {}'.format(typeElement)
    for i in measures.split('#'):
            d = dict([i.split(':')]) 
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
                if str(measure_name) == attr['name']:
                    print 'Compruebo atributo {} y {} en {}'.format(measure_name,measure_value,attr)
                    assert attr['value'] == str(measure_value), 'ERROR: value: ' + str(measure_value) + " not found in: " + str(attr)
                    attr_matches=True
                    if metadata_value:
                        assert attr['metadatas'][1]['name'] == "uom", 'ERROR: ' + str(attr['metadatas'][1])
                        assert str(metadata_value) in attr['metadatas'][1]['value'], 'ERROR: metadata: ' + str(metadata_value) + " not found in: " + str(attr['metadatas'][1])
                    assert attr['metadatas'][0]['name'] == "TimeInstant", 'ERROR: ' + str(attr['metadatas'][0])
                    if timestamp:
                        assert str(timestamp) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(timestamp) + " not found in: " + str(attr['metadatas'][0])
                    else:
                        assert check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                    break
            assert attr_matches, 'ERROR: attribute: ' + str(measure_name) + " not found in: " + str(contextElement['attributes'])
    is_timestamp=False
    for attr in contextElement['attributes']:
        if attr ['name'] == "TimeInstant":
            print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
            if timestamp:
                assert str(timestamp) == attr['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
            else:
                assert check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
            is_timestamp=True
            break
    assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
    if world.def_entity:
            asset_name = DEF_ENTITY_TYPE + ':' + asset_name
            world.thing = DEF_ENTITY_TYPE
    assert assetElement == "{}".format(asset_name), 'ERROR: ' + str(contextElement)
    assert typeElement == world.thing, 'ERROR: ' + str(contextElement)

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
    #print 'Ultima medida recibida {}'.format(response)
    contextElement = response['contextElements'][0]
    assetElement = contextElement['id']
    #print 'Dispositivo {}'.format(assetElement)
    typeElement = contextElement['type']
    #print 'Dispositivo {}'.format(typeElement)
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
                            assert check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                        break
    assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
    is_timestamp=False
    for attr in contextElement['attributes']:
        if attr ['name'] == "TimeInstant":
            print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
            if timestamp:
                assert str(timestamp) == attr['value'], 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
            else:
                assert check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
            is_timestamp=True
            break
    assert is_timestamp, 'ERROR: TimeInstant not found in' + str(contextElement['attributes'])
    if world.def_entity:
            asset_name = DEF_ENTITY_TYPE + ':' + asset_name
            world.thing = DEF_ENTITY_TYPE
    assert assetElement == "{}".format(asset_name), 'ERROR: ' + str(contextElement)
    assert typeElement == world.thing, 'ERROR: ' + str(contextElement)

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
    if (world.field == "timestamp") | (world.field == "sens_type"):
        if world.def_entity:
            asset_name = DEF_ENTITY_TYPE + ':' + asset_name
            world.thing = DEF_ENTITY_TYPE
        assert typeElement == world.thing, 'ERROR: ' + str(contextElement)
        assert assetElement == "{}".format(asset_name), 'ERROR: ' + str(contextElement)
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
                        assert check_timestamp(attr['metadatas'][0]['value']), 'ERROR: metadata: ' + str(world.st2) + " not found in: " + str(attr['metadatas'][0])   
                    else:
                        assert str(world.st) == attr['metadatas'][0]['value'], 'ERROR: metadata: ' + str(world.st) + " not found in: " + str(attr['metadatas'][0])
                    break
            assert attr_matches, 'ERROR: attr:' + str(measure_name) + ' value: ' + str(measure_value) + " not found in: " + str(contextElement['attributes'])
        is_timestamp=False
        for attr in contextElement['attributes']:
            if attr ['name'] == "TimeInstant":
                print 'Compruebo atributo TimeInstant y {} en {}'.format(attr['value'],str(attr))
                if world.field == "timestamp":
                    assert check_timestamp(str(attr['value'])), 'ERROR: timestamp: ' + str(world.st) + " not found in: " + str(attr)
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
    assert typeElement == world.thing, 'ERROR: ' + str(contextElement)
#    if world.are_measures:
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
        assert assetElement == "{}".format(asset_name), 'ERROR: ' + str(contextElement)
    else:
        assert assetElement != "{}".format(asset_name), 'ERROR: device: ' + str(asset_name) + " found in: " + str(contextElement)
        print "Measures are NOT received"
                
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

    def service_created(self, service_name, service_path={}, resource={}):
        headers = {}
        params = {}
        headers[CBROKER_HEADER] = str(service_name)
        if service_path:
            if not service_path == 'void':
                headers[CBROKER_PATH_HEADER] = str(service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        if resource:
            params['resource']= resource
        service =  iotagent.get_service('', headers, params)
        if service.status_code == 200:
            serv = service.json()
            if serv['count'] == 1:
                world.service_exists = True
                if resource:
                    world.service_path_exists = True
                return True
            else:
                return False
        else:
            return False
        
    def device_created(self, service_name, device_name, service_path={}):
        headers = {}
        headers[CBROKER_HEADER] = str(service_name)
        if service_path:
            if not service_path=='void':
                headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        device = iotagent.get_device(device_name, headers)
        if device.status_code == 200:
            world.device_exists=True
            return True
        else:
            return False
            
    def create_device(self, service_name, device_name, service_path={}, endpoint={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}, protocol={}, manager={}):
        headers = {}
        if not service_name=='void':
            headers[CBROKER_HEADER] = str(service_name)
        if service_path:
            if not service_path=='void':
                headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        device={
            "devices":[
                {
#                    "device_id": device_name
                }
                ]
               }
        if device_name:
            if device_name=='void':
                device_name=""
            device['devices'][0]['device_id'] = device_name
        if commands:
            device['devices'][0]['commands'] = commands
        if endpoint:
            device['devices'][0]['endpoint'] = endpoint
        if entity_type:
            device['devices'][0]['entity_type'] = entity_type
#        else:
#            device['devices'][0]['entity_type'] = DEF_ENTITY_TYPE
        if entity_name:
            device['devices'][0]['entity_name'] = entity_name
        if attributes:
            device['devices'][0]['attributes'] = attributes
        if static_attributes:
            device['devices'][0]['static_attributes'] = static_attributes
        if protocol:
            if protocol=="void":
                protocol=""
            device['devices'][0]['protocol'] = protocol
        if manager:
            req = iota_manager.post_device(device,headers)
        else:
            req = iotagent.post_device(device,headers)
#        assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(device_name)
        return req
        
    def create_service(self, service_name, protocol, attributes={}, static_attributes={}):
        headers = {}
        headers[CBROKER_HEADER] = service_name
        headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        resource = URLTypes.get(protocol)
        if (protocol == 'IotTT') | (protocol == 'IoTRepsol'):
            apikey=''
        else:
            apikey='apikey_' + str(service_name)
        service={
            "services":[
                {
                    "apikey": apikey,
                    "entity_type": DEF_ENTITY_TYPE,
                    "cbroker": CBROKER_URL,
                    "resource": resource
                }
                ]
                }
        if attributes:
            service['services'][0]['attributes'] = attributes
        if static_attributes:
            service['services'][0]['static_attributes'] = static_attributes
        req = iotagent.post_service(service, headers)
        assert req.status_code == 201, 'ERROR: ' + req.text + "El servicio {} no se ha creado correctamente".format(service_name)
        world.service_exists = True            
        return req

    def create_service_with_params(self, service_name, service_path, resource={}, apikey={}, cbroker={}, entity_type={}, token={}, attributes={}, static_attributes={}, protocol={}):
        world.protocol={}
        headers = {}
        if not service_name == 'void':
            headers[CBROKER_HEADER] = service_name
        if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
        service={
            "services":[
                {
#                    "resource": resource
                }
                ]
                }
        if resource:
            if not resource == 'void':
                if not resource == 'null':
                    service['services'][0]['resource'] = resource
            else:
                service['services'][0]['resource'] = ""
#        if not apikey == 'void':
        if apikey:
            if not apikey == 'null':
                service['services'][0]['apikey'] = apikey
        else:
            service['services'][0]['apikey'] = ""
        if cbroker:
            if not cbroker == 'null':
                service['services'][0]['cbroker'] = cbroker
        else:
            service['services'][0]['cbroker'] = ""
        if entity_type:
            service['services'][0]['entity_type'] = entity_type
        if token:
            service['services'][0]['token'] = token
        if attributes:
            service['services'][0]['attributes'] = attributes
        if static_attributes:
            service['services'][0]['static_attributes'] = static_attributes
        if protocol:
            if not protocol == 'void':
                if not protocol == 'null':
                    resource = URLTypes.get(protocol)
                    prot = ProtocolTypes.get(protocol)
                    if not prot:
                        prot = protocol
                    service['services'][0]['protocol']= [prot]
            else:
                resource = protocol
                service['services'][0]['protocol'] = []
            req = iota_manager.post_service(service, headers)
        else:
            req = iotagent.post_service(service, headers)
        if req.status_code == 201 or req.status_code == 409:
            world.remember.setdefault(service_name, {})
            if service_path == 'void':
                service_path='/'
#            world.remember[service_name].setdefault('path', set())
#            world.remember[service_name]['path'].add(service_path)
#            world.remember[service_name]['path'][service_path].setdefault('resource', set())
#            world.remember[service_name]['path'][service_path]['resource'].add(service_path)
            world.remember[service_name].setdefault(service_path, {})
            world.remember[service_name][service_path].setdefault('resource', {})
            world.remember[service_name][service_path]['resource'].setdefault(resource, {})
            if not apikey:
                apikey = ""
            world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
#            print world.remember
            world.service_exists = True
            world.service_path_exists = True
        return req

    def delete_device(self, device_name, service_name, service_path={}):
        headers = {}
        headers[CBROKER_HEADER] = service_name
        if service_path:
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        req = iotagent.delete_device(device_name,headers)
        assert req.status_code == 204, 'ERROR: ' + req.text + "El device {} no se ha borrado correctamente".format(device_name)
        return req

    def delete_service(self, service_name, service_path={}, resource={}, apikey={}):
        params={}
        headers = {}
        headers[CBROKER_HEADER] = service_name
        if world.protocol:
            resource2 = URLTypes.get(world.protocol)
            if (world.protocol == 'IotTT') | (world.protocol == 'IoTRepsol'):
                apikey=''
            else:
                apikey='apikey_' + str(service_name)
            params = {"resource": resource2,
                      "apikey": apikey}
        if resource:
            if apikey:
                params = {"resource": resource,
                          "apikey": apikey
                          }
            else:
                params = {"resource": resource}
        if service_path:
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:
            headers[CBROKER_PATH_HEADER] = '/path_' + str(service_name)
        print params
        req = iotagent.delete_service('', headers, params)
        assert req.status_code == 204, 'ERROR: ' + req.text + "El servicio {} no se ha borrado correctamente".format(service_name)
        return req

    def service_precond(self, service_name, protocol, attributes={}, static_attributes={}):
        world.service_name = service_name
        if not self.service_created(service_name):
            service = self.create_service(service_name, protocol, attributes, static_attributes)
            assert service.status_code == 201, 'Error al crear el servcio {} '.format(service_name)
            print 'Servicio {} creado '.format(service_name)
        else:
            print 'El servicio {} existe '.format(service_name)
        world.protocol=protocol
        world.remember.setdefault(service_name, {})
        world.service_exists = True

    def service_with_params_precond(self, service_name, service_path, resource, apikey, cbroker={}, entity_type={}, token={}, attributes={}, static_attributes={}):
        world.protocol={}
        world.service_name = service_name
        if not self.service_created(service_name, service_path, resource):
            service = self.create_service_with_params(service_name, service_path, resource, apikey, cbroker, entity_type, token, attributes, static_attributes)
            assert service.status_code == 201, 'Error al crear el servcio {} '.format(service_name)
            print 'Servicio {} creado '.format(service_name)
        else:
            print 'El servicio {} existe '.format(service_name)
            world.remember.setdefault(service_name, {})
            if service_path == 'void':
                service_path='/'
            world.remember[service_name].setdefault(service_path, {})
            world.remember[service_name][service_path].setdefault('resource', {})
            world.remember[service_name][service_path]['resource'].setdefault(resource, {})
            if not apikey:
                apikey = ""
            world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)
            world.service_exists = True
            world.service_path_exists = True

    def device_precond(self, device_id, endpoint={}, protocol={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}):
        world.device_id = device_id
        if not self.device_created(world.service_name, device_id):
            prot = ProtocolTypes.get(protocol)
            device = self.create_device(world.service_name, device_id, {}, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot)
            assert device.status_code == 201, 'Error al crear el device {} '.format(device_id)
            print 'Device {} creado '.format(device_id)
        else:
            print 'El device {} existe '.format(device_id)
        world.remember[world.service_name].setdefault('device', set())
        world.remember[world.service_name]['device'].add(device_id)
        world.device_exists = True
       
    def device_of_service_precond(self, service_name, service_path, device_id, endpoint={}, commands={}, entity_name={}, entity_type={}, attributes={}, static_attributes={}, protocol={}, manager={}):
        world.device_id = device_id
        if not self.device_created(service_name, device_id, service_path):
            prot = ProtocolTypes.get(protocol)
            device = self.create_device(world.service_name, device_id, service_path, endpoint, commands, entity_name, entity_type, attributes, static_attributes, prot, manager)
            assert device.status_code == 201, 'Error al crear el device {} '.format(device_id)
            print 'Device {} con path {} creado '.format(device_id, service_path)
        else:
            print 'El device {} existe '.format(device_id)
        if service_path=='void':
            service_path2='/'
        else:
            service_path2=service_path
        world.remember[service_name][service_path2].setdefault('device', set())
        world.remember[service_name][service_path2]['device'].add(device_id)
        world.device_exists = True

    def clean(self,dirty):
        if world.service_exists:
            for srv in dirty.keys():
                if world.device_exists:
                    if world.service_path_exists:
                        for path in dirty[srv]:
                            if dirty[srv][path].__contains__('device'):
                                for device in dirty[srv][path]['device']:
                                    req_device = self.delete_device(device,srv,path)
                                    if req_device.status_code == 204:
                                        print 'Se ha borrado el device:{} del servicio:{} y path:{}'.format(device,srv,path) 
                                    else:
                                        print 'No se ha podido borrar el device:{} del servicio:{} y path:{}'.format(device,srv,path)
                    else:
                        if dirty[srv].__contains__('device'):
                            for device in dirty[srv]['device']:
                                req_device = self.delete_device(device,srv)
                                if req_device.status_code == 204:
                                    print 'Se ha borrado el device ' + str(device) + ' del servicio ' + str(srv)
                                else:
                                    print 'No se ha podido borrar el device ' + str(device) + ' del servicio ' + str(srv)
                if world.service_path_exists:
                    for path in dirty[srv]:
                        if dirty[srv][path].__contains__('resource'):
                            for resource in dirty[srv][path]['resource']:
                                for apikey in dirty[srv][path]['resource'][resource]:
                                    req_service = self.delete_service(srv, path, resource, apikey)
                                    if req_service.status_code == 204:
                                        print 'Se ha borrado el servicio:{} path:{} resource:{} y apikey:{}'.format(srv,path,resource,apikey)
                                    else:
                                        print 'No se ha podido borrar el servicio:{} path:{} resource:{} y apikey:{}'.format(srv,path,resource,apikey)
                else:
                    req_service = self.delete_service(srv)
                    if req_service.status_code == 204:
                        print 'Se ha borrado el servicio ' + srv
                    else:
                        print 'No se ha podido borrar el servicio ' + srv