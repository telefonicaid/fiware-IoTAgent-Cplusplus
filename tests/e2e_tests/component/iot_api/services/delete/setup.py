from lettuce import step, world
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
user_steps = UserSteps()


@step('two Services with name "([^"]*)", paths "([^"]*)" and "([^"]*)", protocols "([^"]*)" and "([^"]*)" and apikey "([^"]*)" created')
def service_precond(step, service_name, service_path, service_path2, protocol, protocol2, apikey):
    world.service_name = service_name
    world.service_path = service_path
    world.service_path2 = service_path2
    resource = URLTypes.get(protocol)
    world.resource = resource
    resource2 = URLTypes.get(protocol2)
    world.resource2 = resource2
    world.apikey = apikey
    if service_path:
        user_steps.service_with_params_precond(service_name, service_path, resource, apikey, 'http://myurl:80')
    if service_path2:
        user_steps.service_with_params_precond(service_name, service_path2, resource2, apikey, 'http://myurl:80')

@step('devices for services with name "([^"]*)", paths "([^"]*)" and "([^"]*)" and protocols "([^"]*)" and "([^"]*)" created')
def device_created_precond(step, service_name, service_path, service_path2, protocol, protocol2):
    world.device_name={}
    world.device_name2={}
    if service_path:
        device_name='device1'
        user_steps.device_of_service_precond(service_name, service_path, device_name, {}, {}, {}, {}, {}, {}, protocol)
        world.device_name=device_name
    if service_path2:
        device_name2='device2'
        user_steps.device_of_service_precond(service_name, service_path2, device_name2, {}, {}, {}, {}, {}, {}, protocol2)
        world.device_name2=device_name2

@step('I delete the service "([^"]*)" with path "([^"]*)"')
def delete_service_data(step, service_name, service_path):
    world.service_path_del = service_path
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(service_name)
    if not service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(service_path)
    if world.resource:
        params['resource']= world.resource
    if world.apikey:
        params['apikey']= world.apikey
    world.req =  api.delete_service('', headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I delete the service "([^"]*)" with param "([^"]*)" and path "([^"]*)"')
def delete_service_with_device(step, service_name, device, service_path):
    world.service_path_del = service_path
    headers = {}
    params = {}
    headers[CBROKER_HEADER] = str(service_name)
    if not service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(service_path)
    if world.resource:
        params['resource']= world.resource
    if world.apikey:
        params['apikey']= world.apikey
    if device:
        params['device']= device
#        print params
    world.req =  api.delete_service('', headers, params)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I try to delete the service "([^"]*)" with path "([^"]*)", resource "([^"]*)", apikey "([^"]*)" and param device "([^"]*)"')
def delete_service_data_failed(step, service_name, service_path, resource, apikey, device):
    world.resource = resource
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
        world.service_name = service_name
    if not service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(service_path)
    if resource:
        params['resource']= resource
    if apikey:
        params['apikey']= apikey
    if device:
        params['device']= device
    world.req =  api.delete_service('', headers, params)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido borrar".format(service_name)
    print 'No se ha podido borrar el servicio {}'.format(service_name)
    
@step('the Services with name "([^"]*)" and paths "([^"]*)" and "([^"]*)" are deleted or not')
def check_services_data(step, service_name, service_path, service_path2):
    if service_path == 'true':
        assert not user_steps.service_created(service_name, world.service_path, world.resource)
        if world.service_path == 'void':
            del world.remember[service_name]['/']['resource'][world.resource]
        else:
            del world.remember[service_name][world.service_path]['resource'][world.resource]
    else:
        assert user_steps.service_created(service_name, world.service_path, world.resource)       
    if service_path2:
        if service_path2 == 'true':
            assert not user_steps.service_created(service_name, world.service_path2, world.resource2)
            if world.service_path2 == 'void':
                del world.remember[service_name]['/']['resource'][world.resource]
            else:
                del world.remember[service_name][world.service_path2]['resource'][world.resource]
        else:
            assert user_steps.service_created(service_name, world.service_path2, world.resource2)       
            
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_deleted_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text

@step('the Service with name "([^"]*)" and path "([^"]*)" is not deleted')
def check_service_data(step, service_name, service_path):
    if service_name != 'void':
        if '/' in service_path:
            if world.resource:
                assert user_steps.service_created(service_name, service_path, world.resource)
            else:
                assert user_steps.service_created(service_name, service_path)

@step('devices "([^"]*)" of services with name "([^"]*)" and paths "([^"]*)" and "([^"]*)" are deleted or not')
def check_devices_data(step, devices, service_name, service_path, service_path2):
    if (service_path == 'true') & (devices=='true'):
        assert not user_steps.device_created(service_name, world.device_name, world.service_path)
        if world.service_path == 'void':
            world.remember[service_name]['/']['device'].remove(world.device_name)
        else:
            world.remember[service_name][world.service_path]['device'].remove(world.device_name)
    else:
        assert user_steps.device_created(service_name, world.device_name, world.service_path)       
    if service_path2:
        if (service_path2 == 'true') & (devices=='true'):
            assert not user_steps.device_created(service_name, world.device_name2, world.service_path2)
            if world.service_path2 == 'void':
                world.remember[service_name]['/']['device'].remove(world.device_name2)
            else:
                world.remember[service_name][world.service_path2]['device'].remove(world.device_name2)
        else:
            assert user_steps.device_created(service_name, world.device_name2, world.service_path2)       