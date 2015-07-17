from lettuce import step, world
from common.steps import services_with_params_precond, devices_created_precond, delete_service_data, delete_service_with_device, check_services_data_deleted, check_service_data_deleted, check_devices_data_deleted
from common.functions import Functions

functions = Functions()

@step('I try to delete the service "([^"]*)" with path "([^"]*)", resource "([^"]*)", apikey "([^"]*)" and param device "([^"]*)"')
def delete_service_data_failed(step, service_name, service_path, resource, apikey, device):
    world.resource = resource
    if not service_name == 'void':
        world.service_name = service_name
    if not service_path == 'void':
        world.srv_path = service_path
    else:
        world.srv_path = "/"
    world.req = functions.delete_service_data(service_name, service_path, resource, apikey, device)
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido borrar".format(service_name)
    print 'No se ha podido borrar el servicio {}'.format(service_name)
                
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_deleted_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text