from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)" and path "([^"]*)" created')
def service_created_precond(step, service_name, service_path):
    world.service_name = service_name
    world.service_path = service_path
    if (service_name == 'void'):
        return
    resource = '/iot/d'
    apikey='apikey_' + str(service_name)
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, 'http://myurl:80')

@step('a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)", endpoint "([^"]*)" and atribute or command "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def device_created_precond(step, dev_name, entity_name, entity_type, endpoint, typ, name, type1, value):
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
    user_steps.device_of_service_precond(world.service_name, world.service_path, dev_name, endpoint, commands, entity_name, entity_type, attributes, st_attributes)
   
@step('I update the attribute "([^"]*)" of device "([^"]*)" with value "([^"]*)"')
def update_device_data(step, attribute, device_name, value):
    headers = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if not world.service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(world.service_path)
    else:    
        headers[CBROKER_PATH_HEADER] = '/'
    if (not 'att' in attribute ) & (attribute!='cmd'):
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
    world.req =  api.put_device(device_name, json, headers)
    assert world.req.ok, 'ERROR: ' + world.req.text

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
        
@step('I try to update the device data of device "([^"]*)" with service "([^"]*)" and path "([^"]*)" with the attribute "([^"]*)" and value "([^"]*)"')
def update_wrong_device_data(step, device_name, service_name, service_path, attribute, value):
    headers = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
    if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
    json={
        attribute: value
        }
    if attribute=='empty_json':
        json = {}
    world.req =  api.put_device(device_name, json, headers)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El dispositivo {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
