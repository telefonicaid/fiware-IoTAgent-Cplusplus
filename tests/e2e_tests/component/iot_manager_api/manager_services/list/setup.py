from lettuce import step, world
from common.steps import service_with_params_precond,get_service_data_manager,check_service_data
from common.functions import Functions

functions = Functions()


@step('I try to retrieve the service data of "([^"]*)", path "([^"]*)", protocol "([^"]*)", limit "([^"]*)" and offset "([^"]*)"')
def get_wrong_service_data(step,service_name, service_path, protocol, limit, offset):
    world.req = functions.get_service_created(service_name, service_path, {}, limit, offset, protocol)
    assert world.req.status_code != 200, 'ERROR: ' + world.req.text + "El servicio {} se ha podido recuperar".format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    