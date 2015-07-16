from lettuce import step, world
from common.functions import Functions
from common.steps import service_with_path_created_precond, device_with_entity_values_of_service_precond, delete_device_data, check_device_data_deleted

functions = Functions()

@step('I try to delete the device "([^"]*)" with service name "([^"]*)" and path "([^"]*)"')
def delete_device_data_failed(step, device_name, service_name, service_path):
    world.req =  functions.delete_device_data(device_name, service_name, service_path)
    assert world.req.status_code != 204, 'ERROR: ' + world.req.text + "El device {} se ha podido borrar".format(device_name)
    print 'No se ha borrado el device {}'.format(device_name)
               
@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_deleted_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text