from lettuce import step, world
from common.steps import service_with_all_params_precond,service_with_attribute_created_precond,update_service_data,check_service_data_updated
from iotqautils.gtwRest import Rest_Utils_SBC
from common.functions import Functions
from common.user_steps import UserSteps
from common.gw_configuration import IOT_SERVER_ROOT,CBROKER_HEADER,CBROKER_PATH_HEADER


api = Rest_Utils_SBC(server_root=IOT_SERVER_ROOT+'/iot')
user_steps = UserSteps()
functions = Functions()

@step('a Service2 with name "([^"]*)", path "([^"]*)", resource "([^"]*)", apikey "([^"]*)", cbroker "([^"]*)" and atribute "([^"]*)", with name "([^"]*)", type "([^"]*)" and value "([^"]*)" created')
def service_with_attrs_precond2(step, service_name, service_path, resource, apikey, cbroker, typ, name, type1, value):
    world.service_name = service_name
    world.service_path = service_path
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.entity_type = {}
    world.token = {}
    attributes=[]
    st_attributes=[]
    world.typ = typ
    world.name = name
    world.type = type1
    world.value = value
    if typ=='attr':
        attributes=[
             {
              "name": name,
              "type": type1,
              "object_id": value
              }
             ]
    if typ=='st_att':
        st_attributes=[
             {
              "name": name,
              "type": type1,
              "value": value
              }
             ]
    user_steps.service_with_params_precond(service_name, service_path, resource, apikey, cbroker, {}, {}, attributes, st_attributes)
           
@step('I try to update the attribute "([^"]*)" with value "([^"]*)" of service "([^"]*)" with path "([^"]*)", resource "([^"]*)", apikey "([^"]*)" and cbroker "([^"]*)"')
def update_service_data_failed(step, attribute, value, service_name, service_path, resource, apikey, cbroker):
    if not service_name == 'void':
        world.service_name = service_name
    if not service_path == 'void':
        world.srv_path = service_path
    else:
        world.srv_path = "/"
    world.resource = resource
    world.apikey = apikey
    world.cbroker = cbroker
    world.req=functions.update_service_with_params(attribute, service_name, value, service_path, resource, apikey, fail=True)
    assert world.req.status_code != 204, 'ERROR: ' + world.req.text + "El servicio {} se ha podido actualizar".format(service_name)
    print 'No se ha actualizado el servicio {}'.format(service_name)

@step('user receives the "([^"]*)" and the "([^"]*)"')
def assert_service_created_failed(step, http_status, error_text):
    assert world.req.status_code == int(http_status), "El codigo de respuesta {} es incorrecto".format(world.req.status_code)
    assert str(error_text) in world.req.text, 'ERROR: ' + world.req.text
    if http_status=="409":
        assert world.apikey in world.req.text, 'ERROR: ' + world.req.text        
        assert world.resource in world.req.text, 'ERROR: ' + world.req.text        
        assert world.service_name in world.req.text, 'ERROR: ' + world.req.text        
        assert world.srv_path in world.req.text, 'ERROR: ' + world.req.text        

