from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)" and apikey "([^"]*)" not created')
def service_not_created(step, service_name, service_path, resource, apikey):
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

@step('I create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)"')
def create_service(step,srv_name,srv_path,resource,apikey,cbroker,entity_type,token):
    world.srv_path = srv_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    req=user_steps.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,entity_type,token)
    assert req.status_code == 201, 'ERROR: ' + req.text + "El servicio {} no se ha creado correctamente".format(srv_name)
    print 'Se ha creado el servicio {}'.format(srv_name)

@step('the Service with name "([^"]*)" and path "([^"]*)" is created')
def service_created(step, service_name, service_path):
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
    
@step('I try to create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def create_service_failed(step,srv_name,srv_path,resource,apikey,cbroker):
    world.srv_path = srv_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.req=user_steps.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,'','')
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido crear".format(srv_name)
    print 'No se ha creado el servicio {}'.format(srv_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text.format(world.cbroker)) in world.req.text, 'ERROR: ' + world.req.text
    
    
