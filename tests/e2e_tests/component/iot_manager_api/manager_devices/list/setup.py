from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER,MANAGER_SERVER_ROOT


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
api2 = Rest_Utils_SBC(server_root=MANAGER_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)", path "([^"]*)" and protocol "([^"]*)" created')
def service_precond(step, service_name, service_path, protocol):
    world.service_name = service_name
    world.service_path = service_path
    if (service_name == 'void'):
        return
    resource = URLTypes.get(protocol)
    world.resource = resource
    prot = ProtocolTypes.get(protocol)
    world.prot = prot
    apikey='apikey_' + str(service_name)    
    world.apikey = apikey
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, 'http://myurl:80')

@step('a Device with name "([^"]*)", protocol "([^"]*)", entity_name "([^"]*)" and entity_type "([^"]*)" created')
def device_created_precond(step, dev_name, protocol, entity_name, entity_type):
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.device_name = dev_name
    user_steps.device_of_service_precond(world.service_name, world.service_path, dev_name, {}, {}, entity_name, entity_type, {}, {}, protocol)

@step('I list the devices of "([^"]*)", path "([^"]*)", entity "([^"]*)", protocol "([^"]*)", detailed "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_device_data(step, service_name, service_path, entity, protocol, detailed, limit, offset):
    headers = {}
    params = {}
    world.detailed = detailed
    headers[CBROKER_HEADER] = str(service_name)
    if service_path:
        if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    if detailed:
        params['detailed']= detailed
    if entity:
        params['entity']= entity
    if protocol:
        prot = ProtocolTypes.get(protocol)
        params['protocol']= prot
    if limit:
        params['limit']= limit
    if offset:
        params['offset']= offset
    world.req =  api2.get_listDevices(headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I receive the device data of "([^"]*)" devices with data "([^"]*)"')
def check_device_data(step, num_devices, data):
    entity_name={}
    entity_type={}
    res = world.req.json()
    assert len(res['devices']) == int(num_devices), 'Error: ' + str(num_devices) + ' devices expected, ' + str(len(res['devices'])) + ' received'
    if len(res['devices']) == 1:
        response = res['devices'][0]
        if world.detailed == 'on':
            print response
            assert response['entity_name'] == world.entity_name, 'Expected Result: ' + world.entity_name + '\nObtained Result: ' + response['entity_name']
            assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
            assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
            assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
            if world.service_path == 'void':
                srv_path = '/'
            else:
                srv_path=world.service_path
            assert response['service_path'] == srv_path, 'Expected Result: ' + srv_path + '\nObtained Result: ' + response['service_path']
        assert response['device_id'] == world.device_name, 'Expected Result: ' + world.device_name + '\nObtained Result: ' + response['device_id']
    if len(res['devices']) > 1:
        for i in data.split('/'):
            if '#' in i:
                d = dict([i.split('#')]) 
                device_name=str(d.items()[0][0])
                attrs=str(d.items()[0][1])
                if ':' in attrs:
                    d2 = dict([attrs.split(':')])
                    entity_name=str(d2.items()[0][0])
                    entity_type=str(d2.items()[0][1])
            else:
                device_name=i
            dev_matches=False
            for dev in res['devices']:
                if str(device_name) == dev['device_id']:
                    print 'Compruebo device {} en {}'.format(device_name,dev)
                    assert dev['device_id'] == str(device_name), 'ERROR: device_name: ' + str(device_name) + " not found in: " + str(dev)
                    if entity_name:
                        assert dev['entity_name'] == str(entity_name), 'ERROR: entity_name: ' + str(entity_name) + " not found in: " + str(dev)
                    if entity_type:
                        assert dev['entity_type'] == str(entity_type), 'ERROR: entity_type: ' + str(entity_type) + " not found in: " + str(dev)
                    dev_matches=True
                    break
            assert dev_matches, 'ERROR: device: ' + str(device_name) + " not found in: " + str(res['devices'])

@step('I try to retrieve the devices data of "([^"]*)", path "([^"]*)", entity "([^"]*)", protocol "([^"]*)", detailed "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_wrong_device_data(step,service_name, service_path, entity, protocol, detailed, limit, offset):
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
    if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
    if entity:
        params['entity']= entity
    if protocol:
        prot = ProtocolTypes.get(protocol)
        params['protocol']= prot
    if detailed:
        params['detailed']= detailed
    if limit:
        params['limit']= limit
    if offset:
        params['offset']= offset
    world.req =  api2.get_listDevices(headers, params)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El servicio {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    