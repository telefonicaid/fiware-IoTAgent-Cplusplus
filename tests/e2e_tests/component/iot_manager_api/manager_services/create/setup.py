from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)" and apikey "([^"]*)" not created')
def service_not_created(step, service_name, service_path, protocol, apikey):
    resource = URLTypes.get(protocol)
    if not resource:
        print "no hay que buscar servicio"
        return
    if user_steps.service_created(service_name, service_path, resource):
        print 'ERROR: El servicio {} ya existe'.format(service_name)
        world.remember.setdefault(service_name, {})
        if service_path == 'void':
            service_path='/'
        world.remember[service_name].setdefault(service_path, {})
        world.remember[service_name][service_path].setdefault('resource', {})
        world.remember[service_name][service_path]['resource'].setdefault(resource, {})
        if not apikey:
            apikey = ""
        world.remember[service_name][service_path]['resource'][resource].setdefault(apikey)

@step('I create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)"')
def create_service(step,srv_name,srv_path,protocol,apikey,cbroker,entity_type,token):
    world.typ1 = {}
    world.typ2 = {}
    world.srv_path = srv_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    req=user_steps.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,entity_type,token,{},{},protocol)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('I create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atributes "([^"]*)" and "([^"]*)", with names "([^"]*)" and "([^"]*)", types "([^"]*)" and "([^"]*)" and values "([^"]*)" and "([^"]*)"')
def create_service_with_attrs(step, srv_name, srv_path, protocol, apikey, cbroker, typ1, typ2, name1, name2, type1, type2, value1, value2):
    world.srv_path = srv_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
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
    req=user_steps.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,{},{},attributes,st_attributes,protocol)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('the Service with name "([^"]*)" and path "([^"]*)" is created')
def service_created(step, service_name, service_path):
    attributes=0
    st_attributes=0
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(service_name)
    if service_path:
        if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    if world.resource:
        params['resource']= world.resource
    req =  api.get_service('', headers, params)
    res = req.json()
    assert res['count'] == 1, 'Error: 1 service expected, ' + str(res['count']) + ' received'
    response = res['services'][0]
    assert response['service'] == service_name, 'Expected Result: ' + service_name + '\nObtained Result: ' + response['service']
    assert response['resource'] == world.resource, 'Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
    if world.srv_path:
        if world.srv_path == 'void':
            assert response['service_path'] == '/', 'Expected Result: ' + '/' + '\nObtained Result: ' + response['service_path']
        else:
            assert response['service_path'] == world.srv_path, 'Expected Result: ' + world.srv_path + '\nObtained Result: ' + response['service_path']
    if world.apikey:
        assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
    else:
        assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
    if world.cbroker:
        assert response['cbroker'] == world.cbroker, 'Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']
    else:
        assert response['cbroker'] == "", 'Expected Result: NULL \nObtained Result: ' + response['cbroker']
    if world.entity_type:
        assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
    if world.token:
        assert response['token'] == world.token, 'Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
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
    if world.typ2:
        if world.typ2 == 'attr':
            assert response['attributes'][attributes]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['attributes'][attributes]['name']
            assert response['attributes'][attributes]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['attributes'][attributes]['type']
            assert response['attributes'][attributes]['object_id'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['attributes'][attributes]['object_id']
        if world.typ2 == 'st_att':
            assert response['static_attributes'][st_attributes]['name'] == world.name2, 'Expected Result: ' + world.name2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['name']
            assert response['static_attributes'][st_attributes]['type'] == world.type2, 'Expected Result: ' + world.type2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['type']
            assert response['static_attributes'][st_attributes]['value'] == world.value2, 'Expected Result: ' + world.value2 + '\nObtained Result: ' + response['static_attributes'][st_attributes]['value']
    
@step('I try to create a service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def create_service_failed(step,srv_name,srv_path,protocol,apikey,cbroker):
    world.srv_path = srv_path
    world.srv_name = srv_name
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.req=user_steps.create_service_with_params(srv_name,srv_path,{},apikey,cbroker,{},{},{},{},protocol)
#    world.req=user_steps.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,'','')
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido crear".format(srv_name)
    print 'No se ha creado el servicio {}'.format(srv_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    if http_status=="409":
        assert world.apikey in world.req.text, 'ERROR: ' + world.req.text        
        assert world.resource in world.req.text, 'ERROR: ' + world.req.text        
        assert world.cbroker in world.req.text, 'ERROR: ' + world.req.text        
        assert world.srv_name in world.req.text, 'ERROR: ' + world.req.text        
        assert world.srv_path in world.req.text, 'ERROR: ' + world.req.text        
    assert str(error_text.format(world.cbroker)) in world.req.text, 'ERROR: ' + world.req.text
    
    
