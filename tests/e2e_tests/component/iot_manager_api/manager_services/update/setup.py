from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER,MANAGER_SERVER_ROOT


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
api2 = Rest_Utils_SBC(server_root=MANAGER_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)" created')
def service_precond(step, service_name, service_path, protocol, apikey, cbroker, entity_type, token):
    world.service_name = service_name
    world.service_path = service_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.prot = protocol
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, cbroker, entity_type, token)
   
@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atribute "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def service_with_attrs_precond(step, service_name, service_path, protocol, apikey, cbroker, typ, name, type1, value):
    world.service_name = service_name
    world.service_path = service_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.prot = protocol
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    attributes=[]
    st_attributes=[]
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
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, cbroker, {}, {}, attributes, st_attributes)
   
@step('I update the attribute "([^"]*)" of service "([^"]*)" with value "([^"]*)"')
def update_service_data(step, attribute, service_name, value):
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
#    if world.prot:
#        prot = ProtocolTypes.get(world.prot)
#        if not prot:
#            prot=world.prot
#        print 'PROTOCOLO' + prot
#        params['protocol']= prot
    if world.apikey:
        params['apikey']= world.apikey
    if not 'att' in attribute:
#        json={
#              attribute: value
#        }
        service={
            "services":[
                {
                 attribute: value
                }
                ]
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
#        json={
#              attrs_type: attributes
#              }        
        service={
            "services":[
                {
                 attrs_type: attributes
                }
                ]
                }
    if world.prot:
        prot = ProtocolTypes.get(world.prot)
        if not prot:
            prot=world.prot
        service['services'][0]['protocol']= [prot]
#    if world.apikey:
#        service['services'][0]['apikey']= world.apikey
    world.req =  api2.put_service('', service, headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text
    if attribute == 'apikey':
        print world.remember[service_name][service_path]['resource']
        del world.remember[service_name][service_path]['resource'][world.resource][world.apikey]
        if not value:
            value = ""
        world.remember[service_name][service_path]['resource'][world.resource].setdefault(value)
        print world.remember[service_name][service_path]['resource']
    if (attribute == 'protocol'):
        print world.remember[service_name][service_path]['resource']
        del world.remember[service_name][service_path]['resource'][world.resource]
        world.remember[service_name][service_path]['resource'].setdefault(value, {})
        world.remember[service_name][service_path]['resource'][value].setdefault(world.apikey)
        print world.remember[service_name][service_path]['resource']            

@step('the service data contains attribute "([^"]*)" with value "([^"]*)"')
def check_service_data(step, attribute, value):
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if world.service_path:
        if not world.service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(world.service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    if attribute == 'resource':
        params['resource']= value
    else:
        params['resource']= world.resource
    req =  api.get_service('', headers, params)
    res = req.json()
    response = res['services'][0]
    assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
    if world.service_path == 'void':
        world.service_path = '/'
    assert response['service_path'] == world.service_path, 'Expected Result: ' + world.service_path + '\nObtained Result: ' + response['service_path']
    if attribute == 'protocol':
        resource = URLTypes.get(value)
        assert response['resource'] != world.resource, 'NOT Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
        assert response['resource'] == resource, 'Expected Result: ' + resource + '\nObtained Result: ' + response['resource']
    else:
        assert response['resource'] == world.resource, 'Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
    if attribute == 'apikey':
        if value:
            assert response['apikey'] != world.apikey, 'NOT Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
            assert response['apikey'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['apikey']
        else:
            assert response['apikey'] != world.apikey, 'NOT Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
            assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
    else:
        if world.apikey:
            assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
        else:
            assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
    if attribute == 'cbroker':
        assert response['cbroker'] != world.cbroker, 'NOT Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']
        assert response['cbroker'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['cbroker']
    else:
        assert response['cbroker'] == world.cbroker, 'Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']
    if attribute == 'entity_type':
        assert response['entity_type'] != world.entity_type, 'NOT Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
        assert response['entity_type'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['entity_type']
    else:
        if world.entity_type:
            assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
    if attribute == 'token':
        assert response['token'] != world.token, 'NOT Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
        assert response['token'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['token']
    else:
        if world.token:
            assert response['token'] == world.token, 'Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
    if 'att' in attribute:
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
        assert response['static_attributes'][0]['name'] == name, 'Expected Result: ' + name + '\nObtained Result: ' + response['static_attributes'][0]['name']
        assert response['static_attributes'][0]['type'] == type1, 'Expected Result: ' + type1 + '\nObtained Result: ' + response['static_attributes'][0]['type']
        assert response['static_attributes'][0]['value'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['static_attributes'][0]['value']
        
@step('I try to update the attribute "([^"]*)" with value "([^"]*)" of service "([^"]*)" with path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def update_service_data_failed(step, attribute, value, service_name, service_path, protocol, apikey, cbroker):
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
        world.service_name = service_name
    if not service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(service_path)
        world.service_path = service_path
    else:
        world.service_path = "/"
    if apikey:
        params['apikey']= apikey
    service={
            "services":[
                {
                }
                ]
                }
    if protocol:
        prot = ProtocolTypes.get(protocol)
        if not prot:
            prot=protocol
        if prot == 'void':
            service['services'][0]['protocol']= []
        else:
            service['services'][0]['protocol']= [prot]
#    if apikey:
#        service['services'][0]['apikey']= apikey
#    json={
#        attribute: value
#        }
#    if attribute=='empty_json':
#        json = {}
    if not attribute=='empty_json':
        service['services'][0][attribute]=value
    world.req =  api2.put_service('', service, headers, params)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido actualizar".format(service_name)
    print 'No se ha actualizado el servicio {}'.format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    if http_status=="409":
        assert world.apikey in world.req.text, 'ERROR: ' + world.req.text        
        assert world.resource in world.req.text, 'ERROR: ' + world.req.text        
        assert world.service_name in world.req.text, 'ERROR: ' + world.req.text        
        assert world.service_path in world.req.text, 'ERROR: ' + world.req.text        

@step('the service data NOT contains attribute "([^"]*)" with value "([^"]*)"')
def check_NOT_service_data(step, attribute, value):
    if (world.req.status_code == 400) | (world.req.status_code == 404):
        print 'No se comprueba el servicio'
        return
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if world.service_path:
        if not world.service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(world.service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    params['resource']= world.resource
    req =  api.get_service('', headers, params)
    res = req.json()
    response = res['services'][0]
    assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
    if world.service_path == 'void':
        world.service_path = '/'
    assert response['service_path'] == world.service_path, 'Expected Result: ' + world.service_path + '\nObtained Result: ' + response['service_path']
    if attribute == 'protocol':
        resource = URLTypes.get(value)
        assert response['resource'] != resource, 'NOT Expected Result: ' + resource + '\nObtained Result: ' + response['resource']
    assert response['resource'] == world.resource, 'Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
    if attribute == 'apikey':
        if value:
            assert response['apikey'] != value, 'NOT Expected Result: ' + value + '\nObtained Result: ' + response['apikey']
        else:
            assert response['apikey'] != "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
        assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
    else:
        if world.apikey:
            assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
        else:
            assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
    if attribute == 'cbroker':
        assert response['cbroker'] != value, 'NOT Expected Result: ' + value + '\nObtained Result: ' + response['cbroker']
    assert response['cbroker'] == world.cbroker, 'Expected Result: ' + world.cbroker + '\nObtained Result: ' + response['cbroker']