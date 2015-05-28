import time
from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER,MANAGER_SERVER_ROOT


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
api2 = Rest_Utils_SBC(server_root=MANAGER_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('a Service with name "([^"]*)", path "([^"]*)", protocol "([^"]*)" and apikey "([^"]*)" created')
def service_precond(step, service_name, service_path, protocol, apikey):
    world.service_name = service_name
    world.srv_path = service_path
    resource = URLTypes.get(protocol)
    world.resource = resource
    prot = ProtocolTypes.get(protocol)
    world.prot = prot
    world.apikey = apikey
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, 'http://myurl:80')

@step('I retrieve the service data of "([^"]*)", path "([^"]*)", protocol "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_service_data(step,service_name, service_path, protocol, limit, offset):
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(service_name)
    if service_path:
        if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
        else:    
            headers[CBROKER_PATH_HEADER] = '/'
    if protocol:
        prot = ProtocolTypes.get(protocol)
        if not prot:
            prot = protocol
        world.prot = prot
        params['protocol']= world.prot
    if limit:
        params['limit']= limit
    if offset:
        params['offset']= offset
#    time.sleep(1)
    world.req =  api2.get_service('', headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I receive the service data of "([^"]*)" services')
def check_service_data(step, num_services):
    res = world.req.json()
    print res
    assert len(res['services']) == int(num_services), 'Error: ' + str(num_services) + ' services expected, ' + str(len(res['services'])) + ' received'
    if len(res['services'])==1:
        response = res['services'][0]
        assert response['service'] == world.service_name, 'Expected Result: ' + world.service_name + '\nObtained Result: ' + response['service']
#        assert response['resource'] == world.resource, 'Expected Result: ' + world.resource + '\nObtained Result: ' + response['resource']
        assert response['protocol'] == world.prot, 'Expected Result: ' + world.prot + '\nObtained Result: ' + response['protocol']
        if world.srv_path:
            if world.srv_path == 'void':
                world.srv_path = '/'
            assert response['service_path'] == world.srv_path, 'Expected Result: ' + world.srv_path + '\nObtained Result: ' + response['service_path']
        if world.apikey:
            assert response['apikey'] == world.apikey, 'Expected Result: ' + world.apikey + '\nObtained Result: ' + response['apikey']
        else:
            assert response['apikey'] == "", 'Expected Result: NULL \nObtained Result: ' + response['apikey']
#    time.sleep(1)    
@step('I try to retrieve the service data of "([^"]*)", path "([^"]*)", protocol "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_wrong_service_data(step,service_name, service_path, protocol, limit, offset):
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
    if not service_path == 'void':
            headers[CBROKER_PATH_HEADER] = str(service_path)
    if protocol:
        prot = ProtocolTypes.get(protocol)
        params['protocol']= prot
    if limit:
        params['limit']= limit
    if offset:
        params['offset']= offset
    world.req =  api.get_service('', headers, params)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El servicio {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    