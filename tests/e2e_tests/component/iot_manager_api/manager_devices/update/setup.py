from lettuce import step, world
from common.functions import Functions
from common.steps import service_with_path_created_precond, device_with_attr_or_cmd_created_precond, update_device_data_manager, check_device_data_updated

functions = Functions()

@step('I try to update the device data of device "([^"]*)" with service "([^"]*)", protocol "([^"]*)" and path "([^"]*)" with the attribute "([^"]*)" and value "([^"]*)"')
def update_wrong_device_data(step, device_name, service_name, protocol, service_path, attribute, value):
    world.req =  functions.update_device_with_params(attribute, device_name, value, service_name, service_path, True, True, protocol)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El dispositivo {} se ha podido actualizar".format(service_name)
        
@step('user receives the "([^"]*)", the "([^"]*)" and the "([^"]*)"')
def assert_device_created_failed(step, http_status, http_code, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    if http_code:
        assert str(http_code) in world.req.text, 'ERROR: ' + world.req.text
    if http_code=="409":
        assert world.entity_name in world.req.text, 'ERROR: ' + world.req.text        
