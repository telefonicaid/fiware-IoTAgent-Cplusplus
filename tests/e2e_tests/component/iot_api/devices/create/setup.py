from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
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

@step('a Device with name "([^"]*)" and path "([^"]*)" not created')
def device_not_created(step, device_name, service_path):
    world.service_path = service_path
    if (world.service_name == 'void'):
        if  (not '/' in service_path) and (not service_path=='void'):
            return
    if user_steps.device_created(world.service_name, device_name, service_path):
        print 'ERROR: El device {} ya existe'.format(device_name)
        if service_path=='void':
            service_path2='/'
        else:
            service_path2=service_path
        world.remember[world.service_name][service_path2].setdefault('device', set())
        world.remember[world.service_name][service_path2]['device'].add(device_name)
        world.device_exists = True

@step('I create a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)", endpoint "([^"]*)" and protocol "([^"]*)"')
def create_device(step, dev_name, entity_name, entity_type, endpoint, protocol):
    world.typ1 = {}
    world.typ2 = {}
    world.entity_name = entity_name
    world.entity_type = entity_type
    world.endpoint = endpoint
    prot = ProtocolTypes.get(protocol)    
    req=user_steps.create_device(world.service_name, dev_name, world.service_path, endpoint, {}, entity_name, entity_type, {}, {}, prot)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(dev_name)
    assert req.headers['Location'] == "/iot/devices/"+str(dev_name), 'ERROR de Cabecera: /iot/devices/' + str(dev_name) + ' esperada ' + str(req.headers['Location']) + ' recibida'
    print 'Se ha creado el device {}'.format(dev_name)
    if world.service_path=='void':
        service_path='/'
    else:
        service_path=world.service_path
    world.remember[world.service_name][service_path].setdefault('device', set())
    world.remember[world.service_name][service_path]['device'].add(dev_name)
    world.device_exists = True

