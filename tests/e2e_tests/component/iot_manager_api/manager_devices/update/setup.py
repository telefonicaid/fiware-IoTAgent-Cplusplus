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
    world.device_name = dev_name
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
   
@step('I update the attribute "([^"]*)" of device "([^"]*)" with value "([^"]*)"')
def update_device_data(step, attribute, device_name, value):
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if not world.service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(world.service_path)
    else:    
        headers[CBROKER_PATH_HEADER] = '/'
    if world.prot:
        params['protocol']= world.prot
    if (not 'att' in attribute ) & (attribute!='cmd'):
        if attribute=='protocol':
            resource=URLTypes.get(value)
            update_service_data("resource",world.service_name,resource)       
            value = ProtocolTypes.get(value)
            world.prot=value
        json={
              attribute: value
        }
    else:
        d = dict([value.split('#')]) 
        name=str(d.items()[0][0])
        attrs=str(d.items()[0][1])
        if ':' in attrs:
            d2 = dict([attrs.split(':')])
            type1=str(d2.items()[0][0])
            value=str(d2.items()[0][1])       
        if attribute=='attr':
            attributes=[
                {
                 "name": name,
                 "type": type1,
                 "object_id": value
                 }
                ]
            attrs_type="attributes"
        if attribute=='st_att':
            attributes=[
                {
                 "name": name,
                 "type": type1,
                 "value": value
                 }
                ]
            attrs_type="static_attributes"
        if attribute=='cmd':
            attributes=[
                {
                 "name": name,
                 "type": 'command',
                 "value": value
                 }
                ]
            attrs_type="commands"
        json={
              attrs_type: attributes
              }
    world.req =  api2.put_device(device_name, json, headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text
    if attribute == 'device_id':
        world.device_name=value
        world.remember[world.service_name][world.service_path]['device'].remove(device_name)

@step('the device data contains attribute "([^"]*)" with value "([^"]*)"')
def check_device_data(step, attribute, value):
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if world.service_path:
        if not world.service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(world.service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    req =  api.get_device(world.device_name, headers, params)
    response = req.json()
    assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
    if world.service_path == 'void':
        world.service_path = '/'
    assert response['service_path'] == world.service_path, 'Expected Result: ' + world.service_path + '\nObtained Result: ' + response['service_path']
    if attribute == 'entity_name':
        assert response['entity_name'] != world.entity_name, 'NOT Expected Result: ' + world.entity_name + '\nObtained Result: ' + response['entity_name']
        assert response['entity_name'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['entity_name']
    else:
        assert response['entity_name'] == world.entity_name, 'Expected Result: ' + world.entity_name + '\nObtained Result: ' + response['entity_name']
    if attribute == 'entity_type':
        assert response['entity_type'] != world.entity_type, 'NOT Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
        assert response['entity_type'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['entity_type']
    else:
        assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
    if attribute == 'endpoint':
        assert response['endpoint'] != world.endpoint, 'NOT Expected Result: ' + world.endpoint + '\nObtained Result: ' + response['endpoint']
        assert response['endpoint'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['endpoint']
    else:
        assert response['endpoint'] == world.endpoint, 'Expected Result: ' + world.endpoint + '\nObtained Result: ' + response['endpoint']
    if ('att' in attribute ) | (attribute=='cmd'):
        d = dict([value.split('#')]) 
        name=str(d.items()[0][0])
        attrs=str(d.items()[0][1])
        if ':' in attrs:
            d2 = dict([attrs.split(':')])
            type1=str(d2.items()[0][0])
            value=str(d2.items()[0][1])       
    if attribute == 'attr':
        if attribute == world.typ:
            assert response['attributes'][0]['name'] != world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['attributes'][0]['name']
            assert response['attributes'][0]['type'] != world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['attributes'][0]['type']
            assert response['attributes'][0]['object_id'] != world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['attributes'][0]['object_id']
        if world.typ == 'st_attr':
            assert response['static_attributes'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['static_attributes'][0]['name']
            assert response['static_attributes'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['static_attributes'][0]['type']
            assert response['static_attributes'][0]['value'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['static_attributes'][0]['value']
        if world.typ == 'cmd':
            assert response['commands'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['commands'][0]['name']
            assert response['commands'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['commands'][0]['type']
            assert response['commands'][0]['value'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['commands'][0]['value']
        assert response['attributes'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['attributes'][0]['name']
        assert response['attributes'][0]['type'] == type1, 'Expected Result: ' + type1 + '\nObtained Result: ' + response['attributes'][0]['type']
        assert response['attributes'][0]['object_id'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['attributes'][0]['object_id']
    if attribute == 'st_attr':
        if attribute == world.typ:
            assert response['static_attributes'][0]['name'] != world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['static_attributes'][0]['name']
            assert response['static_attributes'][0]['type'] != world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['static_attributes'][0]['type']
            assert response['static_attributes'][0]['value'] != world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['static_attributes'][0]['value']
        if world.typ == 'attr':
            assert response['attributes'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['attributes'][0]['name']
            assert response['attributes'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['attributes'][0]['type']
            assert response['attributes'][0]['object_id'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['attributes'][0]['object_id']
        if world.typ == 'cmd':
            assert response['commands'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['commands'][0]['name']
            assert response['commands'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['commands'][0]['type']
            assert response['commands'][0]['value'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['commands'][0]['value']
        assert response['static_attributes'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['static_attributes'][0]['name']
        assert response['static_attributes'][0]['type'] == type1, 'Expected Result: ' + type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
        assert response['static_attributes'][0]['value'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['static_attributes'][0]['value']
    if attribute == 'cmd':
        if attribute == world.typ:
            assert response['commands'][0]['name'] != world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['commands'][0]['name']
            assert response['commands'][0]['value'] != world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['commands'][0]['value']
        if world.typ == 'attr':
            assert response['attributes'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['attributes'][0]['name']
            assert response['attributes'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['attributes'][0]['type']
            assert response['attributes'][0]['object_id'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['attributes'][0]['object_id']
        if world.typ == 'st_attr':
            assert response['static_attributes'][0]['name'] == world.name, 'Expected Result: ' + world.name + '\nObtained Result: ' + response['static_attributes'][0]['name']
            assert response['static_attributes'][0]['type'] == world.type, 'Expected Result: ' + world.type + '\nObtained Result: ' + response['static_attributes'][0]['type']
            assert response['static_attributes'][0]['value'] == world.value, 'Expected Result: ' + world.value + '\nObtained Result: ' + response['static_attributes'][0]['value']
        assert response['commands'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['commands'][0]['name']
        assert response['commands'][0]['type'] == "command", 'Expected Result: command \nObtained Result: ' + response['commands'][0]['type']
        assert response['commands'][0]['value'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['commands'][0]['value']
    if world.prot:
        assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
        
@step('I try to update the device data of device "([^"]*)" with service "([^"]*)", protocol "([^"]*)" and path "([^"]*)" with the attribute "([^"]*)" and value "([^"]*)"')
def update_wrong_device_data(step, device_name, service_name, protocol, service_path, attribute, value):
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
    if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
    if attribute=='protocol':
        value2 = ProtocolTypes.get(value)
        if value2:
            value=value2
    json={
        attribute: value
        }
    if attribute=='empty_json':
        json = {}
    if protocol:
        prot = ProtocolTypes.get(protocol)
        if not prot:
            prot = protocol
        params['protocol']= prot
    world.req =  api2.put_device(device_name, json, headers, params)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El dispositivo {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_device_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text

def update_service_data(attribute, service_name, value):
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(service_name)
    if world.service_path:
        if not world.service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(world.service_path)
            service_path=world.service_path
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
            service_path='/'
    if world.resource:
        params['resource']= world.resource
    if world.apikey:
        params['apikey']= world.apikey
    json={
          attribute: value
        }
    world.req =  api2.put_service('', json, headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text
    if (attribute == 'resource'):
        print world.remember[service_name][service_path]['resource']
        del world.remember[service_name][service_path]['resource'][world.resource]
        world.remember[service_name][service_path]['resource'].setdefault(value, {})
        world.remember[service_name][service_path]['resource'][value].setdefault(world.apikey)
        print world.remember[service_name][service_path]['resource']            
