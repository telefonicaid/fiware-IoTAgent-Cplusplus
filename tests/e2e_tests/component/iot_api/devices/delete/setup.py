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

@step('a Device with name "([^"]*)", entity_name "([^"]*)", entity_type "([^"]*)" and protocol "([^"]*)" created')
def device_created_precond(step, dev_name, entity_name, entity_type, protocol):
    world.device_name=dev_name
    user_steps.device_of_service_precond(world.service_name, world.service_path, dev_name, {}, {}, entity_name, entity_type, {}, {}, protocol)

@step('I delete the device "([^"]*)"')
def delete_service_data(step, device_name):
    headers = {}
    headers[CBROKER_HEADER] = str(world.service_name)
    if not world.service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(world.service_path)
    world.req =  api.delete_device(device_name, headers)
    assert world.req.ok, 'ERROR: ' + world.req.text

@step('I try to delete the device "([^"]*)" with service name "([^"]*)" and path "([^"]*)"')
def delete_device_data_failed(step, dev_name, service_name, service_path):
    headers = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
        world.service_name = service_name
    if not service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(service_path)
    world.req =  api.delete_device(dev_name, headers)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El device {} se ha podido borrar".format(service_name)
    print 'No se ha podido borrar el device {}'.format(service_name)
    
@step('the Device with name "([^"]*)" is deleted')
def check_services_data(step, device_name):
    assert not user_steps.device_created(world.service_name, device_name, world.service_path)
    if device_name==world.device_name:
        if world.service_path == 'void':
            world.remember[world.service_name]['/']['device'].remove(device_name)
        else:
            world.remember[world.service_name][world.service_path]['device'].remove(device_name)
            
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_deleted_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text