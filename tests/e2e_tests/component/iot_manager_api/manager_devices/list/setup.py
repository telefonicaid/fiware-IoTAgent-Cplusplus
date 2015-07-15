from lettuce import step, world
from common.steps import service_with_path_created_precond, device_with_entity_values_of_service_precond, get_devices_list_manager, check_devices_data
from common.functions import Functions

functions = Functions()

@step('I try to retrieve the devices data of "([^"]*)", path "([^"]*)", entity "([^"]*)", protocol "([^"]*)", detailed "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_wrong_device_data(step,service_name, service_path, entity, protocol, detailed, limit, offset):
    world.req=functions.get_devices_created(service_name, service_path, entity, limit, offset, detailed, protocol, True)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El dispositivo {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    