from lettuce import step, world
from common.steps import service_with_all_params_manager_precond, service_with_attribute_created_manager_precond, update_service_data_manager, check_service_data_updated_manager, check_NOT_service_data
from common.functions import Functions, URLTypes

functions = Functions()

        
@step('I try to update the attribute "([^"]*)" with value "([^"]*)" of service "([^"]*)" with path "([^"]*)", protocol "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def update_service_data_failed(step, attribute, value, service_name, service_path, protocol, apikey, cbroker):
    if not service_name == 'void':
        world.service_name = service_name
    if not service_path == 'void':
        world.srv_path = service_path
    else:
        world.srv_path = "/"
    resource = URLTypes.get(protocol)
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.req=functions.update_service_with_params(attribute, service_name, value, service_path, {}, apikey, True, True, protocol)
    assert world.req.status_code != 204, 'ERROR: ' + world.req.text + "El servicio {} se ha podido actualizar".format(service_name)
    print 'No se ha actualizado el servicio {}'.format(service_name)

@step('user receives the "([^"]*)", the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, http_code, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    if http_code:
        assert str(http_code) in world.req.text, 'ERROR: ' + world.req.text
