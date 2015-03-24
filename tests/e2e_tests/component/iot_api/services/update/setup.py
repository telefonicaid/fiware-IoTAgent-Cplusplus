from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)", entity_type "([^"]*)" and token "([^"]*)" created')
def service_precond(step, service_name, service_path, resource, apikey, cbroker, entity_type, token):
    world.service_name = service_name
    world.service_path = service_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = entity_type
    world.token = token
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, cbroker, entity_type, token)
   
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
    if world.resource:
        params['resource']= world.resource
    if world.apikey:
        params['apikey']= world.apikey
    json={
        attribute: value
        }
    world.req =  api.put_service('', json, headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text
    if attribute == 'apikey':
        print world.remember[service_name][service_path]['resource']
        del world.remember[service_name][service_path]['resource'][world.resource][world.apikey]
        if not value:
            value = ""
        world.remember[service_name][service_path]['resource'][world.resource].setdefault(value)
        print world.remember[service_name][service_path]['resource']
    if (attribute == 'resource'):
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
    if attribute == 'resource':
        assert response['resource'] != world.resource, 'NOT Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
        assert response['resource'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['resource']
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
        assert response['entity_type'] == world.entity_type, 'Expected Result: ' + world.entity_type + '\nObtained Result: ' + response['entity_type']
    if attribute == 'token':
        assert response['token'] != world.token, 'NOT Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
        assert response['token'] == value, 'Expected Result: ' + value + '\nObtained Result: ' + response['token']
    else:
        assert response['token'] == world.token, 'Expected Result: ' + world.token + '\nObtained Result: ' + response['token']
        
@step('I try to update the attribute "([^"]*)" with value "([^"]*)" of service "([^"]*)" with path "([^"]*)", resource "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def update_service_data_failed(step, attribute, value, service_name, service_path, resource, apikey, cbroker):
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
    if resource:
        params['resource']= resource
    if apikey:
        params['apikey']= apikey
    json={
        attribute: value
        }
    world.req =  api.put_service('', json, headers, params)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido actualizar".format(service_name)
    print 'No se ha actualizado el servicio {}'.format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text

@step('the service data NOT contains attribute "([^"]*)" with value "([^"]*)"')
def check_NOT_service_data(step, attribute, value):
    if (world.req.status_code == 400) | (world.req.status_code == 204):
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
    if attribute == 'resource':
        assert response['resource'] != value, 'NOT Expected Result: ' + value + '\nObtained Result: ' + response['resource']
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