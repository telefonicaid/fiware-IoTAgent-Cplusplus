from lettuce import step, world
from common.functions import Functions
from common.steps import service_with_path_created_precond, device_with_attr_or_cmd_created_precond, get_device_data, check_device_data

functions = Functions()
            
@step('I try to retrieve the device data of device "([^"]*)" with service "([^"]*)" and path "([^"]*)"')
def get_wrong_device_data(step, dev_name, service_name, service_path):
    world.req=functions.get_device_created(service_name, service_path, dev_name)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El dispositivo {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    