@step('I create a Device with name "([^"]*)", protocol "([^"]*)", atributes and/or commands "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_device_with_attrs_cmds(step, dev_name, protocol, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.entity_name = {}
    world.entity_type = {}
    world.endpoint = {}
    commands=[]
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
    if typ1=='attr':
        attributes=[
             {
              "name": name1,
              "type": type1,
              "object_id": value1
              }
             ]
    if typ2=='attr':
        attribute={
              "name": name2,
              "type": type2,
              "object_id": value2
              }
        attributes.append(attribute)
    if typ1=='st_att':
        st_attributes=[
             {
              "name": name1,
              "type": type1,
              "value": value1
              }
             ]
    if typ2=='st_att':
        st_attribute={
              "name": name2,
              "type": type2,
              "value": value2
              }
        st_attributes.append(st_attribute)
    if typ1=='cmd':
        commands=[
             {
              "name": name1,
              "type": 'command',
              "value": value1
              }
             ]
        world.type1 = 'command'
    if typ2=='cmd':
        command={
              "name": name2,
              "type": 'command',
              "value": value2
              }
        world.type2 = 'command'
        commands.append(command)
    prot = ProtocolTypes.get(protocol)    
    req=user_steps.create_device(world.service_name, dev_name, world.service_path, {}, commands, {}, {}, attributes, st_attributes, prot)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El device {} no se ha creado correctamente".format(dev_name)
    assert req.headers['Location'] == "/iot/devices/"+str(dev_name), 'ERROR de Cabecera: /iot/devices/' + str(dev_name) + ' esperada ' + str(req.headers['Location']) + ' recibida'
    print 'Se ha creado el device {}'.format(dev_name)
    if world.service_path=='void':
        service_path='/'
    else:
        service_path=world.service_path
    world.remember[world.service_name][service_path].setdefault('device', set())
    world.remember[world.service_name][service_path]['device'].add(dev_name)
    world.device_exists = True

@step('I try to create a Device with name "([^"]*)", protocol "([^"]*)" and atribute or command "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)"')
def create_device_failed(step, dev_name, protocol, typ, name, type1, value):
    commands=[]
    attributes=[]
    st_attributes=[]
    world.device_name = dev_name
    if protocol:
        prot = ProtocolTypes.get(protocol)
    else:
        prot = {}    
    if typ=='attr':
        attributes=[
             {
              }
             ]
        if name:
            if name=='void':
                name=""
            attributes[0]['name']=name
        if type1:
            if type1=='void':
                type1=""
            attributes[0]['type']=type1
        if value:
            if value=='void':
                value=""
            attributes[0]['object_id']=value
    if typ=='st_att':
        st_attributes=[
             {
              }
             ]
        if name:
            if name=='void':
                name=""
            st_attributes[0]['name']=name
        if type1:
            if type1=='void':
                type1=""
            st_attributes[0]['type']=type1
        if value:
            if value=='void':
                value=""
            st_attributes[0]['value']=value        
    if typ=='cmd':
        commands=[
             {
              }
             ]
        if name:
            if name=='void':
                name=""
            commands[0]['name']=name
        if type1:
            if type1=='void':
                type1=""
            commands[0]['type']=type1
        if value:
            if value=='void':
                value=""
            commands[0]['value']=value
    if typ=='ent_name':
        world.entity_name=name
        world.req=user_steps.create_device(world.service_name, dev_name, world.service_path, {}, commands, name, {}, attributes, st_attributes, prot)
    elif typ=='protocol':
        if not name=='null':
            prot = ProtocolTypes.get(name)
            if prot:
                world.req=user_steps.create_device(world.service_name, dev_name, world.service_path, {}, commands, {}, {}, attributes, st_attributes, prot)
            else:
                world.req=user_steps.create_device(world.service_name, dev_name, world.service_path, {}, commands, {}, {}, attributes, st_attributes, name)
        else:
            world.req=user_steps.create_device(world.service_name, dev_name, world.service_path, {}, commands, {}, {}, attributes, st_attributes)
    else:        
        world.req=user_steps.create_device(world.service_name, dev_name, world.service_path, {}, commands, {}, {}, attributes, st_attributes, prot)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El device {} se ha podido crear".format(dev_name)
    print 'No se ha creado el device {}'.format(dev_name)

@step('the Device with name "([^"]*)" is created')
def device_created(step, dev_name):
    attributes=0
    st_attributes=0
    commands=0
    headers = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if world.service_path:
        if not world.service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(world.service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    req =  api.get_device(dev_name, headers)
    response = req.json()
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
    if world.typ1:
        if world.typ1 == 'attr':
            assert response['attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['attributes'][0]['name']
            assert response['attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['attributes'][0]['type']
            assert response['attributes'][0]['object_id'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['attributes'][0]['object_id']
            attributes+=1
        if world.typ1 == 'st_att':
            assert response['static_attributes'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['static_attributes'][0]['name']
            assert response['static_attributes'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
            assert response['static_attributes'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['static_attributes'][0]['value']
            st_attributes+=1
        if world.typ1 == 'cmd':
            assert response['commands'][0]['name'] == world.name1, 'Expected Result: ' + world.name1 + '\nObtained Result: ' + response['commands'][0]['name']
            assert response['commands'][0]['type'] == world.type1, 'Expected Result: ' + world.type1 + '\nObtained Result: ' + response['commands'][0]['type']
            assert response['commands'][0]['value'] == world.value1, 'Expected Result: ' + world.value1 + '\nObtained Result: ' + response['commands'][0]['value']
            commands+=1
    if world.typ2:
        if world.typ2 == 'attr':
            assert response['attributes'][attributes]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['attributes'][attributes]['name']
            assert response['attributes'][attributes]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['attributes'][attributes]['type']
            assert response['attributes'][attributes]['object_id'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['attributes'][attributes]['object_id']
        if world.typ2 == 'st_att':
            assert response['static_attributes'][st_attributes]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['name']
            assert response['static_attributes'][st_attributes]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['type']
            assert response['static_attributes'][st_attributes]['value'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['value']
        if world.typ2 == 'cmd':
            assert response['commands'][commands]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['commands'][commands]['name']
            assert response['commands'][commands]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['commands'][commands]['type']
            assert response['commands'][commands]['value'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['commands'][commands]['value']
    if world.prot:
        assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
    
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_device_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text.format(world.service_name)) in world.req.text, 'ERROR: ' + world.req.text
    if http_status=="409":
        if 'duplicate' in error_text:
            assert world.device_name in world.req.text, 'ERROR: ' + world.req.text        
            assert world.service_name in world.req.text, 'ERROR: ' + world.req.text
            if world.service_path=='void':
                world.service_path='/'        
            assert world.service_path in world.req.text, 'ERROR: ' + world.req.text
        else:
            assert world.entity_name in world.req.text, 'ERROR: ' + world.req.text        
                    
    
    
