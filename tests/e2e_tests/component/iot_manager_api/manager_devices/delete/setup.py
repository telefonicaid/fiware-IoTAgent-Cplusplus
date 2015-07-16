from lettuce import step, world
from common.functions import Functions
from common.steps import service_with_path_created_precond, device_with_entity_values_of_service_precond, delete_device_data_manager, check_device_data_deleted
from iotqautils.gtwRest import Rest_Utils_SBC
from common.user_steps import UserSteps, URLTypes, ProtocolTypes
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER,MANAGER_SERVER_ROOT


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
api2 = Rest_Utils_SBC(server_root=MANAGER_SERVER_ROOT+'/iot')
user_steps = UserSteps()
functions = Functions()


@step('I try2 to delete the device "([^"]*)" with service name "([^"]*)", protocol "([^"]*)" and path "([^"]*)"')
def delete_device_data_failed2(step, dev_name, service_name, protocol, service_path):
    headers = {}
    params = {}
    if not service_name == 'void':
        headers[CBROKER_HEADER] = service_name
        world.service_name = service_name
    if not service_path == 'void':
        headers[CBROKER_PATH_HEADER] = str(service_path)
    if protocol:
        prot = ProtocolTypes.get(protocol)
        if not prot:
            prot = protocol
        params['protocol']= prot
    world.req =  api2.delete_device(dev_name, headers, params)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El device {} se ha podido borrar".format(service_name)
    print 'No se ha podido borrar el device {}'.format(service_name)
    
@step('I try to delete the device "([^"]*)" with service name "([^"]*)", protocol "([^"]*)" and path "([^"]*)"')
def delete_device_data_failed(step, device_name, service_name, protocol, service_path):
    world.req =  functions.delete_device_data(device_name, service_name, service_path, True, protocol)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El device {} se ha podido borrar".format(service_name)
    print 'No se ha borrado el device {}'.format(service_name)
            
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_deleted_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
#    assert world.req.json()['details'] == str(error_text.format("{ \"id\" ","\""+world.cbroker_id+"\"}")), 'ERROR: ' + world.req.text
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text