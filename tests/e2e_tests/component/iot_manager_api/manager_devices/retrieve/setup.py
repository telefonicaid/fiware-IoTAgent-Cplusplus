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

@step('a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)", endpoint "([^"]*)", protocol "([^"]*)" and atribute or command "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def device_created_precond(step, dev_name, entity_name, entity_type, endpoint, protocol, typ, name, type1, value):
    commands=[]
    attributes=[]
    st_attributes=[]
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.endpoint = endpoint
    world.typ = typ
    world.name = name
    world.type = type1
    world.value = value
    if typ=='attr':
        attributes=[
             {
              "name": name,
              "type": type1,
              "object_id": value
              }
             ]
    if typ=='st_att':
        st_attributes=[
             {
              "name": name,
              "type": type1,
              "value": value
              }
             ]
    if typ=='cmd':
        commands=[
             {
              "name": name,
              "type": 'command',
              "value": value
              }
             ]
        world.type = 'command'   
    user_steps.device_of_service_precond(world.service_name, world.service_path, dev_name, endpoint, commands, entity_name, entity_type, attributes, st_attributes, protocol)

@step('I retrieve the device data of "([^"]*)" with protocol "([^"]*)"')
def get_device_data(step, device_name, protocol):
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if world.service_path:
        if not world.service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(world.service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    if protocol:
        prot = ProtocolTypes.get(protocol)
        if not prot:
            prot = protocol
        params['protocol']= prot
    world.req =  api2.get_device(device_name, headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I receive the device data of "([^"]*)"')
def check_device_data(step, dev_name):
    res = world.req.json()
    assert len(res['devices']) == 1, 'Error: 1 devices expected, ' + str(len(res['devices'])) + ' received'
    response = res['devices'][0]
    assert response['device_id'] == dev_name, 'Expected Result: ' + dev_name + '\nObtained Result: ' + response['device_id']
    assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
    if world.service_path:
        if world.service_path == 'void':
            assert response['service_path'] == '/', 'Expected Result: ' + '/' + '\nObtained Result: ' + response['service_path']
        else:
            assert response['service_path'] == world.service_path, 'Expected Result: ' + world.srv_path + '\nObtained Result: ' + response['service_path']
    if world.entity_name:
        assert response['entity_name'] == world.entity_name, 'Expected Result: ' + world.entity_name + '\nObtained Result: ' + response['entity_name']
    if world.entity_type:
        assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
    if world.endpoint:
        assert response['endpoint'] == world.endpoint, 'Expected Result: ' + world.endpoint + '\nObtained Result: ' + response['endpoint']
    if world.typ:
        if world.typ == 'attr':
            assert response['attributes'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['attributes'][0]['name']
            assert response['attributes'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['attributes'][0]['type']
            assert response['attributes'][0]['object_id'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['attributes'][0]['object_id']
        if world.typ == 'st_attr':
            assert response['static_attributes'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['static_attributes'][0]['name']
            assert response['static_attributes'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['static_attributes'][0]['type']
            assert response['static_attributes'][0]['value'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['static_attributes'][0]['value']
        if world.typ == 'cmd':
            assert response['commands'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['commands'][0]['name']
            assert response['commands'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['commands'][0]['type']
            assert response['commands'][0]['value'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['commands'][0]['value']
    if world.prot:
        assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
            
@step('I try to retrieve the device data of device "([^"]*)" with service "([^"]*)", protocol "([^"]*)" and path "([^"]*)"')
def get_wrong_device_data(step, device_name, service_name, protocol, service_path):
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
    if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
    if protocol:
        prot = ProtocolTypes.get(protocol)
        if not prot:
            prot = protocol
        params['protocol']= prot
    world.req =  api2.get_device(device_name, headers, params)
#    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El dispositivo {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    if http_status=='200':
        res = world.req.json()
        assert len(res['devices']) == 0, 'Error: 0 devices expected, ' + str(len(res['devices'])) + ' received'
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    