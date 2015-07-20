from lettuce import step, world
from common.steps import service_not_created_precond,create_service,create_service_with_attrs,service_created
from common.functions import Functions

functions = Functions()

@step('I try to create a service with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def create_service_failed(step,srv_name,srv_path,resource,apikey,cbroker):
    world.srv_path = srv_path
    world.srv_name = srv_name
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.req=functions.create_service_with_params(srv_name,srv_path,resource,apikey,cbroker,'','')
    assert world.req.status_code != 201, 'ERROR: ' + world.req.text + "El servicio {} se ha podido crear".format(srv_name)
    print 'No se ha creado el servicio {}'.format(srv_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    if http_status=="409":
        assert world.apikey in world.req.text, 'ERROR: ' + world.req.text        
        assert world.resource in world.req.text, 'ERROR: ' + world.req.text        
        assert world.cbroker in world.req.text, 'ERROR: ' + world.req.text        
        assert world.srv_name in world.req.text, 'ERROR: ' + world.req.text        
        assert world.srv_path in world.req.text, 'ERROR: ' + world.req.text        
    assert str(error_text.format(world.cbroker)) in world.req.text, 'ERROR: ' + world.req.text
    
    
